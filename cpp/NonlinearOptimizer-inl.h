/**
 * NonlinearOptimizer-inl.h
 * This is a template definition file, include it where needed (only!)
 * so that the appropriate code is generated and link errors avoided.
 * @brief: Encapsulates nonlinear optimization state
 * @Author: Frank Dellaert
 * Created on: Sep 7, 2009
 */

#pragma once

#include <iostream>
#include <boost/tuple/tuple.hpp>
#include "NonlinearOptimizer.h"

#define INSTANTIATE_NONLINEAR_OPTIMIZER(G,C) \
  template class NonlinearOptimizer<G,C>;

using namespace std;

namespace gtsam {

	/* ************************************************************************* */
	inline bool check_convergence(double relativeErrorTreshold,
			double absoluteErrorTreshold, double currentError, double newError,
			int verbosity) {
		// check if diverges
		double absoluteDecrease = currentError - newError;
		if (verbosity >= 2)
			cout << "absoluteDecrease: " << absoluteDecrease << endl;
//		if (absoluteDecrease < 0)
//			throw overflow_error(
//					"NonlinearFactorGraph::optimize: error increased, diverges.");

		// calculate relative error decrease and update currentError
		double relativeDecrease = absoluteDecrease / currentError;
		if (verbosity >= 2)
			cout << "relativeDecrease: " << relativeDecrease << endl;
		bool converged = (relativeDecrease < relativeErrorTreshold)
				|| (absoluteDecrease < absoluteErrorTreshold);
		if (verbosity >= 1 && converged)
			cout << "converged" << endl;
		return converged;
	}

	/* ************************************************************************* */
	// Constructors without the solver
	/* ************************************************************************* */
	template<class G, class C, class LS>
	NonlinearOptimizer<G, C, LS>::NonlinearOptimizer(shared_graph graph,
			shared_ordering ordering, shared_config config, shared_solver solver, double lambda) :
		graph_(graph), ordering_(ordering), config_(config), error_(graph->error(
				*config)), lambda_(lambda), solver_(solver) {
	}

	/* ************************************************************************* */
	// Constructors without the solver
	/* ************************************************************************* */
//	template<class G, class C, class LS>
//	NonlinearOptimizer<G, C, LS>::NonlinearOptimizer(shared_graph graph,
//			shared_ordering ordering, shared_config config, double lambda) :
//		graph_(graph), ordering_(ordering), config_(config), error_(graph->error(
//				*config)), lambda_(lambda), solver_(new LS(*graph, *config)){
//	}

	/* ************************************************************************* */
	// linearize and optimize
	/* ************************************************************************* */
	template<class G, class C, class LS>
	VectorConfig NonlinearOptimizer<G, C, LS>::linearizeAndOptimizeForDelta() const {
		return solver_->linearizeAndOptimize(*graph_, *config_, *ordering_);
	}

	/* ************************************************************************* */
	// One iteration of Gauss Newton
	/* ************************************************************************* */
	template<class G, class C, class LS>
	NonlinearOptimizer<G, C, LS> NonlinearOptimizer<G, C, LS>::iterate(
			verbosityLevel verbosity) const {

		// linearize and optimize
		VectorConfig delta = linearizeAndOptimizeForDelta();

		// maybe show output
		if (verbosity >= DELTA)
			delta.print("delta");

		// take old config and update it
		shared_config newConfig(new C(expmap(*config_,delta)));

		// maybe show output
		if (verbosity >= CONFIG)
			newConfig->print("newConfig");

		NonlinearOptimizer newOptimizer = NonlinearOptimizer(graph_, ordering_, newConfig, solver_);

		if (verbosity >= ERROR)
			cout << "error: " << newOptimizer.error_ << endl;

		return newOptimizer;
	}

	/* ************************************************************************* */
	template<class G, class C, class LS>
	NonlinearOptimizer<G, C, LS> NonlinearOptimizer<G, C, LS>::gaussNewton(
			double relativeThreshold, double absoluteThreshold,
			verbosityLevel verbosity, int maxIterations) const {
		// linearize, solve, update
		NonlinearOptimizer next = iterate(verbosity);

		// check convergence
		bool converged = gtsam::check_convergence(relativeThreshold,
				absoluteThreshold, error_, next.error_, verbosity);

		// return converged state or iterate
		if (converged)
			return next;
		else
			return next.gaussNewton(relativeThreshold, absoluteThreshold, verbosity);
	}

