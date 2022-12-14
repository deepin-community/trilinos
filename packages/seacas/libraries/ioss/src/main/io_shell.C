// Copyright(C) 1999-2017 National Technology & Engineering Solutions
// of Sandia, LLC (NTESS).  Under the terms of Contract DE-NA0003525 with
// NTESS, the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//
//     * Neither the name of NTESS nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <Ionit_Initializer.h>
#include <Ioss_CodeTypes.h>
#include <Ioss_FileInfo.h>
#include <Ioss_MeshCopyOptions.h>
#include <Ioss_MeshType.h>
#include <Ioss_ParallelUtils.h>
#include <Ioss_ScopeGuard.h>
#include <Ioss_SerializeIO.h>
#include <Ioss_SubSystem.h>
#include <Ioss_SurfaceSplit.h>
#include <Ioss_Utils.h>
#include <fmt/format.h>

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <sys/times.h>
#include <unistd.h>

#include "shell_interface.h"

// ========================================================================

namespace {
  std::string codename;
  std::string version = "5.0";

  bool mem_stats = false;

  void file_copy(IOShell::Interface &interface, int rank);

  Ioss::PropertyManager set_properties(IOShell::Interface &interface);
} // namespace

int main(int argc, char *argv[])
{
  int rank = 0;
#ifdef SEACAS_HAVE_MPI
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  ON_BLOCK_EXIT(MPI_Finalize);
#endif

#ifdef SEACAS_HAVE_KOKKOS
  Kokkos::ScopeGuard kokkos(argc, argv);
#endif

  IOShell::Interface interface;
  bool               success = interface.parse_options(argc, argv);
  if (!success) {
    exit(EXIT_FAILURE);
  }

  codename = interface.options_.basename(argv[0]);

  Ioss::SerializeIO::setGroupFactor(interface.serialize_io_size);
  mem_stats = interface.memory_statistics;

  Ioss::Init::Initializer io;

  std::string in_file  = interface.inputFile[0];
  std::string out_file = interface.outputFile;

  if (rank == 0 && !interface.quiet) {
    fmt::print(stderr,
               "Input:    '{}', Type: {}\n"
               "Output:   '{}', Type: {}\n\n",
               in_file, interface.inFiletype, out_file, interface.outFiletype);
  }

#ifdef SEACAS_HAVE_KOKKOS
  if (rank == 0)
    fmt::print(stderr, "Kokkos default execution space configuration:\n");
  Kokkos::DefaultExecutionSpace::print_configuration(std::cerr, false);
  if (rank == 0)
    fmt::print(stderr, "\n");
#endif

  double begin = Ioss::Utils::timer();
  try {
    file_copy(interface, rank);
  }
  catch (std::exception &e) {
    if (rank == 0) {
      fmt::print(stderr, "\n{}\n\nio_shell terminated due to exception\n", e.what());
    }
    exit(EXIT_FAILURE);
  }

#ifdef SEACAS_HAVE_MPI
  Ioss::ParallelUtils parallel(MPI_COMM_WORLD);
  parallel.barrier();
#endif
  double end = Ioss::Utils::timer();

  if (rank == 0 && !interface.quiet) {
    fmt::print(stderr, "\n\n\tTotal Execution time = {} seconds\n", end - begin);
  }
  if (mem_stats) {
    int64_t MiB = 1024 * 1024;
#ifdef SEACAS_HAVE_MPI
    int64_t min, max, avg;
    int64_t hwmin, hwmax, hwavg;
    parallel.memory_stats(min, max, avg);
    parallel.hwm_memory_stats(hwmin, hwmax, hwavg);
    if (rank == 0) {
      fmt::print(stderr, "\n\tCurrent Memory: {:n}M  {:n}M  {:n}M\n", min / MiB, max / MiB,
                 avg / MiB);
      fmt::print(stderr, "\tHigh Water Memory: {:n}M  {:n}M  {:n}M\n", hwmin / MiB, hwmax / MiB,
                 hwavg / MiB);
    }
#else
    int64_t mem = Ioss::Utils::get_memory_info();
    int64_t hwm = Ioss::Utils::get_hwm_memory_info();
    if (rank == 0) {
      fmt::print(stderr,
                 "\n\tCurrent Memory:    {:n}M\n"
                 "\tHigh Water Memory: {:n}M\n",
                 mem / MiB, hwm / MiB);
    }
#endif
  }
  if (rank == 0) {
    fmt::print(stderr, "\n{} execution successful.\n", codename);
  }
  return EXIT_SUCCESS;
}

