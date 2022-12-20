// @HEADER
//
// ***********************************************************************
//
//        MueLu: A package for multigrid based preconditioning
//                  Copyright 2011 Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ***********************************************************************
//
// @HEADER

#include "MueLu_config.hpp"

#if defined(HAVE_MUELU_EXPLICIT_INSTANTIATION)

// We need both the _decl.hpp and _def.hpp files here, because if ETI
// is ON, then the .hpp file will only include the _decl.hpp file.
#include "MueLu_Details_LinearSolverFactory_decl.hpp"
#include "MueLu_Details_LinearSolverFactory_def.hpp"
// We need this whether or not ETI is on, in order to define typedefs
// for making Tpetra's macros work.
#include "TpetraCore_ETIHelperMacros.h"

#ifdef HAVE_MUELU_EPETRA
// Do explicit instantiation of MueLu::Details::LinearSolverFactory,
// for Epetra objects.
template class MueLu::Details::LinearSolverFactory<Epetra_MultiVector, Epetra_Operator, double>;
#endif // HAVE_MUELU_EPETRA

// Define typedefs that make the Tpetra macros work
TPETRA_ETI_MANGLING_TYPEDEFS()

// Do explicit instantiation of MueLu::Details::LinearSolverFactory, for
// Tpetra objects, for all combinations of Tpetra template parameters
// for which Tpetra does explicit template instantiation (ETI).
//
// TODO amk: does MueLu have a required dependency on Tpetra?
#ifdef HAVE_MUELU_TPETRA
TPETRA_INSTANTIATE_SLGN_NO_ORDINAL_SCALAR( MUELU_DETAILS_LINEARSOLVERFACTORY_INSTANT )
#endif // HAVE_MUELU_TPETRA

// TODO amk: do we also have to do this for Xpetra?

#endif // HAVE_MUELU_EXPLICIT_INSTANTIATION