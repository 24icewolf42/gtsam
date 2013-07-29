/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file    testVectorValues.cpp
 * @author  Richard Roberts
 * @date    Sep 16, 2010
 */

#include <boost/assign/std/vector.hpp>

#include <gtsam/base/Testable.h>
#include <gtsam/linear/VectorValuesOrdered.h>
#include <gtsam/inference/PermutationOrdered.h>

#include <CppUnitLite/TestHarness.h>

using namespace std;
using namespace boost::assign;
using namespace gtsam;

/* ************************************************************************* */
TEST(VectorValuesOrdered, insert) {

  // insert, with out-of-order indices
  VectorValuesOrdered actual;
  actual.insert(0, Vector_(1, 1.0));
  actual.insert(1, Vector_(2, 2.0, 3.0));
  actual.insert(5, Vector_(2, 6.0, 7.0));
  actual.insert(2, Vector_(2, 4.0, 5.0));

  // Check dimensions
  LONGS_EQUAL(6, actual.size());
  LONGS_EQUAL(1, actual.dim(0));
  LONGS_EQUAL(2, actual.dim(1));
  LONGS_EQUAL(2, actual.dim(2));
  LONGS_EQUAL(2, actual.dim(5));

  // Logic
  EXPECT(actual.exists(0));
  EXPECT(actual.exists(1));
  EXPECT(actual.exists(2));
  EXPECT(!actual.exists(3));
  EXPECT(!actual.exists(4));
  EXPECT(actual.exists(5));
  EXPECT(!actual.exists(6));

  // Check values
  EXPECT(assert_equal(Vector_(1, 1.0), actual[0]));
  EXPECT(assert_equal(Vector_(2, 2.0, 3.0), actual[1]));
  EXPECT(assert_equal(Vector_(2, 4.0, 5.0), actual[2]));
  EXPECT(assert_equal(Vector_(2, 6.0, 7.0), actual[5]));
  EXPECT(assert_equal(Vector_(7, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0), actual.asVector()));

  // Check exceptions
  CHECK_EXCEPTION(actual.insert(1, Vector()), invalid_argument);
  CHECK_EXCEPTION(actual.dim(3), out_of_range);
}

/* ************************************************************************* */
TEST(VectorValuesOrdered, dimsConstructor) {
  vector<size_t> dims;
  dims.push_back(1);
  dims.push_back(2);
  dims.push_back(2);

  VectorValuesOrdered actual(dims);
  actual[0] = Vector_(1, 1.0);
  actual[1] = Vector_(2, 2.0, 3.0);
  actual[2] = Vector_(2, 4.0, 5.0);

  // Check dimensions
  LONGS_EQUAL(3, actual.size());
  LONGS_EQUAL(1, actual.dim(0));
  LONGS_EQUAL(2, actual.dim(1));
  LONGS_EQUAL(2, actual.dim(2));

  // Check values
  EXPECT(assert_equal(Vector_(1, 1.0), actual[0]));
  EXPECT(assert_equal(Vector_(2, 2.0, 3.0), actual[1]));
  EXPECT(assert_equal(Vector_(2, 4.0, 5.0), actual[2]));
  EXPECT(assert_equal(Vector_(5, 1.0, 2.0, 3.0, 4.0, 5.0), actual.asVector()));
}

/* ************************************************************************* */
TEST(VectorValuesOrdered, copyConstructor) {

  // insert, with out-of-order indices
  VectorValuesOrdered original;
  original.insert(0, Vector_(1, 1.0));
  original.insert(1, Vector_(2, 2.0, 3.0));
  original.insert(5, Vector_(2, 6.0, 7.0));
  original.insert(2, Vector_(2, 4.0, 5.0));

  VectorValuesOrdered actual(original);

  // Check dimensions
  LONGS_EQUAL(6, actual.size());
  LONGS_EQUAL(1, actual.dim(0));
  LONGS_EQUAL(2, actual.dim(1));
  LONGS_EQUAL(2, actual.dim(2));
  LONGS_EQUAL(2, actual.dim(5));

  // Logic
  EXPECT(actual.exists(0));
  EXPECT(actual.exists(1));
  EXPECT(actual.exists(2));
  EXPECT(!actual.exists(3));
  EXPECT(!actual.exists(4));
  EXPECT(actual.exists(5));
  EXPECT(!actual.exists(6));

  // Check values
  EXPECT(assert_equal(Vector_(1, 1.0), actual[0]));
  EXPECT(assert_equal(Vector_(2, 2.0, 3.0), actual[1]));
  EXPECT(assert_equal(Vector_(2, 4.0, 5.0), actual[2]));
  EXPECT(assert_equal(Vector_(2, 6.0, 7.0), actual[5]));
  EXPECT(assert_equal(Vector_(7, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0), actual.asVector()));

  // Check exceptions
  CHECK_EXCEPTION(actual.insert(1, Vector()), invalid_argument);
}

