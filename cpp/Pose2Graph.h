/**
 * @file    Pose2Graph.h
 * @brief   A factor graph for the 2D PoseSLAM problem
 * @author  Frank Dellaert
 * @author  Viorela Ila
 */

#pragma once

#include "Pose2Factor.h"
#include "NonlinearFactorGraph.h"

namespace gtsam {

	/**
	 * Non-linear factor graph for visual SLAM
	 */
	class Pose2Graph: public gtsam::NonlinearFactorGraph<Pose2Config> {

	public:

		/** default constructor is empty graph */
		Pose2Graph() {
		}

		/**
		 * equals
		 */
		bool equals(const Pose2Graph& p, double tol = 1e-9) const;

		/**
		 * Add a factor without having to do shared factor dance
		 */
		inline void add(const Pose2Config::Key& key1, const Pose2Config::Key& key2,
				const Pose2& measured, const Matrix& covariance) {
			push_back(sharedFactor(new Pose2Factor(key1, key2, measured, covariance)));
		}

		/**
		 *  Add an equality constraint on a pose
		 *  @param key of pose
		 *  @param pose which pose to constrain it to
		 */
		void addConstraint(const Pose2Config::Key& key, const Pose2& pose =	Pose2());

	private:
		/** Serialization function */
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive & ar, const unsigned int version) {
		}
	};

} // namespace gtsam