	/* ************************************************************************* */
	// Recursively try to do tempered Gauss-Newton steps until we succeed.
	// Form damped system with given lambda, and return a new, more optimistic
	// optimizer if error decreased or recurse with a larger lambda if not.
	// TODO: in theory we can't infinitely recurse, but maybe we should put a max.
	/* ************************************************************************* */
	template<class G, class C, class LS>
	NonlinearOptimizer<G, C, LS> NonlinearOptimizer<G, C, LS>::try_lambda(
			const GaussianFactorGraph& linear, verbosityLevel verbosity, double factor) const {

		if (verbosity >= TRYLAMBDA)
			cout << "trying lambda = " << lambda_ << endl;

		// add prior-factors
		GaussianFactorGraph damped = linear.add_priors(1.0/sqrt(lambda_));
		if (verbosity >= DAMPED)
			damped.print("damped");

		// solve
		VectorConfig delta = solver_->optimize(damped, *ordering_);
		if (verbosity >= TRYDELTA)
			delta.print("delta");

		// update config
		shared_config newConfig(new C(expmap(*config_,delta))); // TODO: updateConfig
		if (verbosity >= TRYCONFIG)
			newConfig->print("config");

		// create new optimization state with more adventurous lambda
		NonlinearOptimizer next(graph_, ordering_, newConfig, solver_, lambda_ / factor);

		// if error decreased, return the new state
		if (next.error_ <= error_)
			return next;
		else {
			// TODO: can we avoid copying the config ?
			NonlinearOptimizer cautious(graph_, ordering_, config_, solver_, lambda_ * factor);
			return cautious.try_lambda(linear, verbosity, factor);
		}
	}

	/* ************************************************************************* */
	// One iteration of Levenberg Marquardt
	/* ************************************************************************* */
	template<class G, class C, class LS>
	NonlinearOptimizer<G, C, LS> NonlinearOptimizer<G, C, LS>::iterateLM(
			verbosityLevel verbosity, double lambdaFactor) const {

		// maybe show output
		if (verbosity >= CONFIG)
			config_->print("config");
		if (verbosity >= ERROR)
			cout << "error: " << error_ << endl;
		if (verbosity >= LAMBDA)
			cout << "lambda = " << lambda_ << endl;

		// linearize all factors once
		GaussianFactorGraph linear = graph_->linearize(*config_);
		if (verbosity >= LINEAR)
			linear.print("linear");

		// try lambda steps with successively larger lambda until we achieve descent
		return try_lambda(linear, verbosity, lambdaFactor);
	}

	/* ************************************************************************* */
	template<class G, class C, class LS>
	NonlinearOptimizer<G, C, LS> NonlinearOptimizer<G, C, LS>::levenbergMarquardt(
			double relativeThreshold, double absoluteThreshold,
			verbosityLevel verbosity, int maxIterations, double lambdaFactor) const {

		// do one iteration of LM
		NonlinearOptimizer next = iterateLM(verbosity, lambdaFactor);

		// check convergence
		// TODO: move convergence checks here and incorporate in verbosity levels
		// TODO: build into iterations somehow as an instance variable
		bool converged = gtsam::check_convergence(relativeThreshold,
				absoluteThreshold, error_, next.error_, verbosity);

		// return converged state or iterate
		if (converged || maxIterations <= 1) {
			// maybe show output
			if (verbosity >= CONFIG)
				next.config_->print("final config");
			if (verbosity >= ERROR)
				cout << "final error: " << next.error_ << endl;
			if (verbosity >= LAMBDA)
				cout << "final lambda = " << next.lambda_ << endl;
			return next;
		} else
			return next.levenbergMarquardt(relativeThreshold, absoluteThreshold,
					verbosity, lambdaFactor);
	}

/* ************************************************************************* */

}
