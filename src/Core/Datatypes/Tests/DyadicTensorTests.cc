/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
   University of Utah.

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/

#include <Core/Datatypes/Dyadic3DTensor.h>
#include <Core/Datatypes/MatrixFwd.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include "Core/Utils/Exception.h"

using namespace SCIRun;
using namespace Core::Datatypes;
using namespace Core::Geometry;
using namespace ::testing;

const std::vector<Vector> nativeEigvecs = {
    Vector(1.6, 0.9, 4.3), Vector(4.0, 6.4, 7), Vector(6, 34, 1)};
const std::string eigvecsString = "[1.6 0.9 4.3 4 6.4 7 6 34 1]";

std::vector<DenseColumnMatrix> getEigenEigvecs()
{
  std::vector<DenseColumnMatrix> eigvecs(3);
  for (int i = 0; i < 3; ++i)
  {
    eigvecs[i] = DenseColumnMatrix(3);
    for (int j = 0; j < 3; ++j)
      eigvecs[i][j] = nativeEigvecs[i][j];
  }
  return eigvecs;
}

TEST(Dyadic3DTensorTest, ConstructTensorWithNativeVectors1)
{
  Dyadic3DTensor t(nativeEigvecs);
  std::stringstream ss;
  ss << t;
  ASSERT_EQ(eigvecsString, ss.str());
}

TEST(Dyadic3DTensorTest, ConstructTensorWithNativeVectors2)
{
  Dyadic3DTensor t(nativeEigvecs[0], nativeEigvecs[1], nativeEigvecs[2]);
  std::stringstream ss;
  ss << t;
  ASSERT_EQ(eigvecsString, ss.str());
}

TEST(Dyadic3DTensorTest, ConstructTensorWithEigenMatrices1)
{
  Dyadic3DTensor t(getEigenEigvecs());
  std::stringstream ss;
  ss << t;
  ASSERT_EQ(eigvecsString, ss.str());
}

TEST(Dyadic3DTensorTest, ConstructTensorWithEigenMatrices2)
{
  auto eigvecs = getEigenEigvecs();
  Dyadic3DTensor t(eigvecs[0], eigvecs[1], eigvecs[2]);
  std::stringstream ss;
  ss << t;
  ASSERT_EQ(eigvecsString, ss.str());
}

TEST(Dyadic3DTensorTest, ConstructTensorWithDoubles)
{
  Dyadic3DTensor t(1, 2, 3, 4, 5, 6);
  std::stringstream ss;
  ss << t;
  ASSERT_EQ("[1 2 3 2 4 5 3 5 6]", ss.str());
}

TEST(Dyadic3DTensorTest, CanConstructWithMatrixOfSixValues)
{
  Dyadic3DTensor t =
      symmetricTensorFromSixElementArray(DenseColumnMatrix({1, 2, 3, 4, 5, 6}));
  std::stringstream ss;
  ss << t;
  ASSERT_EQ("[1 2 3 2 4 5 3 5 6]", ss.str());
}

TEST(Dyadic3DTensorTest, CanConstructWithMatrixOfNineValues)
{
  Dyadic3DTensor t =
      symmetricTensorFromSixElementArray(DenseColumnMatrix({1, 2, 3, 2, 4, 5, 3, 5, 6}));
  std::stringstream ss;
  ss << t;
  ASSERT_EQ("[1 2 3 2 4 5 3 5 6]", ss.str());
}

TEST(Dyadic3DTensorTest, CannotConstructTensorWithLessThanThreeNativeEigenvectors)
{
  ASSERT_DEATH(Dyadic3DTensor t({nativeEigvecs[0], nativeEigvecs[1]}), "");
}

TEST(Dyadic3DTensorTest, CannotConstructTensorWithLessThanThreeEigenEigenvectors)
{
  auto eigvecs = getEigenEigvecs();
  ASSERT_DEATH(Dyadic3DTensor t({eigvecs[0], eigvecs[1]}), "");
}

TEST(Dyadic3DTensorTest, CannotConstructTensorWithMoreThanThreeEigenvectors)
{
  ASSERT_DEATH(Dyadic3DTensor t(
                   {nativeEigvecs[0], nativeEigvecs[1], nativeEigvecs[2], nativeEigvecs[0]}),
      "");
}

