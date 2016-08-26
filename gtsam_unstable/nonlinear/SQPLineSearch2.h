/*
 * SQPLineSearch2.h
 * @brief:
 * @date: Apr 29, 2014
 * @author: Duy-Nguyen Ta
 * @author: Ivan Dario Jimenez
 */

#pragma once

#include <gtsam/nonlinear/NonlinearFactorGraph.h>
#include <gtsam/linear/GaussianFactorGraph.h>
#include <gtsam/linear/VectorValues.h>
#include <gtsam_unstable/nonlinear/NonlinearConstraint.h>
#include <gtsam_unstable/nonlinear/NP.h>

namespace gtsam {

/// Forward declaration
class MeritFunction;

/**
 * This class implements line search method following Betts10book
 */
class SQPLineSearch2 {
  NP program_;
public:

  /// State of each iteration
  struct State {
    Values solution; //!< Current solution
    VectorValues lambdas; //!< Lagrange multipliers
    double mu; //!< Penalty weights for the merit function
    double tau; //!< Hessian damping term
    bool converged; //!< Convergence flag
    unsigned int k; //!< iteration number first iteration is 1

    State() :
        converged(false), k(1){
    }

    State(const Values& _solution, const VectorValues& _lambdas, double _mu,
        double _tau, bool _converged, unsigned int _k) :
        solution(_solution), lambdas(_lambdas), mu(_mu), tau(_tau),
        converged(_converged), k(_k) {
    }

    void print(const std::string& s = "State") const {
      std::cout << s << ": " << std::endl;
      solution.print("\tSolution: ");
      lambdas.print("\tLambdas: ");
      std::cout << "mu: " << mu << std::endl;
      std::cout << "tau: " << tau << std::endl;
      std::cout << "k: " << k << std::endl;
      std::cout << "\tConverged: " << converged << std::endl;
    }
  };

public:

  /// Constructor
  SQPLineSearch2(const NP& program) :
      program_(program) {
  }

  /// Create alpha*\sum lambda_i*constrainedHessian_i
  GaussianFactorGraph::shared_ptr multiplyConstrainedHessians(
      const NonlinearFactorGraph& constrainedGraph, const Values& x,
      VectorValues lambdas, double alpha) const;

  /// Build a damped system to overcome negative Hessian
  GaussianFactorGraph::shared_ptr buildDampedSystem(
      const GaussianFactorGraph& linear, const State& state) const ;

  /// Check convergence
  bool checkConvergence(const Values& x, const VectorValues& lambdas) const;

  /// Check if a point is feasible
  bool checkFeasibility(const Values &x) const;

  /// Gets a feasible point
  Values getFeasiblePoint() const;
  
  /// Iterate 1 step
  State iterate(const State& currentState) const;
  
  /**
   * Zero initializes lambdas for all constraints in a NonlinearProgram
   * @param nonlinearProgram
   * @return Zero Initialized Vector Value with vectors of the size of the constraint.
   */
  VectorValues zeroFromConstraints(const NP& nonlinearProgram) const;

  /// Full optimization
  Values optimize(const Values& initials, unsigned int max_iter = 200) const;
  
};

/* ************************************************************************* */
/**
 * Merit function goes with Betts' line search SQP implementation
 * Betts10book 2.27
 */
class MeritFunction {
private:
  NP program_;
  GaussianFactorGraph::shared_ptr linearUnconstrained_, lagrangianGraph_;
  Values x_;
  VectorValues p_, gradf_;

public:

  /// Constructor
  MeritFunction(const NP & program,
      const GaussianFactorGraph::shared_ptr& linearUnconstrained,
      const GaussianFactorGraph::shared_ptr& lagrangianGraph, const Values& x,
      const VectorValues& p);

  /// Update predicted solution, Betts10book, 2.30
  boost::tuple<Values, VectorValues, VectorValues> update(double alpha) const;

  /// Compute 1-norm of the constraints ||c(x)||_1
  double constraintNorm1(const Values x) const;

  /// phi(alpha,mu)
  double phi(double alpha, double mu) const;

  /// Dk(mu)
  double D(double mu) const;

  /// Nocedal06book, 18.36
  double computeNewMu(double currentMu) const;

  double ptHp(const GaussianFactorGraph& linear, const VectorValues& p) const;

};

} /* namespace gtsam */
