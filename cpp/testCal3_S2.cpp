/**
 * @file  testCal3_S2.cpp
 * @brief Unit tests for transform derivatives
 */

#include <CppUnitLite/TestHarness.h>
#include "numericalDerivative.h"
#include "Cal3_S2.h"

using namespace gtsam;

Cal3_S2 K(500, 500, 0.1, 640 / 2, 480 / 2);
Point2 p(1, -2);
double tol = 1e-8;

/** transform derivatives */

/* ************************************************************************* */
TEST( Cal3_S2, Duncalibrate1)
{
	Matrix computed = Duncalibrate1(K, p);
	Matrix numerical = numericalDerivative21(uncalibrate, K, p);
	CHECK(assert_equal(numerical,computed,tol));
}

/* ************************************************************************* */
TEST( Cal3_S2, Duncalibrate2)
{
	Matrix computed = Duncalibrate2(K, p);
	Matrix numerical = numericalDerivative22(uncalibrate, K, p);
	CHECK(assert_equal(numerical,computed,tol));
}

/* ************************************************************************* */
TEST( Cal3_S2, assert_equal)
{
	CHECK(assert_equal(K,K,tol));

	Cal3_S2 K1(501, 500, 0.1, 640 / 2, 480 / 2);
	CHECK(!K.equals(K1,tol));
}

/* ************************************************************************* */
int main() {
	TestResult tr;
	return TestRegistry::runAllTests(tr);
}
/* ************************************************************************* */