/* ************************************************************************* */
TEST(VectorValuesOrdered, assignment) {

  VectorValuesOrdered actual;

  {
    // insert, with out-of-order indices
    VectorValuesOrdered original;
    original.insert(0, Vector_(1, 1.0));
    original.insert(1, Vector_(2, 2.0, 3.0));
    original.insert(5, Vector_(2, 6.0, 7.0));
    original.insert(2, Vector_(2, 4.0, 5.0));
    actual = original;
  }

  // Check dimensions
  LONGS_EQUAL(6, actual.size());
  LONGS_EQUAL(1, actual.dim(0));
  LONGS_EQUAL(2, actual.dim(1));
  LONGS_EQUAL(2, actual.dim(2));
  LONGS_EQUAL(2, actual.dim(5));

  // Logic
  EXPECT(actual.exists(0));
  EXPECT(actual.exists(1));
  EXPECT(actual.exists(2));
  EXPECT(!actual.exists(3));
  EXPECT(!actual.exists(4));
  EXPECT(actual.exists(5));
  EXPECT(!actual.exists(6));

  // Check values
  EXPECT(assert_equal(Vector_(1, 1.0), actual[0]));
  EXPECT(assert_equal(Vector_(2, 2.0, 3.0), actual[1]));
  EXPECT(assert_equal(Vector_(2, 4.0, 5.0), actual[2]));
  EXPECT(assert_equal(Vector_(2, 6.0, 7.0), actual[5]));
  EXPECT(assert_equal(Vector_(7, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0), actual.asVector()));

  // Check exceptions
  CHECK_EXCEPTION(actual.insert(1, Vector()), invalid_argument);
}

/* ************************************************************************* */
TEST(VectorValuesOrdered, SameStructure) {
  // insert, with out-of-order indices
  VectorValuesOrdered original;
  original.insert(0, Vector_(1, 1.0));
  original.insert(1, Vector_(2, 2.0, 3.0));
  original.insert(5, Vector_(2, 6.0, 7.0));
  original.insert(2, Vector_(2, 4.0, 5.0));

  VectorValuesOrdered actual(VectorValuesOrdered::SameStructure(original));

  // Check dimensions
  LONGS_EQUAL(6, actual.size());
  LONGS_EQUAL(1, actual.dim(0));
  LONGS_EQUAL(2, actual.dim(1));
  LONGS_EQUAL(2, actual.dim(2));
  LONGS_EQUAL(2, actual.dim(5));

  // Logic
  EXPECT(actual.exists(0));
  EXPECT(actual.exists(1));
  EXPECT(actual.exists(2));
  EXPECT(!actual.exists(3));
  EXPECT(!actual.exists(4));
  EXPECT(actual.exists(5));
  EXPECT(!actual.exists(6));

  // Check exceptions
  CHECK_EXCEPTION(actual.insert(1, Vector()), invalid_argument);
}

/* ************************************************************************* */
TEST(VectorValuesOrdered, Zero_fromModel) {
  // insert, with out-of-order indices
  VectorValuesOrdered original;
  original.insert(0, Vector_(1, 1.0));
  original.insert(1, Vector_(2, 2.0, 3.0));
  original.insert(5, Vector_(2, 6.0, 7.0));
  original.insert(2, Vector_(2, 4.0, 5.0));

  VectorValuesOrdered actual(VectorValuesOrdered::Zero(original));

  // Check dimensions
  LONGS_EQUAL(6, actual.size());
  LONGS_EQUAL(1, actual.dim(0));
  LONGS_EQUAL(2, actual.dim(1));
  LONGS_EQUAL(2, actual.dim(2));
  LONGS_EQUAL(2, actual.dim(5));

  // Values
  EXPECT(assert_equal(Vector::Zero(1), actual[0]));
  EXPECT(assert_equal(Vector::Zero(2), actual[1]));
  EXPECT(assert_equal(Vector::Zero(2), actual[5]));
  EXPECT(assert_equal(Vector::Zero(2), actual[2]));

  // Logic
  EXPECT(actual.exists(0));
  EXPECT(actual.exists(1));
  EXPECT(actual.exists(2));
  EXPECT(!actual.exists(3));
  EXPECT(!actual.exists(4));
  EXPECT(actual.exists(5));
  EXPECT(!actual.exists(6));

  // Check exceptions
  CHECK_EXCEPTION(actual.insert(1, Vector()), invalid_argument);
}

