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

#include <Ioss_CodeTypes.h>
#include <Ioss_FileInfo.h>
#include <Ioss_Utils.h>
#include <cstddef>
#include <string>
#include <sys/select.h>
#include <sys/unistd.h>

#ifdef SEACAS_HAVE_MPI
#include <numeric>
#endif

#include <cstdio>
#include <sys/stat.h>
#include <unistd.h>

namespace {
  bool internal_access(const std::string &name, int mode);
  bool do_stat(const std::string &filename, struct stat *s);
} // namespace

namespace Ioss {

  FileInfo::FileInfo() = default;

  FileInfo::FileInfo(std::string my_filename) : filename_(std::move(my_filename))
  {
    readable_ = internal_access(filename_, R_OK);
    exists_   = readable_ || internal_access(filename_, F_OK);
  }

  FileInfo::FileInfo(const char *my_filename) : filename_(std::string(my_filename))
  {
    readable_ = internal_access(filename_, R_OK);
    exists_   = readable_ || internal_access(filename_, F_OK);
  }

  FileInfo::FileInfo(const FileInfo &copy_from) = default;

  FileInfo::FileInfo(const std::string &dirpath, const std::string &my_filename)
  {
    static std::string SLASH("/");

    if (!dirpath.empty()) {
      filename_ = dirpath;
      if (filename_.at(filename_.size() - 1) != '/') {
        filename_ += SLASH;
      }
    }
    filename_ += my_filename;
    readable_ = internal_access(filename_, R_OK);
    exists_   = readable_ || internal_access(filename_, F_OK);
  }

  FileInfo::~FileInfo() = default;

  //: Returns TRUE if the file exists (is readable)
  bool FileInfo::exists() const { return exists_; }

  int FileInfo::parallel_exists(MPI_Comm communicator, std::string &where) const
  {
    PAR_UNUSED(communicator);
    PAR_UNUSED(where);

#ifdef SEACAS_HAVE_MPI
    int my_rank = 0;
    int my_size = 1;
    if (communicator != MPI_COMM_NULL) {
      MPI_Comm_rank(communicator, &my_rank);
      MPI_Comm_size(communicator, &my_size);
    }
    if (my_size == 1)
#endif
      return exists_ ? 1 : 0;

#ifdef SEACAS_HAVE_MPI
    // Now handle the parallel case
    std::vector<int> result(my_size);
    int              my_val = exists_ ? 1 : 0;
    MPI_Allgather(&my_val, 1, MPI_INT, &result[0], 1, MPI_INT, communicator);

    int sum = std::accumulate(result.begin(), result.end(), 0);
    if (my_rank == 0 && sum < my_size) {
      std::vector<size_t> procs;
      for (int i = 0; i < my_size; i++) {
        if (result[i] == 0) {
          procs.push_back(i);
        }
      }
      where = Ioss::Utils::format_id_list(procs, "--");
    }
    return sum;
#endif
  }

  //: Returns TRUE if the file is readable
  bool FileInfo::is_readable() const { return readable_; }

  //: Returns TRUE if the file is writable
  bool FileInfo::is_writable() const { return internal_access(filename_, W_OK); }

  //: Returns TRUE if the file is executable
  bool FileInfo::is_executable() const { return internal_access(filename_, X_OK); }

  //: Returns TRUE if we are pointing to a file or a symbolic link to
  //: a file.
  bool FileInfo::is_file() const
  {
    struct stat s
    {
    };
    if (do_stat(filename_, &s)) {
      return S_ISREG(s.st_mode);
    }

    return false;
  }

  //: Returns TRUE if we are pointing to a directory or a symbolic link to
  //: a directory.
  bool FileInfo::is_dir() const
  {
    struct stat s
    {
    };
    if (do_stat(filename_, &s)) {
      return S_ISDIR(s.st_mode);
    }

    return false;
  }

  //: Returns TRUE if we are pointing to a symbolic link
  bool FileInfo::is_symlink() const
  {
    struct stat s
    {
    };
    if (lstat(filename_.c_str(), &s) == 0) {
      return S_ISLNK(s.st_mode);
    }

    return false;
  }

  //: Time of last data modification. See 'man stat(2)'
  time_t FileInfo::modified() const
  {
    struct stat s
    {
    };
    if (do_stat(filename_, &s)) {
      return s.st_mtime;
    }

    return 0;
  }

  //: Time of last access
  time_t FileInfo::accessed() const
  {
    struct stat s
    {
    };
    if (do_stat(filename_, &s)) {
      return s.st_atime;
    }

    return 0;
  }

  //: Time of last status change. (creation, chmod, ...)
  time_t FileInfo::created() const
  {
    struct stat s
    {
    };
    if (do_stat(filename_, &s)) {
      return s.st_ctime;
    }

    return 0;
  }

  //: File size in bytes. Only if is_file() == true
  off_t FileInfo::size() const
  {
    struct stat s
    {
    };
    if (do_stat(filename_, &s)) {
      return s.st_size;
    }

    return 0;
  }

  //: Returns the filename
  const std::string FileInfo::filename() const { return filename_; }

  //: Sets the filename
  void FileInfo::set_filename(const std::string &name)
  {
    filename_ = name;
    readable_ = internal_access(filename_, R_OK);
    exists_   = readable_ || internal_access(filename_, F_OK);
  }

  //: Sets the filename
  void FileInfo::set_filename(const char *name)
  {
    filename_ = std::string(name);
    readable_ = internal_access(filename_, R_OK);
    exists_   = readable_ || internal_access(filename_, F_OK);
  }

  //: Returns the filename extension or the empty string if there is
  //: no extension.  Assumes extension is all characters following the
  //: last period.
  const std::string FileInfo::extension() const
  {
    size_t ind  = filename_.find_last_of('.', std::string::npos);
    size_t inds = filename_.find_last_of('/', std::string::npos);

    // Protect against './filename' returning /filename as extension
    if (ind != std::string::npos && (inds == std::string::npos || inds < ind)) {
      return filename_.substr(ind + 1, filename_.size());
    }

    return std::string();
  }

  const std::string FileInfo::pathname() const
  {
    size_t ind = filename_.find_last_of('/', filename_.size());
    if (ind != std::string::npos) {
      return filename_.substr(0, ind);
    }

    return std::string();
  }

  const std::string FileInfo::tailname() const
  {
    size_t ind = filename_.find_last_of('/', filename_.size());
    if (ind != std::string::npos) {
      return filename_.substr(ind + 1, filename_.size());
    }

    return filename_; // No path, just return the filename
  }

  const std::string FileInfo::basename() const
  {
    std::string tail = tailname();

    // Strip off the extension
    size_t ind = tail.find_last_of('.', tail.size());
    if (ind != std::string::npos) {
      return tail.substr(0, ind);
    }

    return tail;
  }

  bool FileInfo::remove_file()
  {
    int success = std::remove(filename_.c_str());
    return success == 0;
  }
} // namespace Ioss

namespace {
  bool internal_access(const std::string &name, int mode)
  {
    if (name.empty()) {
      return false;
    }
    if (::access(name.c_str(), mode) != 0) {
      return false;
    }
    return true;
  }

  bool do_stat(const std::string &filename, struct stat *s)
  {
#if defined(__PUMAGON__)
    // Portland pgCC compiler on janus has 'char*' instead of 'const char*' for
    // first argument to stat function.
    return (stat((char *)filename.c_str(), s) == 0);
#else
    return (stat(filename.c_str(), s) == 0);
#endif
  }
} // namespace
