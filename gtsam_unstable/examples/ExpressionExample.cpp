/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation,
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file    ExpressionExample.cpp
 * @brief   A structure-from-motion example done with Expressions
 * @author  Frank Dellaert
 * @author  Duy-Nguyen Ta
 * @date    October 1, 2014
 */

/**
 * This is the Expression version of SFMExample
 * See detailed description of headers there, this focuses on explaining the AD part
 */

// The two new headers that allow using our Automatic Differentiation Expression framework
#include <gtsam_unstable/slam/expressions.h>
#include <gtsam_unstable/nonlinear/BADFactor.h>

// Header order is close to far
#include <examples/SFMdata.h>
#include <gtsam/slam/PriorFactor.h>
#include <gtsam/slam/ProjectionFactor.h>
#include <gtsam/geometry/Point2.h>
#include <gtsam/nonlinear/NonlinearFactorGraph.h>
#include <gtsam/nonlinear/DoglegOptimizer.h>
#include <gtsam/nonlinear/Values.h>
#include <gtsam/inference/Symbol.h>

#include <vector>

using namespace std;
using namespace gtsam;

/* ************************************************************************* */
int main(int argc, char* argv[]) {

  Cal3_S2 K(50.0, 50.0, 0.0, 50.0, 50.0);
  noiseModel::Isotropic::shared_ptr measurementNoise = noiseModel::Isotropic::Sigma(2, 1.0); // one pixel in u and v

  // Create the set of ground-truth landmarks and poses
  vector<Point3> points = createPoints();
  vector<Pose3> poses = createPoses();

  // Create a factor graph
  NonlinearFactorGraph graph;

  // Specify uncertainty on first pose prior
  noiseModel::Diagonal::shared_ptr poseNoise = noiseModel::Diagonal::Sigmas((Vector(6) << Vector3::Constant(0.3), Vector3::Constant(0.1)));

  // Here we don't use a PriorFactor but directly the BADFactor class
  // The object x0 is an Expression, and we create a factor wanting it to be equal to poses[0]
  Pose3_ x0('x',0);
  graph.push_back(BADFactor<Pose3>(poseNoise, poses[0], x0));

  // We create a constant Expression for the calibration here
  Cal3_S2_ cK(K);

  // Simulated measurements from each camera pose, adding them to the factor graph
  for (size_t i = 0; i < poses.size(); ++i) {
    for (size_t j = 0; j < points.size(); ++j) {
      SimpleCamera camera(poses[i], K);
      Point2 measurement = camera.project(points[j]);
      // Below an expression for the prediction of the measurement:
      Pose3_ x('x', i);
      Point3_ p('l', j);
      Expression<Point2> prediction = uncalibrate(cK, project(transform_to(x, p)));
      // Again, here we use a BADFactor
      graph.push_back(BADFactor<Point2>(measurementNoise, measurement, prediction));
    }
  }

  // Add prior on first point to constrain scale, again with BADFActor
  noiseModel::Isotropic::shared_ptr pointNoise = noiseModel::Isotropic::Sigma(3, 0.1);
  graph.push_back(BADFactor<Point3>(pointNoise, points[0], Point3_('l', 0)));

  // Create perturbed initial
  Values initialEstimate;
  for (size_t i = 0; i < poses.size(); ++i)
    initialEstimate.insert(Symbol('x', i), poses[i].compose(Pose3(Rot3::rodriguez(-0.1, 0.2, 0.25), Point3(0.05, -0.10, 0.20))));
  for (size_t j = 0; j < points.size(); ++j)
    initialEstimate.insert(Symbol('l', j), points[j].compose(Point3(-0.25, 0.20, 0.15)));
  cout << "initial error = " << graph.error(initialEstimate) << endl;

  /* Optimize the graph and print results */
  Values result = DoglegOptimizer(graph, initialEstimate).optimize();
  cout << "final error = " << graph.error(result) << endl;

  return 0;
}
/* ************************************************************************* */

