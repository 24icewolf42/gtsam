/**
 * @file   Cal3_S2.h
 * @brief  The most common 5DOF 3D->2D calibration
 * @author Frank Dellaert
 */

#pragma once

#include "Matrix.h"
#include "Point2.h"

namespace gtsam {

	/** The most common 5DOF 3D->2D calibration */
	class Cal3_S2: Testable<Cal3_S2> {
	private:
		double fx_, fy_, s_, u0_, v0_;

	public:
		/**
		 * default calibration leaves coordinates unchanged
		 */
		Cal3_S2() :
			fx_(1), fy_(1), s_(0), u0_(0), v0_(0) {
		}

		/**
		 * constructor from doubles
		 */
		Cal3_S2(double fx, double fy, double s, double u0, double v0) :
			fx_(fx), fy_(fy), s_(s), u0_(u0), v0_(v0) {
		}

		void print(const std::string& s = "") const {
			gtsam::print(matrix(), s);
		}

		/**
		 * Check if equal up to specified tolerance
		 */
		bool equals(const Cal3_S2& K, double tol = 10e-9) const;

		/**
		 * return DOF, dimensionality of tangent space
		 */
		size_t dim() const {
			return 5;
		}

		/**
		 * load calibration from location (default name is calibration_info.txt)
		 */
		Cal3_S2(const std::string &path);

		/**
		 * Given 5-dim tangent vector, create new calibration
		 */
		Cal3_S2 exmap(const Vector& d) const {
			return Cal3_S2(fx_ + d(0), fy_ + d(1), s_ + d(2), u0_ + d(3), v0_ + d(4));
		}

		/**
		 * return the principal point
		 */
		Point2 principalPoint() const {
			return Point2(u0_,v0_);
		}

		/**
		 * return vectorized form (column-wise)
		 */
		Vector vector() const {
			double r[] = { fx_, fy_, s_, u0_, v0_ };
			Vector v(5);
			copy(r, r + 5, v.begin());
			return v;
		}

		/**
		 * return calibration matrix K
		 */
		Matrix matrix() const {
			return Matrix_(3, 3, fx_, s_, u0_, 0.0, fy_, v0_, 0.0, 0.0, 1.0);
		}

		/**
		 * convert intrinsic coordinates xy to image coordinates uv
		 */
		Point2 uncalibrate(const Point2& p) const {
			const double x = p.x(), y = p.y();
			return Point2(fx_ * x + s_ * y + u0_, fy_ * y + v0_);
		}

		/** friends */
		friend Matrix Duncalibrate2(const Cal3_S2& K, const Point2& p);

	private:
		/** Serialization function */
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(fx_);
			ar & BOOST_SERIALIZATION_NVP(fy_);
			ar & BOOST_SERIALIZATION_NVP(s_);
			ar & BOOST_SERIALIZATION_NVP(u0_);
			ar & BOOST_SERIALIZATION_NVP(v0_);
		}
	};

	/**
	 * convert intrinsic coordinates xy to image coordinates uv
	 */
	Point2 uncalibrate(const Cal3_S2& K, const Point2& p);
	Matrix Duncalibrate1(const Cal3_S2& K, const Point2& p);
	Matrix Duncalibrate2(const Cal3_S2& K, const Point2& p);
}
