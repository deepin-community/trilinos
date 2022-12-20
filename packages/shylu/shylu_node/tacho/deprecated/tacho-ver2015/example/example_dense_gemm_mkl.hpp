#pragma once
#ifndef __EXAMPLE_DENSE_GEMM_MKL_HPP__
#define __EXAMPLE_DENSE_GEMM_MKL_HPP__

#include <Kokkos_Core.hpp>
#include <impl/Kokkos_Timer.hpp>

#include "ShyLU_NodeTacho_config.h"
#include "Teuchos_ScalarTraits.hpp"

#include "util.hpp"

#include "dense_matrix_base.hpp"
#include "dense_matrix_view.hpp"

#include "gemm.hpp"
#include "dense_flop.hpp"

#ifdef HAVE_SHYLU_NODETACHO_MKL
#include "mkl_service.h"
#endif

namespace Tacho {

  using namespace std;

  template<typename ValueType,
           typename OrdinalType,
           typename SizeType = OrdinalType,
           typename SpaceType = void,
           typename MemoryTraits = void>
  KOKKOS_INLINE_FUNCTION
  int exampleDenseGemmMKL(const OrdinalType mmin,
                          const OrdinalType mmax,
                          const OrdinalType minc,
                          const OrdinalType k,
                          const bool verbose) {
    typedef ValueType   value_type;
    typedef OrdinalType ordinal_type;
    typedef SizeType    size_type;

    typedef DenseMatrixBase<value_type,ordinal_type,size_type,SpaceType,MemoryTraits> DenseMatrixBaseType;

    int r_val = 0;

    Kokkos::Impl::Timer timer;
    double t = 0.0;

    cout << "DenseGemmMKL:: test matrices "
         <<":: mmin = " << mmin << " , mmax = " << mmax << " , minc = " << minc << " , k = "<< k << endl;

    ostringstream os;
    os.precision(3);
    os << scientific;
        
    for (ordinal_type m=mmin;m<=mmax;m+=minc) {
      os.str("");

      DenseMatrixBaseType AA("AA", mmax, k), BB("BB", k, mmax), CC("CC", m, m);

      for (ordinal_type j=0;j<AA.NumCols();++j)
        for (ordinal_type i=0;i<AA.NumRows();++i)
          AA.Value(i,j) = 2.0*((value_type)rand()/(RAND_MAX)) - 1.0;

      for (ordinal_type j=0;j<BB.NumCols();++j)
        for (ordinal_type i=0;i<BB.NumRows();++i)
          BB.Value(i,j) = 2.0*((value_type)rand()/(RAND_MAX)) - 1.0;

      for (ordinal_type j=0;j<CC.NumCols();++j)
        for (ordinal_type i=0;i<CC.NumRows();++i)
          CC.Value(i,j) = 2.0*((value_type)rand()/(RAND_MAX)) - 1.0;

      const double flop = get_flop_gemm<value_type>(m, m, k);

      os << "DenseGemmMKL:: m = " << m << " n = " << m << " k = " << k;
      {
        timer.reset();
        Teuchos::BLAS<ordinal_type,value_type> blas;

        const ordinal_type mm = CC.NumRows();
        const ordinal_type nn = CC.NumCols();
        const ordinal_type kk = BB.NumRows();

        blas.GEMM(Teuchos::NO_TRANS, Teuchos::NO_TRANS,
                  mm, nn, kk,
                  1.0,
                  AA.ValuePtr(), AA.ColStride(),
                  BB.ValuePtr(), BB.ColStride(),
                  1.0,
                  CC.ValuePtr(), CC.ColStride());
        t = timer.seconds();
        os << ":: MKL Performance = " << (flop/t/1.0e9) << " [GFLOPs]  ";
      }
      cout << os.str() << endl;
    }

    return r_val;
  }
}

#endif
