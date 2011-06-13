/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/*
 * SubgraphPreconditioner.h
 * Created on: Dec 31, 2009
 * @author: Frank Dellaert
 */

#pragma once

#include <gtsam/linear/JacobianFactor.h>
#include <gtsam/linear/GaussianBayesNet.h>
#include <gtsam/nonlinear/Ordering.h> // FIXME shouldn't have nonlinear things in linear

namespace gtsam {

	/**
	 * Subgraph conditioner class, as explained in the RSS 2010 submission.
	 * Starting with a graph A*x=b, we split it in two systems A1*x=b1 and A2*x=b2
	 * We solve R1*x=c1, and make the substitution y=R1*x-c1.
	 * To use the class, give the Bayes Net R1*x=c1 and Graph A2*x=b2.
	 * Then solve for yhat using CG, and solve for xhat = system.x(yhat).
	 */
	class SubgraphPreconditioner {

	public:
		typedef boost::shared_ptr<const GaussianBayesNet> sharedBayesNet;
		typedef boost::shared_ptr<const FactorGraph<JacobianFactor> > sharedFG;
		typedef boost::shared_ptr<const VectorValues> sharedValues;
		typedef boost::shared_ptr<const Errors> sharedErrors;

	private:
		sharedFG Ab1_, Ab2_;
		sharedBayesNet Rc1_;
		sharedValues xbar_;
		sharedErrors b2bar_; /** b2 - A2*xbar */

	public:

		SubgraphPreconditioner();
		/**
		 * Constructor
		 * @param Ab1: the Graph A1*x=b1
		 * @param Ab2: the Graph A2*x=b2
		 * @param Rc1: the Bayes Net R1*x=c1
		 * @param xbar: the solution to R1*x=c1
		 */
		SubgraphPreconditioner(const sharedFG& Ab1, const sharedFG& Ab2, const sharedBayesNet& Rc1,	const sharedValues& xbar);

		/** Access Ab1 */
		const sharedFG& Ab1() const { return Ab1_; }

		/** Access Ab2 */
		const sharedFG& Ab2() const { return Ab2_; }

		/** Access Rc1 */
		const sharedBayesNet& Rc1() const { return Rc1_; }

	    /**
	     * Add zero-mean i.i.d. Gaussian prior terms to each variable
	     * @param sigma Standard deviation of Gaussian
	     */
//	    SubgraphPreconditioner add_priors(double sigma) const;

		/* x = xbar + inv(R1)*y */
		VectorValues x(const VectorValues& y) const;

		/* A zero VectorValues with the structure of xbar */
		VectorValues zero() const {
			VectorValues V(*xbar_) ;
			V.makeZero();
			return V ;
		}

		/**
		 * Add constraint part of the error only, used in both calls above
		 * y += alpha*inv(R1')*A2'*e2
		 * Takes a range indicating e2 !!!!
		 */
		void transposeMultiplyAdd2(double alpha, Errors::const_iterator begin,
				Errors::const_iterator end, VectorValues& y) const;

			/** print the object */
		void print(const std::string& s = "SubgraphPreconditioner") const;
	};

  /* error, given y */
  double error(const SubgraphPreconditioner& sp, const VectorValues& y);

  /** gradient = y + inv(R1')*A2'*(A2*inv(R1)*y-b2bar) */
  VectorValues gradient(const SubgraphPreconditioner& sp, const VectorValues& y);

  /** Apply operator A */
  Errors operator*(const SubgraphPreconditioner& sp, const VectorValues& y);

  /** Apply operator A in place: needs e allocated already */
  void multiplyInPlace(const SubgraphPreconditioner& sp, const VectorValues& y, Errors& e);

    /** Apply operator A' */
  VectorValues operator^(const SubgraphPreconditioner& sp, const Errors& e);

  /**
   * Add A'*e to y
   *  y += alpha*A'*[e1;e2] = [alpha*e1; alpha*inv(R1')*A2'*e2]
   */
  void transposeMultiplyAdd(const SubgraphPreconditioner& sp, double alpha, const Errors& e, VectorValues& y);

} // namespace gtsam
