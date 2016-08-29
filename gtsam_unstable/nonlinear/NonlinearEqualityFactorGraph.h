/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation,
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file    NonlinearEqualityFactorGraph.h
 * @author  Duy-Nguyen Ta
 * @author  Krunal Chande
 * @author  Luca Carlone
 * @date    Dec 15, 2014
 */

#pragma once
#include <gtsam_unstable/linear/EqualityFactorGraph.h>
#include <gtsam_unstable/nonlinear/NonlinearEqualityConstraint.h>

namespace gtsam {

class NonlinearEqualityFactorGraph: public FactorGraph<NonlinearConstraint> {
public:
  /// Default constructor
  NonlinearEqualityFactorGraph() {
  }

  /// Linearize to a EqualityFactorGraph
  EqualityFactorGraph::shared_ptr linearize(
      const Values& linearizationPoint) const {
    EqualityFactorGraph::shared_ptr linearGraph(new EqualityFactorGraph());
    for (const NonlinearFactor::shared_ptr& factor : *this) {
      JacobianFactor::shared_ptr jacobian = boost::dynamic_pointer_cast
          < JacobianFactor > (factor->linearize(linearizationPoint));
      NonlinearConstraint::shared_ptr constraint =
          boost::dynamic_pointer_cast < NonlinearConstraint > (factor);
      linearGraph->add(LinearEquality(*jacobian, constraint->dualKey()));
    }
    return linearGraph;
  }

  /**
   * Return true if the max absolute error all factors is less than a tolerance
   */
  bool checkFeasibility(const Values& values, double tol) const {
    for (const NonlinearFactor::shared_ptr& factor : *this) {
      NoiseModelFactor::shared_ptr noiseModelFactor =
          boost::dynamic_pointer_cast < NoiseModelFactor > (factor);
      Vector error = noiseModelFactor->unwhitenedError(values);
      if (error.lpNorm<Eigen::Infinity>() > tol) {
        return false;
      }
    }
    return true;
  }
  
  double error(const Values& values) const {
    double total_error(0.0);
    for(const sharedFactor & factor: *this){
      if(factor)
        total_error += factor->error(values);
    }
    return total_error;
  }
  
  double cost(const Values& values) const {
    double total_cost(0.0);
    for(const sharedFactor& factor: *this){
      if(factor)
        total_cost += factor->unwhitenedError(values).sum();
    }
    return total_cost;
  }
//  /**
//   * Additional cost for -lambda*ConstraintHessian for SQP
//   */
//  GaussianFactorGraph::shared_ptr multipliedHessians(const Values& values, const VectorValues& duals) const {
//    GaussianFactorGraph::shared_ptr constrainedHessians(new GaussianFactorGraph());
//    BOOST_FOREACH(const NonlinearFactor::shared_ptr& factor, *this) {
//      NonlinearConstraint::shared_ptr constraint = boost::dynamic_pointer_cast<NonlinearConstraint>(factor);
//      constrainedHessians->push_back(constraint->multipliedHessian(values, duals));
//    }
//    return constrainedHessians;
//  }

};

}
