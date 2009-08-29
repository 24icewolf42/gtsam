/**
 * @file  Pose2.h
 * @brief 3D Pose
 * @author: Frank Dellaert
 */

// \callgraph

#pragma once

#include "Point2.h"

namespace gtsam {

	/**
	 * A 2D pose (x,y,theta)
	 */
	class Pose2 {

	private:
		double x_, y_, theta_;

	public:

		/** default constructor = origin */
		Pose2() :
			x_(0), y_(0), theta_(0) {
		} // default is origin

		/** copy constructor */
		Pose2(const Pose2& pose) :
			x_(pose.x_), y_(pose.y_), theta_(pose.theta_) {
		}

		/** construct from (x,y,theta) */
		Pose2(double x, double y, double theta) :
			x_(x), y_(y), theta_(theta) {
		}

		/** construct from rotation and translation */
		Pose2(const Point2& t, double theta) :
			x_(t.x()), y_(t.y()), theta_(theta) {
		}

    /** get functions for x, y, theta */
		double x()     const { return x_;}
		double y()     const { return y_;}
		double theta() const { return theta_;}

		/** print with optional string */
		void print(const std::string& s = "") const;

		Pose2 exmap(const Vector& v) const;

		/** assert equality up to a tolerance */
		bool equals(const Pose2& pose, double tol = 1e-9) const;
	};

	/** assert equality up to a tolerance */
	bool assert_equal(const Pose2& A, const Pose2& B, double tol = 1e-9);

} // namespace gtsam
