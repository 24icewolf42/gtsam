/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file testExpressionFactor.cpp
 * @date September 18, 2014
 * @author Frank Dellaert
 * @author Paul Furgale
 * @brief unit tests for Block Automatic Differentiation
 */

#include <gtsam/slam/expressions.h>
#include <gtsam/slam/GeneralSFMFactor.h>
#include <gtsam/slam/ProjectionFactor.h>
#include <gtsam/slam/PriorFactor.h>
#include <gtsam/nonlinear/expressionTesting.h>
#include <gtsam/nonlinear/ExpressionFactor.h>
#include <gtsam/nonlinear/NonlinearFactorGraph.h>
#include <gtsam/nonlinear/expressionTesting.h>
#include <gtsam/base/Testable.h>

#include <CppUnitLite/TestHarness.h>

#include <boost/assign/list_of.hpp>
using boost::assign::list_of;

using namespace std;
using namespace gtsam;

Point2 measured(-17, 30);
SharedNoiseModel model = noiseModel::Unit::Create(2);

// This deals with the overload problem and makes the expressions factor
// understand that we work on Point3
Point2 (*Project)(const Point3&, OptionalJacobian<2, 3>) = &PinholeBase::Project;

namespace leaf {
// Create some values
struct MyValues: public Values {
  MyValues() {
    insert(2, Point2(3, 5));
  }
} values;

// Create leaf
Point2_ p(2);
}

/* ************************************************************************* */
// Leaf
TEST(ExpressionFactor, Leaf) {
  using namespace leaf;

  // Create old-style factor to create expected value and derivatives
  PriorFactor<Point2> old(2, Point2(0, 0), model);

  // Concise version
  ExpressionFactor<Point2> f(model, Point2(0, 0), p);
  EXPECT_DOUBLES_EQUAL(old.error(values), f.error(values), 1e-9);
  EXPECT_LONGS_EQUAL(2, f.dim());
  boost::shared_ptr<GaussianFactor> gf2 = f.linearize(values);
  EXPECT( assert_equal(*old.linearize(values), *gf2, 1e-9));
}

/* ************************************************************************* */
// non-zero noise model
TEST(ExpressionFactor, Model) {
  using namespace leaf;

  SharedNoiseModel model = noiseModel::Diagonal::Sigmas(Vector2(0.1, 0.01));

  // Create old-style factor to create expected value and derivatives
  PriorFactor<Point2> old(2, Point2(0, 0), model);

  // Concise version
  ExpressionFactor<Point2> f(model, Point2(0, 0), p);

  // Check values and derivatives
  EXPECT_DOUBLES_EQUAL(old.error(values), f.error(values), 1e-9);
  EXPECT_LONGS_EQUAL(2, f.dim());
  boost::shared_ptr<GaussianFactor> gf2 = f.linearize(values);
  EXPECT( assert_equal(*old.linearize(values), *gf2, 1e-9));
  EXPECT_CORRECT_FACTOR_JACOBIANS(f, values, 1e-5, 1e-5); // another way
}

/* ************************************************************************* */
// Constrained noise model
TEST(ExpressionFactor, Constrained) {
  using namespace leaf;

  SharedDiagonal model = noiseModel::Constrained::MixedSigmas(Vector2(0.2, 0));

  // Create old-style factor to create expected value and derivatives
  PriorFactor<Point2> old(2, Point2(0, 0), model);

  // Concise version
  ExpressionFactor<Point2> f(model, Point2(0, 0), p);
  EXPECT_DOUBLES_EQUAL(old.error(values), f.error(values), 1e-9);
  EXPECT_LONGS_EQUAL(2, f.dim());
  boost::shared_ptr<GaussianFactor> gf2 = f.linearize(values);
  EXPECT( assert_equal(*old.linearize(values), *gf2, 1e-9));
}

/* ************************************************************************* */
// Unary(Leaf))
TEST(ExpressionFactor, Unary) {

  // Create some values
  Values values;
  values.insert(2, Point3(0, 0, 1));

  JacobianFactor expected( //
      2, (Matrix(2, 3) << 1, 0, 0, 0, 1, 0).finished(), //
      Vector2(-17, 30));

  // Create leaves
  Point3_ p(2);

  // Concise version
  ExpressionFactor<Point2> f(model, measured, project(p));
  EXPECT_LONGS_EQUAL(2, f.dim());
  boost::shared_ptr<GaussianFactor> gf = f.linearize(values);
  boost::shared_ptr<JacobianFactor> jf = //
      boost::dynamic_pointer_cast<JacobianFactor>(gf);
  EXPECT( assert_equal(expected, *jf, 1e-9));
}

