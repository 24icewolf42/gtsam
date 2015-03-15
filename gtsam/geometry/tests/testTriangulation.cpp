/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation,
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * testTriangulation.cpp
 *
 *  Created on: July 30th, 2013
 *      Author: cbeall3
 */

#include <gtsam/geometry/triangulation.h>
#include <gtsam/geometry/SimpleCamera.h>
#include <gtsam/geometry/Cal3Bundler.h>
#include <CppUnitLite/TestHarness.h>

#include <boost/assign.hpp>
#include <boost/assign/std/vector.hpp>

using namespace std;
using namespace gtsam;
using namespace boost::assign;

// Some common constants

static const boost::shared_ptr<Cal3_S2> sharedCal = //
    boost::make_shared<Cal3_S2>(1500, 1200, 0, 640, 480);

// Looking along X-axis, 1 meter above ground plane (x-y)
static const Rot3 upright = Rot3::ypr(-M_PI / 2, 0., -M_PI / 2);
static const Pose3 pose1 = Pose3(upright, gtsam::Point3(0, 0, 1));
PinholeCamera<Cal3_S2> camera1(pose1, *sharedCal);

// create second camera 1 meter to the right of first camera
static const Pose3 pose2 = pose1 * Pose3(Rot3(), Point3(1, 0, 0));
PinholeCamera<Cal3_S2> camera2(pose2, *sharedCal);

// landmark ~5 meters infront of camera
static const Point3 landmark(5, 0.5, 1.2);

// 1. Project two landmarks into two cameras and triangulate
Point2 z1 = camera1.project(landmark);
Point2 z2 = camera2.project(landmark);

//******************************************************************************
// Simple test with a well-behaved two camera situation
TEST( triangulation, twoPoses) {

  vector<Pose3> poses;
  vector<Point2> measurements;

  poses += pose1, pose2;
  measurements += z1, z2;

  double rank_tol = 1e-9;

  // 1. Test simple DLT, perfect in no noise situation
  bool optimize = false;
  boost::optional<Point3> actual1 = //
      triangulatePoint3(poses, sharedCal, measurements, rank_tol, optimize);
  EXPECT(assert_equal(landmark, *actual1, 1e-7));

  // 2. test with optimization on, same answer
  optimize = true;
  boost::optional<Point3> actual2 = //
      triangulatePoint3(poses, sharedCal, measurements, rank_tol, optimize);
  EXPECT(assert_equal(landmark, *actual2, 1e-7));

  // 3. Add some noise and try again: result should be ~ (4.995, 0.499167, 1.19814)
  measurements.at(0) += Point2(0.1, 0.5);
  measurements.at(1) += Point2(-0.2, 0.3);
  optimize = false;
  boost::optional<Point3> actual3 = //
      triangulatePoint3(poses, sharedCal, measurements, rank_tol, optimize);
  EXPECT(assert_equal(Point3(4.995, 0.499167, 1.19814), *actual3, 1e-4));

  // 4. Now with optimization on
  optimize = true;
  boost::optional<Point3> actual4 = //
      triangulatePoint3(poses, sharedCal, measurements, rank_tol, optimize);
  EXPECT(assert_equal(Point3(4.995, 0.499167, 1.19814), *actual4, 1e-4));
}

//******************************************************************************
// Similar, but now with Bundler calibration
TEST( triangulation, twoPosesBundler) {

  boost::shared_ptr<Cal3Bundler> bundlerCal = //
      boost::make_shared<Cal3Bundler>(1500, 0, 0, 640, 480);
  PinholeCamera<Cal3Bundler> camera1(pose1, *bundlerCal);
  PinholeCamera<Cal3Bundler> camera2(pose2, *bundlerCal);

  // 1. Project two landmarks into two cameras and triangulate
  Point2 z1 = camera1.project(landmark);
  Point2 z2 = camera2.project(landmark);

  vector<Pose3> poses;
  vector<Point2> measurements;

  poses += pose1, pose2;
  measurements += z1, z2;

  bool optimize = true;
  double rank_tol = 1e-9;

  boost::optional<Point3> actual = //
      triangulatePoint3(poses, bundlerCal, measurements, rank_tol, optimize);
  EXPECT(assert_equal(landmark, *actual, 1e-7));

  // Add some noise and try again
  measurements.at(0) += Point2(0.1, 0.5);
  measurements.at(1) += Point2(-0.2, 0.3);

  boost::optional<Point3> actual2 = //
      triangulatePoint3(poses, bundlerCal, measurements, rank_tol, optimize);
  EXPECT(assert_equal(Point3(4.995, 0.499167, 1.19847), *actual2, 1e-4));
}

