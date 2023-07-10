#
# Unset the ATDM_CONFIG_ env vars that can be set by the
# <system_name>/enviornment.sh script
#

unset OMP_NUM_THREADS
unset OMP_PROC_BIND
unset OMP_PLACES
unset OMPI_CC
unset OMPI_CXX
unset OMPI_FC
unset Trilinos_CTEST_RUN_CUDA_AWARE_MPI
unset ATDM_CONFIG_ENABLE_SPARC_SETTINGS
unset ATDM_CONFIG_USE_NINJA
unset ATDM_CONFIG_EXTRA_LINK_FLAGS
unset ATDM_CONFIG_CMAKE_CXX_USE_RESPONSE_FILE_FOR_OBJECTS
unset ATDM_CONFIG_NUM_CORES_ON_MACHINE
unset ATDM_CONFIG_MAX_NUM_CORES_TO_USE
unset ATDM_CONFIG_BUILD_COUNT
unset ATDM_CONFIG_C_FLAGS
unset ATDM_CONFIG_CXX_FLAGS
unset ATDM_CONFIG_Fortran_FLAGS
unset ATDM_CONFIG_OPENMP_FORTRAN_FLAGS
unset ATDM_CONFIG_OPENMP_FORTRAN_LIB_NAMES
unset ATDM_CONFIG_OPENMP_GOMP_LIBRARY
unset ATDM_CONFIG_CMAKE_SKIP_INSTALL_RPATH
unset ATDM_CONFIG_PARALLEL_COMPILE_JOBS_LIMIT
unset ATDM_CONFIG_PARALLEL_LINK_JOBS_LIMIT
unset ATDM_CONFIG_CTEST_PARALLEL_LEVEL
unset ATDM_CONFIG_NVCC_WRAPPER
unset ATDM_CONFIG_SPARC_TPL_BASE
unset ATDM_CONFIG_BINUTILS_LIBS
unset ATDM_CONFIG_BLAS_LIBS
unset ATDM_CONFIG_LAPACK_LIBS
unset ATDM_CONFIG_BOOST_LIBS
unset ATDM_CONFIG_USE_HWLOC
unset ATDM_CONFIG_HWLOC_LIBS
unset ATDM_CONFIG_HDF5_LIBS
unset ATDM_CONFIG_NETCDF_LIBS
unset ATDM_CONFIG_SUPERLUDIST_INCLUDE_DIRS
unset ATDM_CONFIG_SUPERLUDIST_LIBS
unset ATDM_CONFIG_METIS_LIBS
unset ATDM_CONFIG_PARMETIS_LIBS
unset ATDM_CONFIG_CGNS_LIBRARY_NAMES
unset ATDM_CONFIG_CGNS_LIBS
unset ATDM_CONFIG_MPI_EXEC
unset ATDM_CONFIG_MPI_PRE_FLAGS
unset ATDM_CONFIG_MPI_POST_FLAGS
unset ATDM_CONFIG_WORKSPACE_BASE_DEFAULT
unset ATDM_CONFIG_INSTALL_PBP_RUNNER_DEFAULT
unset ATDM_CONFIG_TRIL_CMAKE_INSTALL_PREFIX_DATE_BASE_DEFAULT
unset ATDM_CONFIG_MAKE_INSTALL_GROUP_DEFAULT
unset ATDM_CONFIG_SBATCH_DEFAULT_TIMEOUT
unset ATDM_CONFIG_SBATCH_DEFAULT_ACCOUNT
unset atdm_run_script_on_compute_node
unset ATDM_CONFIG_COMPLETED_ENV_SETUP
unset ATDM_CONFIG_Kokkos_ENABLE_SERIAL
unset ATDM_CONFIG_MPI_EXEC_NUMPROCS_FLAG
unset atdm_run_script_on_compute_node
unset ATDM_CONFIG_Trilinos_LINK_SEARCH_START_STATIC
unset ATDM_CONFIG_SBATCH_EXTRA_ARGS

# NOTE: Not unsetting the following env vars on purpose because we want users
# to be able to set them before sourcing atdm/load-env.sh <buildname>:
#
# * ATDM_CONFIG_TRIL_CMAKE_INSTALL_PREFIX
# * ATDM_CONFIG_SET_GROUP_AND_PERMISSIONS_ON_INSTALL_BASE_DIR
# * ATDM_CONFIG_MAKE_INSTALL_GROUP
