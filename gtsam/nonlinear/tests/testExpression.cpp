/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation,
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------1------------------------------------------- */

/**
 * @file testExpression.cpp
 * @date September 18, 2014
 * @author Frank Dellaert
 * @author Paul Furgale
 * @brief unit tests for Block Automatic Differentiation
 */

#include <gtsam/nonlinear/expressions.h>
#include <gtsam/geometry/Cal3_S2.h>
#include <gtsam/geometry/PinholeCamera.h>
#include <gtsam/geometry/Point3.h>
#include <gtsam/base/Testable.h>

#include <CppUnitLite/TestHarness.h>

#include <boost/assign/list_of.hpp>
using boost::assign::list_of;
using boost::assign::map_list_of;

using namespace std;
using namespace gtsam;

typedef Expression<Point3> Point3_;
typedef Expression<Pose3> Pose3_;
typedef Expression<Rot3> Rot3_;

/* ************************************************************************* */
template <class CAL>
Point2 uncalibrate(const CAL& K, const Point2& p, OptionalJacobian<2, 5> Dcal,
                   OptionalJacobian<2, 2> Dp) {
  return K.uncalibrate(p, Dcal, Dp);
}

static const Rot3 someR = Rot3::RzRyRx(1, 2, 3);

/* ************************************************************************* */
// Constant
TEST(Expression, Constant) {
  Rot3_ R(someR);
  Values values;
  Rot3 actual = R.value(values);
  EXPECT(assert_equal(someR, actual));
  EXPECT_LONGS_EQUAL(0, R.traceSize())
}

/* ************************************************************************* */
// Leaf
TEST(Expression, Leaf) {
  Rot3_ R(100);
  Values values;
  values.insert(100, someR);

  Rot3 actual2 = R.value(values);
  EXPECT(assert_equal(someR, actual2));
}

/* ************************************************************************* */
// Many Leaves
TEST(Expression, Leaves) {
  Values values;
  Point3 somePoint(1, 2, 3);
  values.insert(Symbol('p', 10), somePoint);
  std::vector<Point3_> points = createUnknowns<Point3>(10, 'p', 1);
  EXPECT(assert_equal(somePoint, points.back().value(values)));
}

/* ************************************************************************* */
// Unary(Leaf)
namespace unary {
Point2 f1(const Point3& p, OptionalJacobian<2, 3> H) {
  return Point2();
}
double f2(const Point3& p, OptionalJacobian<1, 3> H) {
  return 0.0;
}
Vector f3(const Point3& p, OptionalJacobian<Eigen::Dynamic, 3> H) {
  return p;
}
Point3_ p(1);
set<Key> expected = list_of(1);
}  // namespace unary

TEST(Expression, Unary1) {
  using namespace unary;
  Expression<Point2> e(f1, p);
  EXPECT(expected == e.keys());
}
TEST(Expression, Unary2) {
  using namespace unary;
  Double_ e(f2, p);
  EXPECT(expected == e.keys());
}

/* ************************************************************************* */
// Unary(Leaf), dynamic
TEST(Expression, Unary3) {
  using namespace unary;
  //  Expression<Vector> e(f3, p);
}

/* ************************************************************************* */
// Nullary Method
TEST(Expression, NullaryMethod) {
  // Create expression
  Expression<Point3> p(67);
  Expression<double> norm(&gtsam::norm, p);

  // Create Values
  Values values;
  values.insert(67, Point3(3, 4, 5));

  // Check dims as map
  std::map<Key, int> map;
  norm.dims(map);
  LONGS_EQUAL(1, map.size());

  // Get value and Jacobians
  std::vector<Matrix> H(1);
  double actual = norm.value(values, H);

  // Check all
  EXPECT(actual == sqrt(50));
  Matrix expected(1, 3);
  expected << 3.0 / sqrt(50.0), 4.0 / sqrt(50.0), 5.0 / sqrt(50.0);
  EXPECT(assert_equal(expected, H[0]));
}

