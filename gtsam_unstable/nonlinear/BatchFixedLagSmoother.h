/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file    BatchFixedLagSmoother.h
 * @brief   An LM-based fixed-lag smoother.
 *
 * @author  Michael Kaess, Stephen Williams
 * @date    Oct 14, 2012
 */

// \callgraph
#pragma once

#include <gtsam_unstable/nonlinear/FixedLagSmoother.h>
#include <gtsam/nonlinear/LevenbergMarquardtOptimizer.h>
#include <queue>

namespace gtsam {

class BatchFixedLagSmoother : public FixedLagSmoother {

public:

  /// Typedef for a shared pointer to an Incremental Fixed-Lag Smoother
  typedef boost::shared_ptr<BatchFixedLagSmoother> shared_ptr;

  /** default constructor */
  BatchFixedLagSmoother(double smootherLag = 0.0, const LevenbergMarquardtParams& parameters = LevenbergMarquardtParams(), bool enforceConsistency = false) :
    FixedLagSmoother(smootherLag), parameters_(parameters), enforceConsistency_(enforceConsistency) { };

  /** destructor */
  virtual ~BatchFixedLagSmoother() { };

  /** Print the factor for debugging and testing (implementing Testable) */
  virtual void print(const std::string& s = "BatchFixedLagSmoother:\n", const KeyFormatter& keyFormatter = DefaultKeyFormatter) const;

  /** Check if two IncrementalFixedLagSmoother Objects are equal */
  virtual bool equals(const FixedLagSmoother& rhs, double tol = 1e-9) const;

  /** Add new factors, updating the solution and relinearizing as needed. */
  Result update(const NonlinearFactorGraph& newFactors = NonlinearFactorGraph(), const Values& newTheta = Values(),
      const KeyTimestampMap& timestamps = KeyTimestampMap());

  /** Compute an estimate from the incomplete linear delta computed during the last update.
   * This delta is incomplete because it was not updated below wildfire_threshold.  If only
   * a single variable is needed, it is faster to call calculateEstimate(const KEY&).
   */
  Values calculateEstimate() const {
    return theta_;
  }

  /** Compute an estimate for a single variable using its incomplete linear delta computed
   * during the last update.  This is faster than calling the no-argument version of
   * calculateEstimate, which operates on all variables.
   * @param key
   * @return
   */
  template<class VALUE>
  VALUE calculateEstimate(Key key) const {
    return theta_.at<VALUE>(key);
  }

  /** read the current set of optimizer parameters */
  const LevenbergMarquardtParams& params() const {
    return parameters_;
  }

  /** update the current set of optimizer parameters */
  LevenbergMarquardtParams& params() {
    return parameters_;
  }

protected:

  /** A typedef defining an Key-Factor mapping **/
  typedef std::map<Key, std::set<Index> > FactorIndex;

  /** The L-M optimization parameters **/
  LevenbergMarquardtParams parameters_;

  /** A flag indicating if the optimizer should enforce probabilistic consistency by maintaining the
   * linearization point of all variables involved in linearized/marginal factors at the edge of the
   * smoothing window. This idea is from ??? TODO: Look up paper reference **/
  bool enforceConsistency_;

  /** The nonlinear factors **/
  NonlinearFactorGraph factors_;

  /** The current linearization point **/
  Values theta_;

  /** The set of keys involved in current linearized factors. These keys should not be relinearized. **/
  Values linearizedKeys_;

  /** The set of available factor graph slots. These occur because we are constantly deleting factors, leaving holes. **/
  std::queue<size_t> availableSlots_;

  /** A cross-reference structure to allow efficient factor lookups by key **/
  FactorIndex factorIndex_;



  /** Augment the list of factors with a set of new factors */
  void updateFactors(const NonlinearFactorGraph& newFactors);

  /** Remove factors from the list of factors by slot index */
  void removeFactors(const std::set<size_t>& deleteFactors);

  /** Erase any keys associated with timestamps before the provided time */
  void eraseKeys(const std::set<Key>& keys);

  /** Marginalize out selected variables */
  void marginalizeKeys(const std::set<Key>& marginalizableKeys);


private:
  /** Private methods for printing debug information */
  static void PrintKeySet(const std::set<Key>& keys, const std::string& label);
  static void PrintSymbolicFactor(const NonlinearFactor::shared_ptr& factor);
  static void PrintSymbolicFactor(const GaussianFactor::shared_ptr& factor, const Ordering& ordering);
  static void PrintSymbolicGraph(const NonlinearFactorGraph& graph, const std::string& label);
  static void PrintSymbolicGraph(const GaussianFactorGraph& graph, const Ordering& ordering, const std::string& label);
}; // BatchFixedLagSmoother

} /// namespace gtsam