TEST(Dyadic3DTensorTest, CannotConstructTensorWithMoreThanThreeEigenEigenvectors)
{
  auto eigvecs = getEigenEigvecs();
  ASSERT_DEATH(Dyadic3DTensor t({eigvecs[0], eigvecs[1], eigvecs[2], eigvecs[0]}), "");
}

TEST(Dyadic3DTensorTest, CannotConstructTensorWithLessThanSixDoubles)
{
  ASSERT_DEATH(Dyadic3DTensor t({1, 2, 3, 4, 5}), "");
}

TEST(Dyadic3DTensorTest, CannotConstructTensorWithMoreThanSixDoubles)
{
  ASSERT_DEATH(Dyadic3DTensor t({1, 2, 3, 4, 5, 6, 7}), "");
}

TEST(DyadicTensorTest, ConstructTensorWithEigenVectors)
{
  std::vector<DenseColumnMatrix> nativeEigvecs = {
      DenseColumnMatrix(4), DenseColumnMatrix(4), DenseColumnMatrix(4), DenseColumnMatrix(4)};
  int n = 0;
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      nativeEigvecs[i][j] = ++n;
  DyadicTensor t(nativeEigvecs);
  std::stringstream ss;
  ss << t;

  ASSERT_EQ("[1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16]", ss.str());
}

TEST(Dyadic3DTensorTest, StringConversion)
{
  Dyadic3DTensor t(nativeEigvecs);
  Dyadic3DTensor t2();
  std::stringstream ss;
  ss << t;
  // ss >> t2;
  // ASSERT_EQ(t, t2); TODO fix test
}

TEST(Dyadic3DTensorTest, GetEigenvalues)
{
  std::vector<Vector> vecs = {Vector(1, 0, 0), Vector(0, 2, 0), Vector(0, 0, 3)};
  Dyadic3DTensor t(vecs);
  auto eigvals = t.getEigenvalues();
  ASSERT_EQ(3.0, eigvals[0]);
  ASSERT_EQ(2.0, eigvals[1]);
  ASSERT_EQ(1.0, eigvals[2]);
}

TEST(Dyadic3DTensorTest, GetEigenvectors)
{
  std::vector<DenseColumnMatrix> vecs = {
      DenseColumnMatrix({1, 0, 0}), DenseColumnMatrix({0, 2, 0}), DenseColumnMatrix({0, 0, 3})};
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      vecs[i][j] = (i == j) ? i + 1 : 0;

  Dyadic3DTensor t(vecs);
  auto eigvecs = t.getEigenvectors();

  ASSERT_EQ(DenseColumnMatrix({0, 0, 1}), eigvecs[0]);
  ASSERT_EQ(DenseColumnMatrix({0, 1, 0}), eigvecs[1]);
  ASSERT_EQ(DenseColumnMatrix({1, 0, 0}), eigvecs[2]);
}

TEST(Dyadic3DTensorTest, GetEigenvector)
{
  std::vector<DenseColumnMatrix> vecs = {
      DenseColumnMatrix({1, 0, 0}), DenseColumnMatrix({0, 2, 0}), DenseColumnMatrix({0, 0, 3})};
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      vecs[i][j] = (i == j) ? i + 1 : 0;

  Dyadic3DTensor t(vecs);

  ASSERT_EQ(DenseColumnMatrix({0, 1, 0}), t.getEigenvector(1));
}

TEST(DyadicTensorTest, Equivalent)
{
  std::vector<DenseColumnMatrix> vecs = {
      DenseColumnMatrix({1, 0, 0}), DenseColumnMatrix({0, 2, 0}), DenseColumnMatrix({0, 0, 3})};
  Dyadic3DTensor t(nativeEigvecs);
  DyadicTensor t2(vecs);

  ASSERT_FALSE(t == t2);
  ASSERT_TRUE(t != t2);
  ASSERT_TRUE(t == t);
  ASSERT_TRUE(t2 == t2);
}