/* ************************************************************************* */
// Binary(Leaf,Leaf)
namespace binary {
// Create leaves
double doubleF(const Pose3& pose,  //
               const Point3& point, OptionalJacobian<1, 6> H1, OptionalJacobian<1, 3> H2) {
  return 0.0;
}
Pose3_ x(1);
Point3_ p(2);
Point3_ p_cam(x, &Pose3::transform_to, p);
}

/* ************************************************************************* */
// Check that creating an expression to double compiles
TEST(Expression, BinaryToDouble) {
  using namespace binary;
  Double_ p_cam(doubleF, x, p);
}

/* ************************************************************************* */
// keys
TEST(Expression, BinaryKeys) {
  set<Key> expected = list_of(1)(2);
  EXPECT(expected == binary::p_cam.keys());
}

/* ************************************************************************* */
// dimensions
TEST(Expression, BinaryDimensions) {
  map<Key, int> actual, expected = map_list_of<Key, int>(1, 6)(2, 3);
  binary::p_cam.dims(actual);
  EXPECT(actual == expected);
}

/* ************************************************************************* */
// dimensions
TEST(Expression, BinaryTraceSize) {
  typedef internal::BinaryExpression<Point3, Pose3, Point3> Binary;
  size_t expectedTraceSize = sizeof(Binary::Record);
  EXPECT_LONGS_EQUAL(expectedTraceSize, binary::p_cam.traceSize());
}

/* ************************************************************************* */
// Binary(Leaf,Unary(Binary(Leaf,Leaf)))
namespace tree {
using namespace binary;
// Create leaves
Expression<Cal3_S2> K(3);

// Create expression tree
Point2 (*f)(const Point3&, OptionalJacobian<2, 3>) = &PinholeBase::Project;
Expression<Point2> projection(f, p_cam);
Expression<Point2> uv_hat(uncalibrate<Cal3_S2>, K, projection);
}

/* ************************************************************************* */
// keys
TEST(Expression, TreeKeys) {
  set<Key> expected = list_of(1)(2)(3);
  EXPECT(expected == tree::uv_hat.keys());
}

/* ************************************************************************* */
// dimensions
TEST(Expression, TreeDimensions) {
  map<Key, int> actual, expected = map_list_of<Key, int>(1, 6)(2, 3)(3, 5);
  tree::uv_hat.dims(actual);
  EXPECT(actual == expected);
}

/* ************************************************************************* */
// TraceSize
TEST(Expression, TreeTraceSize) {
  typedef internal::BinaryExpression<Point3, Pose3, Point3> Binary1;
  EXPECT_LONGS_EQUAL(internal::upAligned(sizeof(Binary1::Record)), tree::p_cam.traceSize());

  typedef internal::UnaryExpression<Point2, Point3> Unary;
  EXPECT_LONGS_EQUAL(internal::upAligned(sizeof(Unary::Record)) + tree::p_cam.traceSize(),
                     tree::projection.traceSize());

  EXPECT_LONGS_EQUAL(0, tree::K.traceSize());

  typedef internal::BinaryExpression<Point2, Cal3_S2, Point2> Binary2;
  EXPECT_LONGS_EQUAL(internal::upAligned(sizeof(Binary2::Record)) + tree::K.traceSize() +
                         tree::projection.traceSize(),
                     tree::uv_hat.traceSize());
}

/* ************************************************************************* */
TEST(Expression, compose1) {
  // Create expression
  Rot3_ R1(1), R2(2);
  Rot3_ R3 = R1 * R2;

  // Check keys
  set<Key> expected = list_of(1)(2);
  EXPECT(expected == R3.keys());
}

/* ************************************************************************* */
// Test compose with arguments referring to the same rotation
TEST(Expression, compose2) {
  // Create expression
  Rot3_ R1(1), R2(1);
  Rot3_ R3 = R1 * R2;

  // Check keys
  set<Key> expected = list_of(1);
  EXPECT(expected == R3.keys());
}

/* ************************************************************************* */
// Test compose with one arguments referring to constant rotation
TEST(Expression, compose3) {
  // Create expression
  Rot3_ R1(Rot3::identity()), R2(3);
  Rot3_ R3 = R1 * R2;

  // Check keys
  set<Key> expected = list_of(3);
  EXPECT(expected == R3.keys());
}