//******************************************************************************
TEST( triangulation, fourPoses) {
  vector<Pose3> poses;
  vector<Point2> measurements;

  poses += pose1, pose2;
  measurements += z1, z2;

  boost::optional<Point3> actual = triangulatePoint3(poses, sharedCal,
      measurements);
  EXPECT(assert_equal(landmark, *actual, 1e-2));

  // 2. Add some noise and try again: result should be ~ (4.995, 0.499167, 1.19814)
  measurements.at(0) += Point2(0.1, 0.5);
  measurements.at(1) += Point2(-0.2, 0.3);

  boost::optional<Point3> actual2 = //
      triangulatePoint3(poses, sharedCal, measurements);
  EXPECT(assert_equal(landmark, *actual2, 1e-2));

  // 3. Add a slightly rotated third camera above, again with measurement noise
  Pose3 pose3 = pose1 * Pose3(Rot3::ypr(0.1, 0.2, 0.1), Point3(0.1, -2, -.1));
  SimpleCamera camera3(pose3, *sharedCal);
  Point2 z3 = camera3.project(landmark);

  poses += pose3;
  measurements += z3 + Point2(0.1, -0.1);

  boost::optional<Point3> triangulated_3cameras = //
      triangulatePoint3(poses, sharedCal, measurements);
  EXPECT(assert_equal(landmark, *triangulated_3cameras, 1e-2));

  // Again with nonlinear optimization
  boost::optional<Point3> triangulated_3cameras_opt = triangulatePoint3(poses,
      sharedCal, measurements, 1e-9, true);
  EXPECT(assert_equal(landmark, *triangulated_3cameras_opt, 1e-2));

  // 4. Test failure: Add a 4th camera facing the wrong way
  Pose3 pose4 = Pose3(Rot3::ypr(M_PI / 2, 0., -M_PI / 2), Point3(0, 0, 1));
  SimpleCamera camera4(pose4, *sharedCal);

#ifdef GTSAM_THROW_CHEIRALITY_EXCEPTION
  CHECK_EXCEPTION(camera4.project(landmark), CheiralityException);

  poses += pose4;
  measurements += Point2(400, 400);

  CHECK_EXCEPTION(triangulatePoint3(poses, sharedCal, measurements),
      TriangulationCheiralityException);
#endif
}

//******************************************************************************
TEST( triangulation, fourPoses_distinct_Ks) {
  Cal3_S2 K1(1500, 1200, 0, 640, 480);
  // create first camera. Looking along X-axis, 1 meter above ground plane (x-y)
  SimpleCamera camera1(pose1, K1);

  // create second camera 1 meter to the right of first camera
  Cal3_S2 K2(1600, 1300, 0, 650, 440);
  SimpleCamera camera2(pose2, K2);

  // 1. Project two landmarks into two cameras and triangulate
  Point2 z1 = camera1.project(landmark);
  Point2 z2 = camera2.project(landmark);

  vector<SimpleCamera> cameras;
  vector<Point2> measurements;

  cameras += camera1, camera2;
  measurements += z1, z2;

  boost::optional<Point3> actual = //
      triangulatePoint3(cameras, measurements);
  EXPECT(assert_equal(landmark, *actual, 1e-2));

  // 2. Add some noise and try again: result should be ~ (4.995, 0.499167, 1.19814)
  measurements.at(0) += Point2(0.1, 0.5);
  measurements.at(1) += Point2(-0.2, 0.3);

  boost::optional<Point3> actual2 = //
      triangulatePoint3(cameras, measurements);
  EXPECT(assert_equal(landmark, *actual2, 1e-2));

  // 3. Add a slightly rotated third camera above, again with measurement noise
  Pose3 pose3 = pose1 * Pose3(Rot3::ypr(0.1, 0.2, 0.1), Point3(0.1, -2, -.1));
  Cal3_S2 K3(700, 500, 0, 640, 480);
  SimpleCamera camera3(pose3, K3);
  Point2 z3 = camera3.project(landmark);

  cameras += camera3;
  measurements += z3 + Point2(0.1, -0.1);

  boost::optional<Point3> triangulated_3cameras = //
      triangulatePoint3(cameras, measurements);
  EXPECT(assert_equal(landmark, *triangulated_3cameras, 1e-2));

  // Again with nonlinear optimization
  boost::optional<Point3> triangulated_3cameras_opt = triangulatePoint3(cameras,
      measurements, 1e-9, true);
  EXPECT(assert_equal(landmark, *triangulated_3cameras_opt, 1e-2));

  // 4. Test failure: Add a 4th camera facing the wrong way
  Pose3 pose4 = Pose3(Rot3::ypr(M_PI / 2, 0., -M_PI / 2), Point3(0, 0, 1));
  Cal3_S2 K4(700, 500, 0, 640, 480);
  SimpleCamera camera4(pose4, K4);

#ifdef GTSAM_THROW_CHEIRALITY_EXCEPTION
  CHECK_EXCEPTION(camera4.project(landmark), CheiralityException);

  cameras += camera4;
  measurements += Point2(400, 400);
  CHECK_EXCEPTION(triangulatePoint3(cameras, measurements),
      TriangulationCheiralityException);
#endif
}

//******************************************************************************
TEST( triangulation, twoIdenticalPoses) {
  // create first camera. Looking along X-axis, 1 meter above ground plane (x-y)
  SimpleCamera camera1(pose1, *sharedCal);

  // 1. Project two landmarks into two cameras and triangulate
  Point2 z1 = camera1.project(landmark);

  vector<Pose3> poses;
  vector<Point2> measurements;

  poses += pose1, pose1;
  measurements += z1, z1;

  CHECK_EXCEPTION(triangulatePoint3(poses, sharedCal, measurements),
      TriangulationUnderconstrainedException);
}

//******************************************************************************
TEST( triangulation, onePose) {
  // we expect this test to fail with a TriangulationUnderconstrainedException
  // because there's only one camera observation

  vector<Pose3> poses;
  vector<Point2> measurements;

  poses += Pose3();
  measurements += Point2();

  CHECK_EXCEPTION(triangulatePoint3(poses, sharedCal, measurements),
      TriangulationUnderconstrainedException);
}

//******************************************************************************
int main() {
  TestResult tr;
  return TestRegistry::runAllTests(tr);
}
//******************************************************************************