TEST(DyadicTensorTest, DifferentDimensionsNotEquivalent)
{
  DyadicTensor t(
      {DenseColumnMatrix({1, 0, 0}), DenseColumnMatrix({0, 1, 0}), DenseColumnMatrix({0, 0, 1})});
  DyadicTensor t2({DenseColumnMatrix({1, 0, 0, 0}), DenseColumnMatrix({0, 1, 0, 0}),
      DenseColumnMatrix({0, 0, 1, 0}), DenseColumnMatrix({0, 0, 0, 1})});

  ASSERT_TRUE(t != t2);
}

TEST(DyadicTensorTest, EqualsOperatorTensor)
{
  DyadicTensor t(
      {DenseColumnMatrix({1, 0, 0}), DenseColumnMatrix({0, 1, 0}), DenseColumnMatrix({0, 0, 1})});
  DyadicTensor t2 = t;

  ASSERT_TRUE(t == t2);

  t2(1, 1) = 3;

  ASSERT_TRUE(t != t2);
}

TEST(DyadicTensorTest, EqualsOperatorDouble)
{
  DyadicTensor t(
      {DenseColumnMatrix({1, 0, 0}), DenseColumnMatrix({0, 1, 0}), DenseColumnMatrix({0, 0, 1})});
  DyadicTensor t2(
      {DenseColumnMatrix({5, 5, 5}), DenseColumnMatrix({5, 5, 5}), DenseColumnMatrix({5, 5, 5})});

  t = 5;

  ASSERT_TRUE(t == t2);
}

TEST(DyadicTensorTest, PlusEqualsTensorOperator)
{
  DyadicTensor t({DenseColumnMatrix({2, 8}), DenseColumnMatrix({5, 3})});
  DyadicTensor t2({DenseColumnMatrix({6, 3}), DenseColumnMatrix({4, 6})});
  DyadicTensor expected({DenseColumnMatrix({8, 11}), DenseColumnMatrix({9, 9})});

  DyadicTensor result = t;
  result += t2;

  ASSERT_EQ(expected, result);
}

// Coefficient wise multiplication
TEST(DyadicTensorTest, MultiplyTensorOperator)
{
  DyadicTensor t({DenseColumnMatrix({2, 8}), DenseColumnMatrix({5, 3})});
  DyadicTensor t2({DenseColumnMatrix({6, 3}), DenseColumnMatrix({4, 6})});
  DyadicTensor expected({DenseColumnMatrix({12, 24}), DenseColumnMatrix({20, 18})});

  DyadicTensor result = t * t2;

  ASSERT_EQ(expected, result);
}

// Coefficient wise multiplication
TEST(DyadicTensorTest, MultiplyDoubleOperator)
{
  DyadicTensor t({DenseColumnMatrix({2, 8}), DenseColumnMatrix({5, 3})});
  DyadicTensor expected({DenseColumnMatrix({5, 20}), DenseColumnMatrix({12.5, 10.5})});

  DyadicTensor result = t * 2.5;

  ASSERT_EQ(expected, result);
}

// Coefficient wise multiplication
TEST(DyadicTensorTest, MultiplyEqualsTensorOperator)
{
  DyadicTensor t({DenseColumnMatrix({2, 8}), DenseColumnMatrix({5, 3})});
  DyadicTensor t2({DenseColumnMatrix({6, 3}), DenseColumnMatrix({4, 6})});
  DyadicTensor expected({DenseColumnMatrix({12, 24}), DenseColumnMatrix({20, 18})});

  DyadicTensor result = t;
  result *= t2;

  ASSERT_EQ(expected, result);
}

// Analog Equivalent of matrix multiplication
TEST(DyadicTensorTest, Contraction)
{
  DyadicTensor t({DenseColumnMatrix({2, 8}), DenseColumnMatrix({5, 3})});
  DyadicTensor t2({DenseColumnMatrix({6, 3}), DenseColumnMatrix({4, 6})});
  DyadicTensor expected({DenseColumnMatrix({27, 57}), DenseColumnMatrix({38, 50})});

  DyadicTensor result = t.contract(t2);

  ASSERT_EQ(expected, result);
}