/* ************************************************************************* */
// Test with ternary function
Rot3 composeThree(const Rot3& R1, const Rot3& R2, const Rot3& R3, OptionalJacobian<3, 3> H1,
                  OptionalJacobian<3, 3> H2, OptionalJacobian<3, 3> H3) {
  // return dummy derivatives (not correct, but that's ok for testing here)
  if (H1)
    *H1 = eye(3);
  if (H2)
    *H2 = eye(3);
  if (H3)
    *H3 = eye(3);
  return R1 * (R2 * R3);
}

TEST(Expression, ternary) {
  // Create expression
  Rot3_ A(1), B(2), C(3);
  Rot3_ ABC(composeThree, A, B, C);

  // Check keys
  set<Key> expected = list_of(1)(2)(3);
  EXPECT(expected == ABC.keys());
}

/* ************************************************************************* */
TEST(Expression, ScalarMultiply) {
  const Key key(67);
  const Point3_ sum_ = 23 * Point3_(key);

  set<Key> expected_keys = list_of(key);
  EXPECT(expected_keys == sum_.keys());

  map<Key, int> actual_dims, expected_dims = map_list_of<Key, int>(key, 3);
  sum_.dims(actual_dims);
  EXPECT(actual_dims == expected_dims);

  // Check dims as map
  std::map<Key, int> map;
  sum_.dims(map);
  LONGS_EQUAL(1, map.size());

  Values values;
  values.insert<Point3>(key, Point3(1, 0, 2));

  // Check value
  const Point3 expected(23, 0, 46);
  EXPECT(assert_equal(expected, sum_.value(values)));

  // Check value + Jacobians
  std::vector<Matrix> H(1);
  EXPECT(assert_equal(expected, sum_.value(values, H)));
  EXPECT(assert_equal(23 * I_3x3, H[0]));
}

/* ************************************************************************* */
TEST(Expression, Sum) {
  const Key key(67);
  const Point3_ sum_ = Point3_(key) + Point3_(Point3(1, 1, 1));

  set<Key> expected_keys = list_of(key);
  EXPECT(expected_keys == sum_.keys());

  map<Key, int> actual_dims, expected_dims = map_list_of<Key, int>(key, 3);
  sum_.dims(actual_dims);
  EXPECT(actual_dims == expected_dims);

  // Check dims as map
  std::map<Key, int> map;
  sum_.dims(map);
  LONGS_EQUAL(1, map.size());

  Values values;
  values.insert<Point3>(key, Point3(2, 2, 2));

  // Check value
  const Point3 expected(3, 3, 3);
  EXPECT(assert_equal(expected, sum_.value(values)));

  // Check value + Jacobians
  std::vector<Matrix> H(1);
  EXPECT(assert_equal(expected, sum_.value(values, H)));
  EXPECT(assert_equal(I_3x3, H[0]));
}

/* ************************************************************************* */
TEST(Expression, TripleSum) {
  const Key key(67);
  const Point3_ p1_(Point3(1, 1, 1)), p2_(key);
  const SumExpression<Point3> sum_ = p1_ + p2_ + p1_;

  LONGS_EQUAL(3, sum_.nrTerms());
  LONGS_EQUAL(1, sum_.keys().size());

  Values values;
  values.insert<Point3>(key, Point3(2, 2, 2));

  // Check value
  const Point3 expected(4, 4, 4);
  EXPECT(assert_equal(expected, sum_.value(values)));

  // Check value + Jacobians
  std::vector<Matrix> H(1);
  EXPECT(assert_equal(expected, sum_.value(values, H)));
  EXPECT(assert_equal(I_3x3, H[0]));
}

/* ************************************************************************* */
TEST(Expression, SumOfUnaries) {
  const Key key(67);
  const Double_ norm_(&gtsam::norm, Point3_(key));
  const Double_ sum_ = norm_ + norm_;

  Values values;
  values.insert<Point3>(key, Point3(6, 0, 0));

  // Check value
  EXPECT_DOUBLES_EQUAL(12, sum_.value(values), 1e-9);

  // Check value + Jacobians
  std::vector<Matrix> H(1);
  EXPECT_DOUBLES_EQUAL(12, sum_.value(values, H), 1e-9);
  EXPECT(assert_equal(Vector3(2, 0, 0).transpose(), H[0]));
}

