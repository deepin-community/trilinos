#!/bin/bash
#export SALLOC_CTEST_TIME_LIMIT_MINUTES=1:00:00
if [ "${Trilinos_TRACK}" == "" ] ; then
  export Trilinos_TRACK=PrimaryATDM
fi
$WORKSPACE/Trilinos/cmake/ctest/drivers/atdm/tlcc2/local-driver.sh
