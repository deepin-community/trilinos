// @HEADER
// ***********************************************************************
//
//                           Sacado Package
//                 Copyright (2006) Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
// USA
// Questions? Contact David M. Gay (dmgay@sandia.gov) or Eric T. Phipps
// (etphipp@sandia.gov).
//
// ***********************************************************************
// @HEADER

#ifndef FADLAPACKUNITTESTS_HPP
#define FADLAPACKUNITTESTS_HPP

// Sacado includes
#include "Sacado_No_Kokkos.hpp"
#include "Sacado_Fad_LAPACK.hpp"
#include "Sacado_Random.hpp"

// gtest includes
#include <gtest/gtest.h>

#include "GTestUtils.hpp"

#define COMPARE_FAD_VECTORS(X1, X2, n)          \
  ASSERT_TRUE(X1.size() == std::size_t(n));  \
  ASSERT_TRUE(X2.size() == std::size_t(n));  \
  for (unsigned int i=0; i<n; i++) {            \
    COMPARE_FADS(X1[i], X2[i]);                 \
  }                                             \
  ;

// A class for testing differentiated LAPACK operations for general Fad types
template <class FadType>
class FadLAPACKUnitTests : public ::testing::Test {
protected:
  typedef typename Sacado::ScalarType<FadType>::type ScalarType;
  typedef Sacado::Fad::Vector<unsigned int,FadType> VectorType;

  // Random number generator
  Sacado::Random<ScalarType> urand;

  // Real random number generator for derivative components
  Sacado::Random<double> real_urand;

  // Number of matrix rows
  unsigned int m;

  // Number of matrix columns
  unsigned int n;

  // Number of matrix columns for level 3 blas
  unsigned int l;

  // Number of derivative components
  unsigned int ndot;

  // Tolerances to which fad objects should be the same
  double tol_a, tol_r;

  FadType fad;

  FadLAPACKUnitTests() :
    urand(), real_urand(), m(5), n(6), l(4), ndot(7),
    tol_a(1.0e-11), tol_r(1.0e-11) {}

}; // class FadLAPACKUnitTests

TYPED_TEST_SUITE_P(FadLAPACKUnitTests);

// What is the purpose of this test?  It doesn't test Fad at all.
TYPED_TEST_P(FadLAPACKUnitTests, testGESV) {
  const int n = 2;
  const int nrhs = 1;
  double A[] = { 1.1, 0.1, .01, 0.9 };
  const int lda = 2;
  int IPIV[] = {0, 0};
  double B[] = { 0.1, 0.2 };
  const int ldb = 2;
  int info(0);

  const double refX[] = {0.088978766430738, 0.212335692618807};

  Teuchos::LAPACK<int,double> teuchos_lapack;
  teuchos_lapack.GESV(n, nrhs, &A[0], lda, &IPIV[0], &B[0], ldb, &info);

  COMPARE_VALUES(B[0],refX[0]);
  COMPARE_VALUES(B[1],refX[1]);

  //Teuchos::LAPACK<int,FadType> sacado_lapack(false);
  //sacado_blas.SCAL(m, alpha, &x2[0], 1);
  //COMPARE_VALUES(1,0);
}

REGISTER_TYPED_TEST_SUITE_P(
  FadLAPACKUnitTests,
  testGESV
  );

#endif // FADLAPACKUNITTESTS_HPP
