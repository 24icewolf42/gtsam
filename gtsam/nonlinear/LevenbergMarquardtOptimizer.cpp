/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation,
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file    LevenbergMarquardtOptimizer.cpp
 * @brief   
 * @author  Richard Roberts
 * @date  Feb 26, 2012
 */

#include <gtsam/nonlinear/LevenbergMarquardtOptimizer.h>
#include <gtsam/linear/linearExceptions.h>
#include <gtsam/linear/GaussianFactorGraph.h>
#include <gtsam/linear/VectorValues.h>

#include <boost/algorithm/string.hpp>
#include <string>
#include <cmath>
#include <ctime>
#include <fstream>

using namespace std;

namespace gtsam {

/* ************************************************************************* */
LevenbergMarquardtParams::VerbosityLM LevenbergMarquardtParams::verbosityLMTranslator(const std::string &src) const {
  std::string s = src;  boost::algorithm::to_upper(s);
  if (s == "SILENT") return LevenbergMarquardtParams::SILENT;
  if (s == "LAMBDA") return LevenbergMarquardtParams::LAMBDA;
  if (s == "TRYLAMBDA") return LevenbergMarquardtParams::TRYLAMBDA;
  if (s == "TRYCONFIG") return LevenbergMarquardtParams::TRYCONFIG;
  if (s == "TRYDELTA") return LevenbergMarquardtParams::TRYDELTA;
  if (s == "DAMPED") return LevenbergMarquardtParams::DAMPED;

  /* default is silent */
  return LevenbergMarquardtParams::SILENT;
}

/* ************************************************************************* */
std::string LevenbergMarquardtParams::verbosityLMTranslator(VerbosityLM value) const {
  std::string s;
  switch (value) {
  case LevenbergMarquardtParams::SILENT:    s = "SILENT" ;     break;
  case LevenbergMarquardtParams::LAMBDA:    s = "LAMBDA" ;     break;
  case LevenbergMarquardtParams::TRYLAMBDA: s = "TRYLAMBDA" ;  break;
  case LevenbergMarquardtParams::TRYCONFIG: s = "TRYCONFIG" ;  break;
  case LevenbergMarquardtParams::TRYDELTA:  s = "TRYDELTA" ;   break;
  case LevenbergMarquardtParams::DAMPED:    s = "DAMPED" ;     break;
  default:                                  s = "UNDEFINED" ;  break;
  }
  return s;
}

/* ************************************************************************* */
void LevenbergMarquardtParams::print(const std::string& str) const {
  NonlinearOptimizerParams::print(str);
  std::cout << "              lambdaInitial: " << lambdaInitial << "\n";
  std::cout << "               lambdaFactor: " << lambdaFactor << "\n";
  std::cout << "           lambdaUpperBound: " << lambdaUpperBound << "\n";
  std::cout << "                verbosityLM: " << verbosityLMTranslator(verbosityLM) << "\n";
  std::cout.flush();
}

/* ************************************************************************* */
GaussianFactorGraph::shared_ptr LevenbergMarquardtOptimizer::linearize() const {
  return graph_.linearize(state_.values);
}

/* ************************************************************************* */
void LevenbergMarquardtOptimizer::iterate() {

  // Log current error/lambda to file
  if (!params_.logFile.empty()) {
    ofstream os(params_.logFile.c_str(), ios::app);

    timeval rawtime;
    gettimeofday(&rawtime, NULL);
    double currentTime = rawtime.tv_sec + rawtime.tv_usec / 1000000.0;

    os << state_.iterations << "," << currentTime-state_.startTime << ","
        << state_.error << "," << state_.lambda << endl;
  }

  gttic(LM_iterate);

  // Linearize graph
  GaussianFactorGraph::shared_ptr linear = linearize();

  // Pull out parameters we'll use
  const NonlinearOptimizerParams::Verbosity nloVerbosity = params_.verbosity;
  const LevenbergMarquardtParams::VerbosityLM lmVerbosity = params_.verbosityLM;

  // Keep increasing lambda until we make make progress
  while(true) {
    if (lmVerbosity >= LevenbergMarquardtParams::TRYLAMBDA)
      cout << "trying lambda = " << state_.lambda << endl;
      ++state_.totalNumberInnerIterations;
      // cout << "state_.totalNumberInnerIterations = " << state_.totalNumberInnerIterations << endl;
    // Add prior-factors
    // TODO: replace this dampening with a backsubstitution approach
    gttic(damp);
    GaussianFactorGraph dampedSystem = *linear;
    {
      double sigma = 1.0 / std::sqrt(state_.lambda);
      dampedSystem.reserve(dampedSystem.size() + state_.values.size());
      // for each of the variables, add a prior
      BOOST_FOREACH(const Values::KeyValuePair& key_value, state_.values) {
        size_t dim = key_value.value.dim();
        Matrix A = eye(dim);
        Vector b = zero(dim);
        SharedDiagonal model = noiseModel::Isotropic::Sigma(dim, sigma);
        dampedSystem += boost::make_shared<JacobianFactor>(key_value.key, A, b, model);
      }
    }
    gttoc(damp);
    if (lmVerbosity >= LevenbergMarquardtParams::DAMPED) dampedSystem.print("damped");

    // Try solving
    try {
      // Solve Damped Gaussian Factor Graph
      const VectorValues delta = solve(dampedSystem, state_.values, params_);

      if (lmVerbosity >= LevenbergMarquardtParams::TRYLAMBDA) cout << "linear delta norm = " << delta.norm() << endl;
      if (lmVerbosity >= LevenbergMarquardtParams::TRYDELTA) delta.print("delta");

      // update values
      gttic(retract);
      Values newValues = state_.values.retract(delta);
      gttoc(retract);

      // create new optimization state with more adventurous lambda
      gttic(compute_error);
      double error = graph_.error(newValues);
      gttoc(compute_error);

      if (lmVerbosity >= LevenbergMarquardtParams::TRYLAMBDA) cout << "next error = " << error << endl;

      if(error <= state_.error) {
        state_.values.swap(newValues);
        state_.error = error;
        state_.lambda /= params_.lambdaFactor;
        break;
      } else {
        // Either we're not cautious, or the same lambda was worse than the current error.
        // The more adventurous lambda was worse too, so make lambda more conservative
        // and keep the same values.
        if(state_.lambda >= params_.lambdaUpperBound) {
          if(nloVerbosity >= NonlinearOptimizerParams::ERROR)
            cout << "Warning:  Levenberg-Marquardt giving up because cannot decrease error with maximum lambda" << endl;
          break;
        } else {
          state_.lambda *= params_.lambdaFactor;
        }
      }
    } catch(IndeterminantLinearSystemException& e) {
      (void) e; // Prevent unused variable warning
      if(lmVerbosity >= LevenbergMarquardtParams::LAMBDA)
        cout << "Negative matrix, increasing lambda" << endl;
      // Either we're not cautious, or the same lambda was worse than the current error.
      // The more adventurous lambda was worse too, so make lambda more conservative
      // and keep the same values.
      if(state_.lambda >= params_.lambdaUpperBound) {
        if(nloVerbosity >= NonlinearOptimizerParams::ERROR)
          cout << "Warning:  Levenberg-Marquardt giving up because cannot decrease error with maximum lambda" << endl;
        break;
      } else {
        state_.lambda *= params_.lambdaFactor;
      }
    }
// Frank asks: why would we do that?
//    catch(...) {
//      throw;
//    }
  } // end while

  if (lmVerbosity >= LevenbergMarquardtParams::LAMBDA)
    cout << "using lambda = " << state_.lambda << endl;

  // Increment the iteration counter
  ++state_.iterations;
}

/* ************************************************************************* */
LevenbergMarquardtParams LevenbergMarquardtOptimizer::ensureHasOrdering(
  LevenbergMarquardtParams params, const NonlinearFactorGraph& graph) const
{
  if(!params.ordering)
    params.ordering = Ordering::COLAMD(graph);
  return params;
}

} /* namespace gtsam */