/* ************************************************************************* */
TEST(VectorValuesOrdered, Zero_fromDims) {
  vector<size_t> dims;
  dims.push_back(1);
  dims.push_back(2);
  dims.push_back(2);

  VectorValuesOrdered actual(VectorValuesOrdered::Zero(dims));

  // Check dimensions
  LONGS_EQUAL(3, actual.size());
  LONGS_EQUAL(1, actual.dim(0));
  LONGS_EQUAL(2, actual.dim(1));
  LONGS_EQUAL(2, actual.dim(2));

  // Values
  EXPECT(assert_equal(Vector::Zero(1), actual[0]));
  EXPECT(assert_equal(Vector::Zero(2), actual[1]));
  EXPECT(assert_equal(Vector::Zero(2), actual[2]));
}

/* ************************************************************************* */
TEST(VectorValuesOrdered, Zero_fromUniform) {
  VectorValuesOrdered actual(VectorValuesOrdered::Zero(3, 2));

  // Check dimensions
  LONGS_EQUAL(3, actual.size());
  LONGS_EQUAL(2, actual.dim(0));
  LONGS_EQUAL(2, actual.dim(1));
  LONGS_EQUAL(2, actual.dim(2));

  // Values
  EXPECT(assert_equal(Vector::Zero(2), actual[0]));
  EXPECT(assert_equal(Vector::Zero(2), actual[1]));
  EXPECT(assert_equal(Vector::Zero(2), actual[2]));
}

/* ************************************************************************* */
TEST(VectorValuesOrdered, resizeLike) {
  // insert, with out-of-order indices
  VectorValuesOrdered original;
  original.insert(0, Vector_(1, 1.0));
  original.insert(1, Vector_(2, 2.0, 3.0));
  original.insert(5, Vector_(2, 6.0, 7.0));
  original.insert(2, Vector_(2, 4.0, 5.0));

  VectorValuesOrdered actual(10, 3);
  actual.resizeLike(original);

  // Check dimensions
  LONGS_EQUAL(6, actual.size());
  LONGS_EQUAL(1, actual.dim(0));
  LONGS_EQUAL(2, actual.dim(1));
  LONGS_EQUAL(2, actual.dim(2));
  LONGS_EQUAL(2, actual.dim(5));

  // Logic
  EXPECT(actual.exists(0));
  EXPECT(actual.exists(1));
  EXPECT(actual.exists(2));
  EXPECT(!actual.exists(3));
  EXPECT(!actual.exists(4));
  EXPECT(actual.exists(5));
  EXPECT(!actual.exists(6));

  // Check exceptions
  CHECK_EXCEPTION(actual.insert(1, Vector()), invalid_argument);
}

/* ************************************************************************* */
TEST(VectorValuesOrdered, resize_fromUniform) {
  VectorValuesOrdered actual(4, 10);
  actual.resize(3, 2);

  actual[0] = Vector_(2, 1.0, 2.0);
  actual[1] = Vector_(2, 2.0, 3.0);
  actual[2] = Vector_(2, 4.0, 5.0);

  // Check dimensions
  LONGS_EQUAL(3, actual.size());
  LONGS_EQUAL(2, actual.dim(0));
  LONGS_EQUAL(2, actual.dim(1));
  LONGS_EQUAL(2, actual.dim(2));

  // Check values
  EXPECT(assert_equal(Vector_(2, 1.0, 2.0), actual[0]));
  EXPECT(assert_equal(Vector_(2, 2.0, 3.0), actual[1]));
  EXPECT(assert_equal(Vector_(2, 4.0, 5.0), actual[2]));
  EXPECT(assert_equal(Vector_(6, 1.0, 2.0, 2.0, 3.0, 4.0, 5.0), actual.asVector()));
}

