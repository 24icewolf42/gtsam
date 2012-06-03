/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file    inference.h
 * @brief   Contains *generic* inference algorithms that convert between templated
 * graphical models, i.e., factor graphs, Bayes nets, and Bayes trees
 * @author  Frank Dellaert
 * @author  Richard Roberts
 */

#pragma once

#include <gtsam/inference/VariableIndex.h>
#include <gtsam/inference/Permutation.h>

#include <boost/foreach.hpp>
#include <boost/optional.hpp>

#include <deque>

namespace gtsam {

namespace inference {

/**
 * Compute a permutation (variable ordering) using colamd
 */
Permutation::shared_ptr PermutationCOLAMD(const VariableIndex& variableIndex);

/**
 * Compute a permutation (variable ordering) using constrained colamd
 */
template<typename CONSTRAINED>
Permutation::shared_ptr PermutationCOLAMD(const VariableIndex& variableIndex, const CONSTRAINED& constrainLast);

/**
 * Compute a CCOLAMD permutation using the constraint groups in cmember.
 */
Permutation::shared_ptr PermutationCOLAMD_(const VariableIndex& variableIndex, std::vector<int>& cmember);

/** Factor the factor graph into a conditional and a remaining factor graph.
 * Given the factor graph \f$ f(X) \f$, and \c variables to factorize out
 * \f$ V \f$, this function factorizes into \f$ f(X) = f(V;Y)f(Y) \f$, where
 * \f$ Y := X\V \f$ are the remaining variables.  If \f$ f(X) = p(X) \f$ is
 * a probability density or likelihood, the factorization produces a
 * conditional probability density and a marginal \f$ p(X) = p(V|Y)p(Y) \f$.
 *
 * For efficiency, this function treats the variables to eliminate
 * \c variables as fully-connected, so produces a dense (fully-connected)
 * conditional on all of the variables in \c variables, instead of a sparse
 * BayesNet.  If the variables are not fully-connected, it is more efficient
 * to sequentially factorize multiple times.
 */
template<class Graph>
typename Graph::FactorizationResult eliminate(const Graph& factorGraph, const std::vector<typename Graph::KeyType>& variables,
    const typename Graph::Eliminate& eliminateFcn, boost::optional<const VariableIndex&> variableIndex = boost::none);

/** Eliminate a single variable, by calling
 * eliminate(const Graph&, const std::vector<typename Graph::KeyType>&, const typename Graph::Eliminate&, boost::optional<const VariableIndex&>)
 */
template<class Graph>
typename Graph::FactorizationResult eliminateOne(const Graph& factorGraph, typename Graph::KeyType variable,
    const typename Graph::Eliminate& eliminateFcn, boost::optional<const VariableIndex&> variableIndex = boost::none) {
  std::vector<size_t> variables(1, variable);
  return eliminate(factorGraph, variables, eliminateFcn, variableIndex);
}

}

} // namespace gtsam

#include <gtsam/inference/inference-inl.h>
