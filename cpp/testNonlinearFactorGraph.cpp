/** 
 * @file    testNonlinearFactorGraph.cpp
 * @brief   Unit tests for Non-Linear Factor Graph
 * @brief   testNonlinearFactorGraph
 * @author  Carlos Nieto
 * @author  Christian Potthast
 */

/*STL/C++*/
#include <iostream>
using namespace std;

#include <boost/assign/std/list.hpp>
using namespace boost::assign;

#include <CppUnitLite/TestHarness.h>

#define GTSAM_MAGIC_KEY

#include "Matrix.h"
#include "smallExample.h"
#include "FactorGraph-inl.h"
#include "NonlinearFactorGraph-inl.h"

using namespace gtsam;

/* ************************************************************************* */
TEST( ExampleNonlinearFactorGraph, equals )
{
	ExampleNonlinearFactorGraph fg = createNonlinearFactorGraph();
	ExampleNonlinearFactorGraph fg2 = createNonlinearFactorGraph();
	CHECK( fg.equals(fg2) );
}

/* ************************************************************************* */
TEST( ExampleNonlinearFactorGraph, error )
{
	ExampleNonlinearFactorGraph fg = createNonlinearFactorGraph();
	VectorConfig c1 = createConfig();
	double actual1 = fg.error(c1);
	DOUBLES_EQUAL( 0.0, actual1, 1e-9 );

	VectorConfig c2 = createNoisyConfig();
	double actual2 = fg.error(c2);
	DOUBLES_EQUAL( 5.625, actual2, 1e-9 );
}

/* ************************************************************************* */
TEST( ExampleNonlinearFactorGraph, GET_ORDERING)
{
  Ordering expected;
  expected += "l1","x1","x2";
  ExampleNonlinearFactorGraph nlfg = createNonlinearFactorGraph();
  Ordering actual = nlfg.getOrdering();
  CHECK(assert_equal(expected,actual));
}

/* ************************************************************************* */
TEST( ExampleNonlinearFactorGraph, probPrime )
{
	ExampleNonlinearFactorGraph fg = createNonlinearFactorGraph();
	VectorConfig cfg = createConfig();

	// evaluate the probability of the factor graph
	double actual = fg.probPrime(cfg);
	double expected = 1.0;
	DOUBLES_EQUAL(expected,actual,0);
}

/* ************************************************************************* *
// TODO: Commented out until noise model is passed to Gaussian factor graph
TEST( ExampleNonlinearFactorGraph, linearize )
{
	ExampleNonlinearFactorGraph fg = createNonlinearFactorGraph();
	VectorConfig initial = createNoisyConfig();
	GaussianFactorGraph linearized = fg.linearize(initial);
	GaussianFactorGraph expected = createGaussianFactorGraph();
	CHECK(assert_equal(expected,linearized));
}

/* ************************************************************************* */
int main() {
	TestResult tr;
	return TestRegistry::runAllTests(tr);
}
/* ************************************************************************* */