namespace {
  void file_copy(IOShell::Interface &interface, int rank)
  {
    Ioss::PropertyManager properties = set_properties(interface);

    bool first = true;
    for (const auto &inpfile : interface.inputFile) {

      //========================================================================
      // INPUT Database...
      //========================================================================
      Ioss::DatabaseIO *dbi = Ioss::IOFactory::create(
          interface.inFiletype, inpfile, Ioss::READ_MODEL, (MPI_Comm)MPI_COMM_WORLD, properties);
      if (dbi == nullptr || !dbi->ok(true)) {
        std::exit(EXIT_FAILURE);
      }

      if (mem_stats) {
        dbi->progress("Database Creation");
      }
      if (!interface.lower_case_variable_names) {
        dbi->set_lower_case_variable_names(false);
      }
      if (interface.outFiletype == "cgns") {
        // CGNS stores BCs (SideSets) on the zones which
        // correspond to element blocks.  If split input sideblocks
        // by element block, then output is much easier.
        dbi->set_surface_split_type(Ioss::SPLIT_BY_ELEMENT_BLOCK);
      }
      else {
        dbi->set_surface_split_type(Ioss::int_to_surface_split(interface.surface_split_type));
      }
      dbi->set_field_separator(interface.fieldSuffixSeparator);

      dbi->set_field_recognition(!interface.disable_field_recognition);

      if (interface.ints_64_bit) {
        dbi->set_int_byte_size_api(Ioss::USE_INT64_API);
      }

      if (!interface.groupName.empty()) {
        bool success = dbi->open_group(interface.groupName);
        if (!success) {
          if (rank == 0) {
            fmt::print(stderr, "ERROR: Unable to open group '{}' in file '{}'\n",
                       interface.groupName, inpfile);
          }
          return;
        }
      }

      // NOTE: 'region' owns 'db' pointer at this time...
      Ioss::Region region(dbi, "region_1");

      if (region.mesh_type() == Ioss::MeshType::HYBRID) {
        fmt::print(stderr,
                   "\nERROR: io_shell does not support '{}' meshes. Only 'Unstructured' or "
                   "'Structured' mesh is supported at this time.\n",
                   region.mesh_type_string());
        return;
      }

      // Get length of longest name on input file...
      int max_name_length = dbi->maximum_symbol_length();
      if (max_name_length > 0) {
        properties.add(Ioss::Property("MAXIMUM_NAME_LENGTH", max_name_length));
      }

      // Get integer size being used on the input file and propgate
      // to output file...
      int int_byte_size_api = dbi->int_byte_size_api();
      if (!properties.exists("INTEGER_SIZE_API")) {
        if (interface.ints_32_bit) {
          properties.add(Ioss::Property("INTEGER_SIZE_DB", 4));
        }
        properties.add(Ioss::Property("INTEGER_SIZE_API", int_byte_size_api));
      }
      if (int_byte_size_api == 8) {
        interface.ints_64_bit = true;
      }
      //========================================================================
      // OUTPUT Database...
      //========================================================================
      bool append = false;
      if (interface.append_step < std::numeric_limits<int>::max()) {
        properties.add(Ioss::Property("APPEND_OUTPUT_AFTER_STEP", interface.append_step));
        append = true;
      }

      if (interface.append_time < std::numeric_limits<double>::max()) {
        properties.add(Ioss::Property("APPEND_OUTPUT_AFTER_TIME", interface.append_time));
        append = true;
      }

      if (append) {
        properties.add(Ioss::Property("APPEND_OUTPUT", Ioss::DB_APPEND));
      }

      if (interface.minimize_open_files) {
        properties.add(Ioss::Property("MINIMIZE_OPEN_FILES", "ON"));
      }

      Ioss::MeshCopyOptions options{};
      options.verbose           = !interface.quiet;
      options.memory_statistics = interface.memory_statistics;
      options.debug             = interface.debug;
      options.ints_64_bit       = interface.ints_64_bit;
      options.delete_timesteps  = interface.delete_timesteps;
      options.minimum_time      = interface.minimum_time;
      options.maximum_time      = interface.maximum_time;
      options.data_storage_type = interface.data_storage_type;
      options.delay             = interface.timestep_delay;
      options.reverse           = interface.reverse;
      options.add_proc_id       = interface.add_processor_id_field;

      size_t ts_count = 0;
      if (region.property_exists("state_count") &&
          region.get_property("state_count").get_int() > 0) {
        ts_count = region.get_property("state_count").get_int();
      }

      int flush_interval = interface.flush_interval; // Default is zero -- do not flush until end
      properties.add(Ioss::Property("FLUSH_INTERVAL", flush_interval));

      if (interface.split_times == 0 || interface.delete_timesteps || ts_count == 0 || append ||
          interface.inputFile.size() > 1) {
        Ioss::DatabaseIO *dbo =
            Ioss::IOFactory::create(interface.outFiletype, interface.outputFile,
                                    Ioss::WRITE_RESTART, (MPI_Comm)MPI_COMM_WORLD, properties);
        if (dbo == nullptr || !dbo->ok(true)) {
          std::exit(EXIT_FAILURE);
        }

        // NOTE: 'output_region' owns 'dbo' pointer at this time
        Ioss::Region output_region(dbo, "region_2");
        // Set the qa information...
        output_region.property_add(Ioss::Property(std::string("code_name"), codename));
        output_region.property_add(Ioss::Property(std::string("code_version"), version));

        if (interface.inputFile.size() > 1) {
          properties.add(Ioss::Property("APPEND_OUTPUT", Ioss::DB_APPEND_GROUP));

          if (!first) {
            // Putting each file into its own output group...
            // The name of the group will be the basename portion of the filename...
            Ioss::FileInfo file(inpfile);
            dbo->create_subgroup(file.tailname());
          }
          else {
            first = false;
          }
        }

        // Do normal copy...
        Ioss::Utils::copy_database(region, output_region, options);
        if (mem_stats) {
          dbo->release_memory();
        }
      }
      else {
        // We are splitting out the timesteps into separate files.
        // Each file will contain `split_times` timesteps. If
        // 'split_cyclic` is > 0, then recycle filenames
        // (A,B,C,A,B,C,...) otherwise
        // keep creating new filenames (0001, 0002, 0003, ...)

        // Get list of all times on input database...
        std::vector<double> times;
        for (size_t step = 0; step < ts_count; step++) {
          double time = region.get_state_time(step + 1);
          if (time < interface.minimum_time) {
            continue;
          }
          if (time > interface.maximum_time) {
            break;
          }
          times.push_back(time);
        }
        ts_count = times.size();

        int splits = (ts_count + interface.split_times - 1) / interface.split_times;
        int width  = std::to_string(splits).length();
        for (int split = 0; split < splits; split++) {
          int step_min = split * interface.split_times;
          int step_max = step_min + interface.split_times - 1;
          if (step_max >= (int)times.size()) {
            step_max = (int)times.size() - 1;
          }
          options.minimum_time = times[step_min];
          options.maximum_time = times[step_max];

          std::string filename = interface.outputFile;
          if (interface.split_cyclic > 0) {
            static const std::string suffix{"ABCDEFGHIJKLMNOPQRSTUVWXYZ"};
            filename += "." + suffix.substr(split % interface.split_cyclic, 1);
          }
          else {
            filename = fmt::format("{0}_{1:0{2}}", filename, split + 1, width);
          }

          if (rank == 0 && !interface.quiet) {
            if (step_min == step_max) {
              fmt::print(stderr, "\tWriting step {:n} to {}\n", step_min + 1, filename);
            }
            else {
              fmt::print("\tWriting steps {:n}..{:n} to {}\n", step_min + 1, step_max + 1,
                         filename);
            }
          }

          Ioss::DatabaseIO *dbo =
              Ioss::IOFactory::create(interface.outFiletype, filename, Ioss::WRITE_RESTART,
                                      (MPI_Comm)MPI_COMM_WORLD, properties);
          if (dbo == nullptr || !dbo->ok(true)) {
            std::exit(EXIT_FAILURE);
          }

          // NOTE: 'output_region' owns 'dbo' pointer at this time
          Ioss::Region output_region(dbo, "region_2");
          // Set the qa information...
          output_region.property_add(Ioss::Property(std::string("code_name"), codename));
          output_region.property_add(Ioss::Property(std::string("code_version"), version));

          Ioss::Utils::copy_database(region, output_region, options);
          if (mem_stats) {
            dbo->release_memory();
          }
          options.verbose = false;
        }
      }
      if (mem_stats) {
        dbi->progress("Prior to Memory Released... ");
        dbi->release_memory();
        dbi->progress("Memory Released... ");
      }
    } // loop over input files
  }

