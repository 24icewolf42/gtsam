/*
 * @file testNonlinearConstraint.cpp
 * @brief Tests for nonlinear constraints handled via SQP
 * @author Alex Cunningham
 */

#include <CppUnitLite/TestHarness.h>
#include <VectorConfig.h>
#include <NonlinearConstraint.h>
#include <NonlinearConstraint-inl.h>

using namespace gtsam;

/* ************************************************************************* */
// unary functions with scalar variables
/* ************************************************************************* */
namespace test1 {
/** p = 1, gradG(x) = 2x */
Matrix grad_g(const VectorConfig& config, const std::string& key) {
	double x = config[key](0);
	return Matrix_(1,1, 2*x);
}

/** p = 1, g(x) = x^2-5 = 0 */
Vector g_func(const VectorConfig& config, const std::string& key) {
	double x = config[key](0);
	return Vector_(1, x*x-5);
}
} // \namespace test1

/* ************************************************************************* */
TEST( NonlinearConstraint1, unary_scalar_construction ) {
	// construct a constraint on x
	// the lagrange multipliers will be expected on L_x1
	// and there is only one multiplier
	size_t p = 1;
	NonlinearConstraint1<VectorConfig> c1("x", *test1::grad_g, *test1::g_func, p, "L_x1");

	// get a configuration to use for finding the error
	VectorConfig config;
	config.insert("x", Vector_(1, 1.0));

	// calculate the error
	Vector actual = c1.error_vector(config);
	Vector expected = Vector_(1.0, -4.0);
	CHECK(assert_equal(actual, expected, 1e-5));
}

/* ************************************************************************* */
TEST( NonlinearConstraint1, unary_scalar_linearize ) {
	size_t p = 1;
	NonlinearConstraint1<VectorConfig> c1("x", *test1::grad_g, *test1::g_func, p, "L_x1");

	// get a configuration to use for linearization
	VectorConfig realconfig;
	realconfig.insert("x", Vector_(1, 1.0));

	// get a configuration of Lagrange multipliers
	VectorConfig lagrangeConfig;
	lagrangeConfig.insert("L_x1", Vector_(1, 3.0));

	// linearize the system
	GaussianFactor::shared_ptr actFactor, actConstraint;
	boost::tie(actFactor, actConstraint) = c1.linearize(realconfig, lagrangeConfig);

	// verify
	GaussianFactor expFactor("x", Matrix_(1,1, 6.0), "L_x1", eye(1), zero(1), 1.0);
	GaussianFactor expConstraint("x", Matrix_(1,1, 2.0), Vector_(1,-4.0), 0.0);
	CHECK(assert_equal(*actFactor, expFactor));
	CHECK(assert_equal(*actConstraint, expConstraint));
}

/* ************************************************************************* */
TEST( NonlinearConstraint1, unary_scalar_equal ) {
	NonlinearConstraint1<VectorConfig>
		c1("x", *test1::grad_g, *test1::g_func, 1, "L_x1"),
		c2("x", *test1::grad_g, *test1::g_func, 1, "L_x1"),
		c3("x", *test1::grad_g, *test1::g_func, 2, "L_x1"),
		c4("y", *test1::grad_g, *test1::g_func, 1, "L_x1");

	CHECK(assert_equal(c1, c2));
	CHECK(assert_equal(c2, c1));
	CHECK(!c1.equals(c3));
	CHECK(!c1.equals(c4));
}

/* ************************************************************************* */
// binary functions with scalar variables
/* ************************************************************************* */
namespace test2 {
/** gradient for x, gradG(x,y) in x: 2x*/
Matrix grad_g1(const VectorConfig& config, const std::string& key) {
	double x = config[key](0);
	return Matrix_(1,1, 2.0*x);
}

/** gradient for y, gradG(x,y) in y: -1 */
Matrix grad_g2(const VectorConfig& config, const std::string& key) {
	double x = config[key](0);
	return Matrix_(1,1, -1.0);
}

/** p = 1, g(x) = x^2-5 -y = 0 */
Vector g_func(const VectorConfig& config, const std::string& key1, const std::string& key2) {
	double x = config[key1](0);
	double y = config[key2](0);
	return Vector_(1, x*x-5.0-y);
}
} // \namespace test2

/* ************************************************************************* */
TEST( NonlinearConstraint2, binary_scalar_construction ) {
	// construct a constraint on x and y
	// the lagrange multipliers will be expected on L_xy
	// and there is only one multiplier
	size_t p = 1;
	NonlinearConstraint2<VectorConfig> c1(
			"x", *test2::grad_g1,
			"y", *test2::grad_g2,
			*test2::g_func, p, "L_xy");

	// get a configuration to use for finding the error
	VectorConfig config;
	config.insert("x", Vector_(1, 1.0));
	config.insert("y", Vector_(1, 2.0));

	// calculate the error
	Vector actual = c1.error_vector(config);
	Vector expected = Vector_(1.0, -6.0);
	CHECK(assert_equal(actual, expected, 1e-5));
}

/* ************************************************************************* */
TEST( NonlinearConstraint2, binary_scalar_linearize ) {
	// create a constraint
	size_t p = 1;
	NonlinearConstraint2<VectorConfig> c1(
			"x", *test2::grad_g1,
			"y", *test2::grad_g2,
			*test2::g_func, p, "L_xy");

	// get a configuration to use for finding the error
	VectorConfig realconfig;
	realconfig.insert("x", Vector_(1, 1.0));
	realconfig.insert("y", Vector_(1, 2.0));

	// get a configuration of Lagrange multipliers
	VectorConfig lagrangeConfig;
	lagrangeConfig.insert("L_xy", Vector_(1, 3.0));

	// linearize the system
	GaussianFactor::shared_ptr actFactor, actConstraint;
	boost::tie(actFactor, actConstraint) = c1.linearize(realconfig, lagrangeConfig);

	// verify
	GaussianFactor expFactor("x", Matrix_(1,1, 6.0),
							 "y", Matrix_(1,1, -3.0),
							 "L_xy", eye(1), zero(1), 1.0);
	GaussianFactor expConstraint("x", Matrix_(1,1, 2.0),
								 "y", Matrix_(1,1, -1.0),
								 Vector_(1,-6.0), 0.0);
	CHECK(assert_equal(*actFactor, expFactor));
	CHECK(assert_equal(*actConstraint, expConstraint));
}

/* ************************************************************************* */
TEST( NonlinearConstraint2, binary_scalar_equal ) {
	NonlinearConstraint2<VectorConfig>
		c1("x", *test2::grad_g1, "y", *test2::grad_g2,*test2::g_func, 1, "L_xy"),
		c2("x", *test2::grad_g1, "y", *test2::grad_g2,*test2::g_func, 1, "L_xy"),
		c3("y", *test2::grad_g1, "x", *test2::grad_g2,*test2::g_func, 1, "L_xy"),
		c4("x", *test2::grad_g1, "z", *test2::grad_g2,*test2::g_func, 3, "L_xy");

	CHECK(assert_equal(c1, c2));
	CHECK(assert_equal(c2, c1));
	CHECK(!c1.equals(c3));
	CHECK(!c1.equals(c4));
}

/* ************************************************************************* */
int main() { TestResult tr; return TestRegistry::runAllTests(tr); }
/* ************************************************************************* */
