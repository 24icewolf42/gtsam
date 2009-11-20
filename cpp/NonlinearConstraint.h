/*
 * @file NonlinearConstraint.h
 * @brief Implements nonlinear constraints that can be linearized and
 * inserted into an existing nonlinear graph and solved via SQP
 * @author Alex Cunningham
 */

#pragma once

#include <map>
#include <iostream>
#include "NonlinearFactor.h"

namespace gtsam {

/**
 * Base class for nonlinear constraints
 * This allows for both equality and inequality constraints,
 * where equality constraints are active all the time (even slightly
 * nonzero constraint functions will still be active - inequality
 * constraints should be sure to force to actual zero)
 *
 * The measurement z in the underlying NonlinearFactor is the
 * set of Lagrange multipliers.
 */
template <class Config>
class NonlinearConstraint : public NonlinearFactor<Config> {

protected:
	/** key for the lagrange multipliers */
	std::string lagrange_key_;

	/** number of lagrange multipliers */
	size_t p_;

public:
	/** Constructor - sets the cost function and the lagrange multipliers
	 * @param lagrange_key is the label for the associated lagrange multipliers
	 * @param dim_lagrange is the number of associated constraints
	 */
	NonlinearConstraint(const std::string& lagrange_key, size_t dim_lagrange) :
		NonlinearFactor<Config>(zero(dim_lagrange), 1.0),
		lagrange_key_(lagrange_key), p_(dim_lagrange) {}

	/** returns the key used for the Lagrange multipliers */
	std::string& lagrangeKey() const { return lagrange_key_; }

	/** returns the number of lagrange multipliers */
	size_t nrConstraints() const { return p_; }

	/** Print */
	virtual void print(const std::string& s = "") const =0;

	/** Check if two factors are equal */
	virtual bool equals(const Factor<Config>& f, double tol=1e-9) const=0;

	/** error function - returns the result of the constraint function */
	virtual inline Vector error_vector(const Config& c) const=0;

	/**
	 * Linearize using a real Config and a VectorConfig of Lagrange multipliers
	 * Returns the two separate Gaussian factors to solve
	 * @param config is the real Config of the real variables
	 * @param lagrange is the VectorConfig of lagrange multipliers
	 * @return a pair GaussianFactor (probabilistic) and GaussianFactor (constraint)
	 */
	virtual std::pair<GaussianFactor::shared_ptr, GaussianFactor::shared_ptr>
	linearize(const Config& config, const VectorConfig& lagrange) const=0;

	/**
	 * linearize with only Config, which is not currently implemented
	 * This will be implemented later for other constrained optimization
	 * algorithms
	 */
	virtual boost::shared_ptr<GaussianFactor> linearize(const Config& c) const {
		throw std::invalid_argument("No current constraint linearization for a single Config!");
	}
};


/**
 * A unary constraint with arbitrary cost and gradient functions
 */
template <class Config>
class NonlinearConstraint1 : public NonlinearConstraint<Config> {

private:
	/** calculates the constraint function of the current config
	 * If the value is zero, the constraint is not active
	 * @param config is a configuration of all the variables
	 * @param key is the id for the selected variable
	 * @return the cost for each of p constraints, arranged in a vector
	 */
	Vector (*g_)(const Config& config, const std::string& key);

	/**
	 * Calculates the gradient of the constraint function
	 * returns a pxn matrix
	 * @param config to use for linearization
	 * @param key of selected variable
	 * @return the jacobian of the constraint in terms of key
	 */
	Matrix (*gradG_) (const Config& config, const std::string& key);

	/** key for the constrained variable */
	std::string key_;

public:

	/**
	 * Basic constructor
	 * @param key is the identifier for the variable constrained
	 * @param gradG gives the gradient of the constraint function
	 * @param g is the constraint function
	 * @param dim_constraint is the size of the constraint (p)
	 * @param lagrange_key is the identifier for the lagrange multiplier
	 */
	NonlinearConstraint1(
			const std::string& key,
			Matrix (*gradG)(const Config& config, const std::string& key),
			Vector (*g)(const Config& config, const std::string& key),
			size_t dim_constraint,
			const std::string& lagrange_key="") :
				NonlinearConstraint<Config>(lagrange_key, dim_constraint),
				g_(g), gradG_(gradG), key_(key) {
		// set a good lagrange key here
		// TODO:should do something smart to find a unique one
		if (lagrange_key == "")
			this->lagrange_key_ = "L_" + key;
	}