/* ************************************************************************* */
// Unary(Leaf)) and Unary(Unary(Leaf)))
// wide version (not handled in fixed-size pipeline)
typedef Eigen::Matrix<double,9,3> Matrix93;
Vector9 wide(const Point3& p, OptionalJacobian<9,3> H) {
  Vector9 v;
  v << p.vector(), p.vector(), p.vector();
  if (H) *H << eye(3), eye(3), eye(3);
  return v;
}
typedef Eigen::Matrix<double,9,9> Matrix9;
Vector9 id9(const Vector9& v, OptionalJacobian<9,9> H) {
  if (H) *H = Matrix9::Identity();
  return v;
}
TEST(ExpressionFactor, Wide) {
  // Create some values
  Values values;
  values.insert(2, Point3(0, 0, 1));
  Point3_ point(2);
  Vector9 measured;
  measured.setZero();
  Expression<Vector9> expression(wide,point);
  SharedNoiseModel model = noiseModel::Unit::Create(9);

  ExpressionFactor<Vector9> f1(model, measured, expression);
  EXPECT_CORRECT_FACTOR_JACOBIANS(f1, values, 1e-5, 1e-9);

  Expression<Vector9> expression2(id9,expression);
  ExpressionFactor<Vector9> f2(model, measured, expression2);
  EXPECT_CORRECT_FACTOR_JACOBIANS(f2, values, 1e-5, 1e-9);
}

/* ************************************************************************* */
static Point2 myUncal(const Cal3_S2& K, const Point2& p,
    OptionalJacobian<2,5> Dcal, OptionalJacobian<2,2> Dp) {
  return K.uncalibrate(p, Dcal, Dp);
}