/* ************************************************************************* */
TEST(VectorValuesOrdered, resize_fromDims) {
  vector<size_t> dims;
  dims.push_back(1);
  dims.push_back(2);
  dims.push_back(2);

  VectorValuesOrdered actual(4, 10);
  actual.resize(dims);
  actual[0] = Vector_(1, 1.0);
  actual[1] = Vector_(2, 2.0, 3.0);
  actual[2] = Vector_(2, 4.0, 5.0);

  // Check dimensions
  LONGS_EQUAL(3, actual.size());
  LONGS_EQUAL(1, actual.dim(0));
  LONGS_EQUAL(2, actual.dim(1));
  LONGS_EQUAL(2, actual.dim(2));

  // Check values
  EXPECT(assert_equal(Vector_(1, 1.0), actual[0]));
  EXPECT(assert_equal(Vector_(2, 2.0, 3.0), actual[1]));
  EXPECT(assert_equal(Vector_(2, 4.0, 5.0), actual[2]));
  EXPECT(assert_equal(Vector_(5, 1.0, 2.0, 3.0, 4.0, 5.0), actual.asVector()));
}

/* ************************************************************************* */
TEST(VectorValuesOrdered, append) {
  // insert
  VectorValuesOrdered actual;
  actual.insert(0, Vector_(1, 1.0));
  actual.insert(1, Vector_(2, 2.0, 3.0));
  actual.insert(2, Vector_(2, 4.0, 5.0));

  // append
  vector<size_t> dims(2);
  dims[0] = 3;
  dims[1] = 5;
  actual.append(dims);

  // Check dimensions
  LONGS_EQUAL(5, actual.size());
  LONGS_EQUAL(1, actual.dim(0));
  LONGS_EQUAL(2, actual.dim(1));
  LONGS_EQUAL(2, actual.dim(2));
  LONGS_EQUAL(3, actual.dim(3));
  LONGS_EQUAL(5, actual.dim(4));

  // Logic
  EXPECT(actual.exists(0));
  EXPECT(actual.exists(1));
  EXPECT(actual.exists(2));
  EXPECT(actual.exists(3));
  EXPECT(actual.exists(4));
  EXPECT(!actual.exists(5));

  // Check values
  EXPECT(assert_equal(Vector_(1, 1.0), actual[0]));
  EXPECT(assert_equal(Vector_(2, 2.0, 3.0), actual[1]));
  EXPECT(assert_equal(Vector_(2, 4.0, 5.0), actual[2]));

  // Check exceptions
  CHECK_EXCEPTION(actual.insert(3, Vector()), invalid_argument);
}

/* ************************************************************************* */
TEST(VectorValuesOrdered, hasSameStructure) {
  VectorValuesOrdered v1(2, 3);
  VectorValuesOrdered v2(3, 2);
  VectorValuesOrdered v3(4, 2);
  VectorValuesOrdered v4(4, 2);

  EXPECT(!v1.hasSameStructure(v2));
  EXPECT(!v2.hasSameStructure(v3));
  EXPECT(v3.hasSameStructure(v4));
  EXPECT(VectorValuesOrdered().hasSameStructure(VectorValuesOrdered()));
  EXPECT(!v1.hasSameStructure(VectorValuesOrdered()));
}


/* ************************************************************************* */
TEST(VectorValuesOrdered, permute) {

  VectorValuesOrdered original;
  original.insert(0, Vector_(1, 1.0));
  original.insert(1, Vector_(2, 2.0, 3.0));
  original.insert(2, Vector_(2, 4.0, 5.0));
  original.insert(3, Vector_(2, 6.0, 7.0));

  VectorValuesOrdered expected;
  expected.insert(0, Vector_(2, 4.0, 5.0)); // from 2
  expected.insert(1, Vector_(1, 1.0)); // from 0
  expected.insert(2, Vector_(2, 6.0, 7.0)); // from 3
  expected.insert(3, Vector_(2, 2.0, 3.0)); // from 1

  Permutation permutation(4);
  permutation[0] = 2;
  permutation[1] = 0;
  permutation[2] = 3;
  permutation[3] = 1;

  VectorValuesOrdered actual = original;
  actual.permuteInPlace(permutation);

  EXPECT(assert_equal(expected, actual));
}

/* ************************************************************************* */
TEST(VectorValuesOrdered, subvector) {
  VectorValuesOrdered init;
  init.insert(0, Vector_(1, 1.0));
  init.insert(1, Vector_(2, 2.0, 3.0));
  init.insert(2, Vector_(2, 4.0, 5.0));
  init.insert(3, Vector_(2, 6.0, 7.0));

  std::vector<gtsam::Index> indices;
  indices += 0, 2, 3;
  Vector expSubVector = Vector_(5, 1.0, 4.0, 5.0, 6.0, 7.0);
  EXPECT(assert_equal(expSubVector, init.vector(indices)));
}

/* ************************************************************************* */
int main() { TestResult tr; return TestRegistry::runAllTests(tr); }
/* ************************************************************************* */