/* ************************************************************************* */
TEST(Expression, UnaryOfSum) {
  const Key key1(42), key2(67);
  const Point3_ sum_ = Point3_(key1) + Point3_(key2);
  const Double_ norm_(&gtsam::norm, sum_);

  map<Key, int> actual_dims, expected_dims = map_list_of<Key, int>(key1, 3)(key2, 3);
  norm_.dims(actual_dims);
  EXPECT(actual_dims == expected_dims);

  Values values;
  values.insert<Point3>(key1, Point3(1, 0, 0));
  values.insert<Point3>(key2, Point3(0, 1, 0));

  // Check value
  EXPECT_DOUBLES_EQUAL(sqrt(2), norm_.value(values), 1e-9);

  // Check value + Jacobians
  std::vector<Matrix> H(2);
  EXPECT_DOUBLES_EQUAL(sqrt(2), norm_.value(values, H), 1e-9);
  EXPECT(assert_equal(0.5 * sqrt(2) * Vector3(1, 1, 0).transpose(), H[0]));
  EXPECT(assert_equal(0.5 * sqrt(2) * Vector3(1, 1, 0).transpose(), H[1]));
}

/* ************************************************************************* */
TEST(Expression, WeightedSum) {
  const Key key1(42), key2(67);
  const Point3_ weighted_sum_ = 17 * Point3_(key1) + 23 * Point3_(key2);

  map<Key, int> actual_dims, expected_dims = map_list_of<Key, int>(key1, 3)(key2, 3);
  weighted_sum_.dims(actual_dims);
  EXPECT(actual_dims == expected_dims);

  Values values;
  values.insert<Point3>(key1, Point3(1, 0, 0));
  values.insert<Point3>(key2, Point3(0, 1, 0));

  // Check value
  const Point3 expected = 17 * Point3(1, 0, 0) + 23 * Point3(0, 1, 0);
  EXPECT(assert_equal(expected, weighted_sum_.value(values)));

  // Check value + Jacobians
  std::vector<Matrix> H(2);
  EXPECT(assert_equal(expected, weighted_sum_.value(values, H)));
  EXPECT(assert_equal(17 * I_3x3, H[0]));
  EXPECT(assert_equal(23 * I_3x3, H[1]));
}

/* ************************************************************************* */
TEST(Expression, Subtract) {
  const Vector3 p = Vector3::Random(), q = Vector3::Random();
  Values values;
  values.insert(0, p);
  values.insert(1, q);
  const Vector3_ expression = Vector3_(0) - Vector3_(1);
  set<Key> expected_keys = {0, 1};
  EXPECT(expression.keys() == expected_keys);

  // Check value + Jacobians
  std::vector<Matrix> H(2);
  EXPECT(assert_equal<Vector3>(p - q, expression.value(values, H)));
  EXPECT(assert_equal(I_3x3, H[0]));
  EXPECT(assert_equal(-I_3x3, H[1]));
}

/* ************************************************************************* */
TEST(Expression, LinearExpression) {
  const Key key(67);
  const boost::function<Vector3(Point3)> f = [](const Point3& p) { return (Vector3)p; };
  const Matrix3 kIdentity = I_3x3;
  const Expression<Vector3> linear_ = linearExpression(f, Point3_(key), kIdentity);

  Values values;
  values.insert<Point3>(key, Point3(1, 0, 2));

  // Check value
  const Vector3 expected(1, 0, 2);
  EXPECT(assert_equal(expected, linear_.value(values)));

  // Check value + Jacobians
  std::vector<Matrix> H(1);
  EXPECT(assert_equal(expected, linear_.value(values, H)));
  EXPECT(assert_equal(I_3x3, H[0]));
}

/* ************************************************************************* */
int main() {
  TestResult tr;
  return TestRegistry::runAllTests(tr);
}
/* ************************************************************************* */