// Binary(Leaf,Leaf)
TEST(ExpressionFactor, Binary) {

  typedef internal::BinaryExpression<Point2, Cal3_S2, Point2> Binary;

  Cal3_S2_ K_(1);
  Point2_ p_(2);
  Binary binary(myUncal, K_, p_);

  // Create some values
  Values values;
  values.insert(1, Cal3_S2());
  values.insert(2, Point2(0, 0));

  // traceRaw will fill raw with [Trace<Point2> | Binary::Record]
  EXPECT_LONGS_EQUAL(8, sizeof(double));
  EXPECT_LONGS_EQUAL(16, sizeof(Point2));
  EXPECT_LONGS_EQUAL(40, sizeof(Cal3_S2));
  EXPECT_LONGS_EQUAL(16, sizeof(internal::ExecutionTrace<Point2>));
  EXPECT_LONGS_EQUAL(16, sizeof(internal::ExecutionTrace<Cal3_S2>));
  EXPECT_LONGS_EQUAL(2*5*8, sizeof(internal::Jacobian<Point2,Cal3_S2>::type));
  EXPECT_LONGS_EQUAL(2*2*8, sizeof(internal::Jacobian<Point2,Point2>::type));
  size_t expectedRecordSize = sizeof(Cal3_S2)
      + sizeof(internal::ExecutionTrace<Cal3_S2>)
      + +sizeof(internal::Jacobian<Point2, Cal3_S2>::type) + sizeof(Point2)
      + sizeof(internal::ExecutionTrace<Point2>)
      + sizeof(internal::Jacobian<Point2, Point2>::type);
  EXPECT_LONGS_EQUAL(expectedRecordSize + 8, sizeof(Binary::Record));

  // Check size
  size_t size = binary.traceSize();
  CHECK(size);
  EXPECT_LONGS_EQUAL(expectedRecordSize + 8, size);
  // Use Variable Length Array, allocated on stack by gcc
  // Note unclear for Clang: http://clang.llvm.org/compatibility.html#vla
  internal::ExecutionTraceStorage traceStorage[size];
  internal::ExecutionTrace<Point2> trace;
  Point2 value = binary.traceExecution(values, trace, traceStorage);
  EXPECT(assert_equal(Point2(),value, 1e-9));
  // trace.print();

  // Expected Jacobians
  Matrix25 expected25;
  expected25 << 0, 0, 0, 1, 0, 0, 0, 0, 0, 1;
  Matrix2 expected22;
  expected22 << 1, 0, 0, 1;

  // Check matrices
  boost::optional<Binary::Record*> r = trace.record<Binary::Record>();
  CHECK(r);
  EXPECT(assert_equal(expected25, (Matrix ) (*r)->dTdA1, 1e-9));
  EXPECT(assert_equal(expected22, (Matrix ) (*r)->dTdA2, 1e-9));
}
/* ************************************************************************* */
// Unary(Binary(Leaf,Leaf))
TEST(ExpressionFactor, Shallow) {

  // Create some values
  Values values;
  values.insert(1, Pose3());
  values.insert(2, Point3(0, 0, 1));

  // Create old-style factor to create expected value and derivatives
  GenericProjectionFactor<Pose3, Point3> old(measured, model, 1, 2,
      boost::make_shared<Cal3_S2>());
  double expected_error = old.error(values);
  GaussianFactor::shared_ptr expected = old.linearize(values);

  // Create leaves
  Pose3_ x_(1);
  Point3_ p_(2);

  // Construct expression, concise evrsion
  Point2_ expression = project(transform_to(x_, p_));

  // Get and check keys and dims
  FastVector<Key> keys;
  FastVector<int> dims;
  boost::tie(keys, dims) = expression.keysAndDims();
  LONGS_EQUAL(2,keys.size());
  LONGS_EQUAL(2,dims.size());
  LONGS_EQUAL(1,keys[0]);
  LONGS_EQUAL(2,keys[1]);
  LONGS_EQUAL(6,dims[0]);
  LONGS_EQUAL(3,dims[1]);

  // traceExecution of shallow tree
  typedef internal::UnaryExpression<Point2, Point3> Unary;
  typedef internal::BinaryExpression<Point3, Pose3, Point3> Binary;
  size_t expectedTraceSize = sizeof(Unary::Record) + sizeof(Binary::Record);
  EXPECT_LONGS_EQUAL(96, sizeof(Unary::Record));
#ifdef GTSAM_USE_QUATERNIONS
  EXPECT_LONGS_EQUAL(352, sizeof(Binary::Record));
  LONGS_EQUAL(96+352, expectedTraceSize);
#else
  EXPECT_LONGS_EQUAL(384, sizeof(Binary::Record));
  LONGS_EQUAL(96+384, expectedTraceSize);
#endif
  size_t size = expression.traceSize();
  CHECK(size);
  EXPECT_LONGS_EQUAL(expectedTraceSize, size);
  internal::ExecutionTraceStorage traceStorage[size];
  internal::ExecutionTrace<Point2> trace;
  Point2 value = expression.traceExecution(values, trace, traceStorage);
  EXPECT(assert_equal(Point2(),value, 1e-9));
  // trace.print();

  // Expected Jacobians
  Matrix23 expected23;
  expected23 << 1, 0, 0, 0, 1, 0;

  // Check matrices
  boost::optional<Unary::Record*> r = trace.record<Unary::Record>();
  CHECK(r);
  EXPECT(assert_equal(expected23, (Matrix)(*r)->dTdA1, 1e-9));

  // Linearization
  ExpressionFactor<Point2> f2(model, measured, expression);
  EXPECT_DOUBLES_EQUAL(expected_error, f2.error(values), 1e-9);
  EXPECT_LONGS_EQUAL(2, f2.dim());
  boost::shared_ptr<GaussianFactor> gf2 = f2.linearize(values);
  EXPECT( assert_equal(*expected, *gf2, 1e-9));
}

/* ************************************************************************* */
// Binary(Leaf,Unary(Binary(Leaf,Leaf)))
TEST(ExpressionFactor, tree) {

  // Create some values
  Values values;
  values.insert(1, Pose3());
  values.insert(2, Point3(0, 0, 1));
  values.insert(3, Cal3_S2());

  // Create old-style factor to create expected value and derivatives
  GeneralSFMFactor2<Cal3_S2> old(measured, model, 1, 2, 3);
  double expected_error = old.error(values);
  GaussianFactor::shared_ptr expected = old.linearize(values);

  // Create leaves
  Pose3_ x(1);
  Point3_ p(2);
  Cal3_S2_ K(3);

  // Create expression tree
  Point3_ p_cam(x, &Pose3::transform_to, p);
  Point2_ xy_hat(Project, p_cam);
  Point2_ uv_hat(K, &Cal3_S2::uncalibrate, xy_hat);

  // Create factor and check value, dimension, linearization
  ExpressionFactor<Point2> f(model, measured, uv_hat);
  EXPECT_DOUBLES_EQUAL(expected_error, f.error(values), 1e-9);
  EXPECT_LONGS_EQUAL(2, f.dim());
  boost::shared_ptr<GaussianFactor> gf = f.linearize(values);
  EXPECT( assert_equal(*expected, *gf, 1e-9));

  // Concise version
  ExpressionFactor<Point2> f2(model, measured,
      uncalibrate(K, project(transform_to(x, p))));
  EXPECT_DOUBLES_EQUAL(expected_error, f2.error(values), 1e-9);
  EXPECT_LONGS_EQUAL(2, f2.dim());
  boost::shared_ptr<GaussianFactor> gf2 = f2.linearize(values);
  EXPECT( assert_equal(*expected, *gf2, 1e-9));

  // Try ternary version
  ExpressionFactor<Point2> f3(model, measured, project3(x, p, K));
  EXPECT_DOUBLES_EQUAL(expected_error, f3.error(values), 1e-9);
  EXPECT_LONGS_EQUAL(2, f3.dim());
  boost::shared_ptr<GaussianFactor> gf3 = f3.linearize(values);
  EXPECT( assert_equal(*expected, *gf3, 1e-9));
}