	/** Print */
	void print(const std::string& s = "") const;

	/** Check if two factors are equal */
	bool equals(const Factor<Config>& f, double tol=1e-9) const;

	/** error function - returns the result of the constraint function */
	inline Vector error_vector(const Config& c) const {
		return g_(c, key_);
	}

	/**
	 * Linearize using a real Config and a VectorConfig of Lagrange multipliers
	 * Returns the two separate Gaussian factors to solve
	 * @param config is the real Config of the real variables
	 * @param lagrange is the VectorConfig of lagrange multipliers
	 * @return a pair GaussianFactor (probabilistic) and GaussianFactor (constraint)
	 */
	std::pair<GaussianFactor::shared_ptr, GaussianFactor::shared_ptr>
	linearize(const Config& config, const VectorConfig& lagrange) const;
};

/**
 * A binary constraint with arbitrary cost and gradient functions
 */
template <class Config>
class NonlinearConstraint2 : public NonlinearConstraint<Config> {

private:
	/** calculates the constraint function of the current config
	 * If the value is zero, the constraint is not active
	 * @param config is a configuration of all the variables
	 * @param key1 is the id for the first variable
	 * @param key2 is the id for the second variable
	 * @return the cost for each of p constraints, arranged in a vector
	 */
	Vector (*g_)(const Config& config, const std::string& key1, const std::string& key2);

	/**
	 * Calculates the gradients of the constraint function in terms of
	 * the first and second variables
	 * returns a pxn matrix
	 * @param config to use for linearization
	 * @param key of selected variable
	 * @return the jacobian of the constraint in terms of key
	 */
	Matrix (*gradG1_) (const Config& config, const std::string& key);
	Matrix (*gradG2_) (const Config& config, const std::string& key);

	/** keys for the constrained variables */
	std::string key1_;
	std::string key2_;

public:

	/**
	 * Basic constructor
	 * @param key is the identifier for the variable constrained
	 * @param gradG gives the gradient of the constraint function
	 * @param g is the constraint function
	 * @param dim_constraint is the size of the constraint (p)
	 * @param lagrange_key is the identifier for the lagrange multiplier
	 */
	NonlinearConstraint2(
			const std::string& key1,
			Matrix (*gradG1)(const Config& config, const std::string& key),
			const std::string& key2,
			Matrix (*gradG2)(const Config& config, const std::string& key),
			Vector (*g)(const Config& config, const std::string& key1, const std::string& key2),
			size_t dim_constraint,
			const std::string& lagrange_key="") :
				NonlinearConstraint<Config>(lagrange_key, dim_constraint),
				g_(g), gradG1_(gradG1), gradG2_(gradG2), key1_(key1), key2_(key2) {
		// set a good lagrange key here
		// TODO:should do something smart to find a unique one
		if (lagrange_key == "")
			this->lagrange_key_ = "L_" + key1 + key2;
	}

	/** Print */
	void print(const std::string& s = "") const;

	/** Check if two factors are equal */
	bool equals(const Factor<Config>& f, double tol=1e-9) const;

	/** error function - returns the result of the constraint function */
	inline Vector error_vector(const Config& c) const {
		return g_(c, key1_, key2_);
	}

	/**
	 * Linearize using a real Config and a VectorConfig of Lagrange multipliers
	 * Returns the two separate Gaussian factors to solve
	 * @param config is the real Config of the real variables
	 * @param lagrange is the VectorConfig of lagrange multipliers
	 * @return a pair GaussianFactor (probabilistic) and GaussianFactor (constraint)
	 */
	std::pair<GaussianFactor::shared_ptr, GaussianFactor::shared_ptr>
	linearize(const Config& config, const VectorConfig& lagrange) const;
};

}