TEST(DyadicTensorTest, MinusOperator)
{
  DyadicTensor t({DenseColumnMatrix({2, 8}), DenseColumnMatrix({5, 3})});
  DyadicTensor t2({DenseColumnMatrix({6, 3}), DenseColumnMatrix({4, 6})});
  DyadicTensor expected({DenseColumnMatrix({-4, 5}), DenseColumnMatrix({1, -3})});

  DyadicTensor result = t - t2;

  ASSERT_EQ(expected, result);
}

TEST(DyadicTensorTest, MinusEqualsOperator)
{
  DyadicTensor t({DenseColumnMatrix({2, 8}), DenseColumnMatrix({5, 3})});
  DyadicTensor t2({DenseColumnMatrix({6, 3}), DenseColumnMatrix({4, 6})});
  DyadicTensor expected({DenseColumnMatrix({-4, 5}), DenseColumnMatrix({1, -3})});

  DyadicTensor result = t;
  result -= t2;

  ASSERT_EQ(expected, result);
}

TEST(DyadicTensorTest, FrobeniusNorm)
{
  DyadicTensor t({DenseColumnMatrix({3, 0}), DenseColumnMatrix({0, 6})});
  ASSERT_EQ(std::sqrt(45), t.frobeniusNorm());
}

TEST(DyadicTensorTest, MaxNorm)
{
  DyadicTensor t({DenseColumnMatrix({3, 0}), DenseColumnMatrix({0, 6})});
  ASSERT_EQ(6, t.maxNorm());
}

TEST(DyadicTensorTest, SetEigens)
{
  DyadicTensor t({DenseColumnMatrix({3, 0}), DenseColumnMatrix({0, 6})});
  std::vector<double> eigvals = {3, 4};
  std::vector<DenseColumnMatrix> eigvecs = {DenseColumnMatrix({0, 1}), DenseColumnMatrix({1, 0})};
  t.setEigens(eigvecs, eigvals);
  ASSERT_EQ(eigvals, t.getEigenvalues());
  ASSERT_EQ(eigvecs, t.getEigenvectors());
}

TEST(DyadicTensorTest, SetEigensFail1)
{
  DyadicTensor t({DenseColumnMatrix({3, 0, 0}), DenseColumnMatrix({0, 0, 6})});
  ASSERT_DEATH(t.setEigens({DenseColumnMatrix({0, 1}), DenseColumnMatrix({1, 0})}, {3, 4, 5}), "");
}

TEST(DyadicTensorTest, SetEigensFail2)
{
  DyadicTensor t({DenseColumnMatrix({3, 0, 0}), DenseColumnMatrix({0, 0, 6})});
  ASSERT_DEATH(t.setEigens({DenseColumnMatrix({0, 0, 1}), DenseColumnMatrix({1, 0, 0}),
                               DenseColumnMatrix({0, 1, 0})},
                   {3, 4}),
      "");
}

TEST(Dyadic3DTensorTest, LinearCertainty)
{
  Dyadic3DTensor t(
      DenseColumnMatrix({1, 0, 0}), DenseColumnMatrix({0, 5, 0}), DenseColumnMatrix({0, 0, 2}));
  ASSERT_EQ(3.0 / 8.0, t.linearCertainty());
}

TEST(Dyadic3DTensorTest, PlanarCertainty)
{
  Dyadic3DTensor t(
      DenseColumnMatrix({1, 0, 0}), DenseColumnMatrix({0, 5, 0}), DenseColumnMatrix({0, 0, 2}));
  ASSERT_EQ(1.0 / 4.0, t.planarCertainty());
}

TEST(Dyadic3DTensorTest, SphericalCertainty)
{
  Dyadic3DTensor t(
      DenseColumnMatrix({1, 0, 0}), DenseColumnMatrix({0, 5, 0}), DenseColumnMatrix({0, 0, 2}));
  ASSERT_EQ(3.0 / 8.0, t.sphericalCertainty());
}

TEST(Dyadic3DTensorTest, CertaintySum)
{
  Dyadic3DTensor t(
      DenseColumnMatrix({1, 0, 0}), DenseColumnMatrix({0, 5, 0}), DenseColumnMatrix({0, 0, 2}));
  ASSERT_EQ(1.0, t.linearCertainty() + t.planarCertainty() + t.sphericalCertainty());
}
