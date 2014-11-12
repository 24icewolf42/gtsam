/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file testMethod.cpp
 * @brief Unit test for Method class
 * @author Frank Dellaert
 * @date Nov 12, 2014
 **/

#include <wrap/Method.h>
#include <CppUnitLite/TestHarness.h>
#include <iostream>

using namespace std;
using namespace wrap;

/* ************************************************************************* */
// Constructor
TEST( Method, Constructor ) {
  Method method;
}

/* ************************************************************************* */
// addOverload
TEST( Method, addOverload ) {
  Method method;
  method.name = "myName";
  bool verbose = true, is_const = true;
  ArgumentList args;
  const ReturnValue retVal(ReturnType("return_type"));
  method.addOverload(verbose, is_const, "myName", args, retVal);
  EXPECT_LONGS_EQUAL(1,method.argLists.size());
  EXPECT_LONGS_EQUAL(1,method.returnVals.size());
}

/* ************************************************************************* */
int main() {
  TestResult tr;
  return TestRegistry::runAllTests(tr);
}
/* ************************************************************************* */