/* ************************************************************************* */

TEST(ExpressionFactor, Compose1) {

  // Create expression
  Rot3_ R1(1), R2(2);
  Rot3_ R3 = R1 * R2;

  // Create factor
  ExpressionFactor<Rot3> f(noiseModel::Unit::Create(3), Rot3(), R3);

  // Create some values
  Values values;
  values.insert(1, Rot3());
  values.insert(2, Rot3());

  // Check unwhitenedError
  std::vector<Matrix> H(2);
  Vector actual = f.unwhitenedError(values, H);
  EXPECT( assert_equal(eye(3), H[0],1e-9));
  EXPECT( assert_equal(eye(3), H[1],1e-9));

  // Check linearization
  JacobianFactor expected(1, eye(3), 2, eye(3), zero(3));
  boost::shared_ptr<GaussianFactor> gf = f.linearize(values);
  boost::shared_ptr<JacobianFactor> jf = //
      boost::dynamic_pointer_cast<JacobianFactor>(gf);
  EXPECT( assert_equal(expected, *jf,1e-9));
}

/* ************************************************************************* */
// Test compose with arguments referring to the same rotation
TEST(ExpressionFactor, compose2) {

  // Create expression
  Rot3_ R1(1), R2(1);
  Rot3_ R3 = R1 * R2;

  // Create factor
  ExpressionFactor<Rot3> f(noiseModel::Unit::Create(3), Rot3(), R3);

  // Create some values
  Values values;
  values.insert(1, Rot3());

  // Check unwhitenedError
  std::vector<Matrix> H(1);
  Vector actual = f.unwhitenedError(values, H);
  EXPECT_LONGS_EQUAL(1, H.size());
  EXPECT( assert_equal(2*eye(3), H[0],1e-9));

  // Check linearization
  JacobianFactor expected(1, 2 * eye(3), zero(3));
  boost::shared_ptr<GaussianFactor> gf = f.linearize(values);
  boost::shared_ptr<JacobianFactor> jf = //
      boost::dynamic_pointer_cast<JacobianFactor>(gf);
  EXPECT( assert_equal(expected, *jf,1e-9));
}

/* ************************************************************************* */
// Test compose with one arguments referring to a constant same rotation
TEST(ExpressionFactor, compose3) {

  // Create expression
  Rot3_ R1(Rot3::identity()), R2(3);
  Rot3_ R3 = R1 * R2;

  // Create factor
  ExpressionFactor<Rot3> f(noiseModel::Unit::Create(3), Rot3(), R3);

  // Create some values
  Values values;
  values.insert(3, Rot3());

  // Check unwhitenedError
  std::vector<Matrix> H(1);
  Vector actual = f.unwhitenedError(values, H);
  EXPECT_LONGS_EQUAL(1, H.size());
  EXPECT( assert_equal(eye(3), H[0],1e-9));

  // Check linearization
  JacobianFactor expected(3, eye(3), zero(3));
  boost::shared_ptr<GaussianFactor> gf = f.linearize(values);
  boost::shared_ptr<JacobianFactor> jf = //
      boost::dynamic_pointer_cast<JacobianFactor>(gf);
  EXPECT( assert_equal(expected, *jf,1e-9));
}

/* ************************************************************************* */
// Test compose with three arguments
Rot3 composeThree(const Rot3& R1, const Rot3& R2, const Rot3& R3,
    OptionalJacobian<3, 3> H1, OptionalJacobian<3, 3> H2, OptionalJacobian<3, 3> H3) {
  // return dummy derivatives (not correct, but that's ok for testing here)
  if (H1)
    *H1 = eye(3);
  if (H2)
    *H2 = eye(3);
  if (H3)
    *H3 = eye(3);
  return R1 * (R2 * R3);
}