  Ioss::PropertyManager set_properties(IOShell::Interface &interface)
  {
    Ioss::PropertyManager properties;

    if (interface.ints_64_bit) {
      properties.add(Ioss::Property("INTEGER_SIZE_DB", 8));
      properties.add(Ioss::Property("INTEGER_SIZE_API", 8));
    }

    if (interface.ints_32_bit) {
      properties.add(Ioss::Property("INTEGER_SIZE_DB", 4));
    }

    if (interface.reals_32_bit) {
      properties.add(Ioss::Property("REAL_SIZE_DB", 4));
    }

    if (interface.in_memory_read) {
      properties.add(Ioss::Property("MEMORY_READ", 1));
    }

    if (interface.in_memory_write) {
      properties.add(Ioss::Property("MEMORY_WRITE", 1));
    }

    if (interface.compression_level > 0 || interface.shuffle) {
      properties.add(Ioss::Property("FILE_TYPE", "netcdf4"));
      properties.add(Ioss::Property("COMPRESSION_LEVEL", interface.compression_level));
      properties.add(Ioss::Property("COMPRESSION_SHUFFLE", static_cast<int>(interface.shuffle)));
    }

    if (interface.compose_output == "default") {
      if (interface.outFiletype == "cgns") {
        properties.add(Ioss::Property("COMPOSE_RESULTS", "YES"));
        properties.add(Ioss::Property("COMPOSE_RESTART", "YES"));
      }
      else {
        properties.add(Ioss::Property("COMPOSE_RESULTS", "NO"));
        properties.add(Ioss::Property("COMPOSE_RESTART", "NO"));
      }
    }
    else if (interface.compose_output == "external") {
      properties.add(Ioss::Property("COMPOSE_RESULTS", "NO"));
      properties.add(Ioss::Property("COMPOSE_RESTART", "NO"));
    }
    else if (interface.compose_output != "none") {
      properties.add(Ioss::Property("COMPOSE_RESULTS", "YES"));
      properties.add(Ioss::Property("COMPOSE_RESTART", "YES"));
    }

    if (interface.file_per_state) {
      properties.add(Ioss::Property("FILE_PER_STATE", "YES"));
    }

    if (interface.netcdf4) {
      properties.add(Ioss::Property("FILE_TYPE", "netcdf4"));
    }

    if (interface.netcdf5) {
      properties.add(Ioss::Property("FILE_TYPE", "netcdf5"));
    }

    if (interface.inputFile.size() > 1) {
      properties.add(Ioss::Property("ENABLE_FILE_GROUPS", 1));
    }

    if (interface.debug) {
      properties.add(Ioss::Property("LOGGING", 1));
    }

    if (interface.memory_statistics) {
      properties.add(Ioss::Property("ENABLE_TRACING", 1));
    }

    if (!interface.decomp_method.empty()) {
      properties.add(Ioss::Property("DECOMPOSITION_METHOD", interface.decomp_method));
    }
    return properties;
  }
} // namespace