TEST(ExpressionFactor, composeTernary) {

  // Create expression
  Rot3_ A(1), B(2), C(3);
  Rot3_ ABC(composeThree, A, B, C);

  // Create factor
  ExpressionFactor<Rot3> f(noiseModel::Unit::Create(3), Rot3(), ABC);

  // Create some values
  Values values;
  values.insert(1, Rot3());
  values.insert(2, Rot3());
  values.insert(3, Rot3());

  // Check unwhitenedError
  std::vector<Matrix> H(3);
  Vector actual = f.unwhitenedError(values, H);
  EXPECT_LONGS_EQUAL(3, H.size());
  EXPECT( assert_equal(eye(3), H[0],1e-9));
  EXPECT( assert_equal(eye(3), H[1],1e-9));
  EXPECT( assert_equal(eye(3), H[2],1e-9));

  // Check linearization
  JacobianFactor expected(1, eye(3), 2, eye(3), 3, eye(3), zero(3));
  boost::shared_ptr<GaussianFactor> gf = f.linearize(values);
  boost::shared_ptr<JacobianFactor> jf = //
      boost::dynamic_pointer_cast<JacobianFactor>(gf);
  EXPECT( assert_equal(expected, *jf,1e-9));
}

TEST(ExpressionFactor, tree_finite_differences) {

  // Create some values
  Values values;
  values.insert(1, Pose3());
  values.insert(2, Point3(0, 0, 1));
  values.insert(3, Cal3_S2());

  // Create leaves
  Pose3_ x(1);
  Point3_ p(2);
  Cal3_S2_ K(3);

  // Create expression tree
  Point3_ p_cam(x, &Pose3::transform_to, p);
  Point2_ xy_hat(Project, p_cam);
  Point2_ uv_hat(K, &Cal3_S2::uncalibrate, xy_hat);

  const double fd_step = 1e-5;
  const double tolerance = 1e-5;
  EXPECT_CORRECT_EXPRESSION_JACOBIANS(uv_hat, values, fd_step, tolerance);
}

TEST(ExpressionFactor, push_back) {
  NonlinearFactorGraph graph;
  graph.addExpressionFactor(model, Point2(0, 0), leaf::p);
}

/* ************************************************************************* */
// Test with multiple compositions on duplicate keys
static double specialSum(const double& v1, const double& v2,
    OptionalJacobian<1, 1> H1, OptionalJacobian<1, 1> H2) {
  if (H1) (*H1) << 1.0;
  if (H2) (*H2) << 2.0;
  return v1 + 2.0 * v2;
}

TEST(Expression, testMultipleCompositions) {
  const double tolerance = 1e-5;
  const double fd_step = 1e-9;

  double v1 = 2;
  double v2 = 3;

  Values values;
  values.insert(1, v1);
  values.insert(2, v2);

  Expression<double> v1_(Key(1));
  Expression<double> v2_(Key(2));

  // BinaryExpression
  //   Leaf, key = 1
  //   Leaf, key = 2
  Expression<double> sum1_(specialSum, v1_, v2_);
  GTSAM_PRINT(sum1_);
  EXPECT_CORRECT_EXPRESSION_JACOBIANS(sum1_, values, fd_step, tolerance);

  // BinaryExpression
  //   BinaryExpression
  //     Leaf, key = 1
  //     Leaf, key = 2
  //   Leaf, key = 1
  Expression<double> sum2_(specialSum, sum1_, v1_);
  GTSAM_PRINT(sum2_);
  EXPECT_CORRECT_EXPRESSION_JACOBIANS(sum2_, values, fd_step, tolerance);

  // BinaryExpression
  //   BinaryExpression
  //     BinaryExpression
  //       Leaf, key = 1
  //       Leaf, key = 2
  //     Leaf, key = 1
  //   BinaryExpression
  //     Leaf, key = 1
  //     Leaf, key = 2
  Expression<double> sum3_(specialSum, sum2_, sum1_);
  GTSAM_PRINT(sum3_);
  EXPECT_CORRECT_EXPRESSION_JACOBIANS(sum3_, values, fd_step, tolerance);
}

/* ************************************************************************* */
int main() {
  TestResult tr;
  return TestRegistry::runAllTests(tr);
}
/* ************************************************************************* */

