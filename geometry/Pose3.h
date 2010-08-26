/**
 *@file  Pose3.h
 *@brief 3D Pose
 */

// \callgraph

#pragma once

#include <boost/numeric/ublas/vector_proxy.hpp>

#include <gtsam/geometry/Point3.h>
#include <gtsam/geometry/Rot3.h>
#include <gtsam/base/Testable.h>
#include <gtsam/base/Lie.h>

namespace gtsam {

  /** A 3D pose (R,t) : (Rot3,Point3) */
  class Pose3 : Testable<Pose3>, public Lie<Pose3> {
  public:
	  static const size_t dimension = 6;

  private:
    Rot3 R_;
    Point3 t_;

  public:

    /** Default constructor is origin */
    Pose3() {}

    /** Copy constructor */
    Pose3(const Pose3& pose) : R_(pose.R_), t_(pose.t_) {}

    /** Construct from R,t */
    Pose3(const Rot3& R, const Point3& t) : R_(R), t_(t) {}

    /** Constructor from 4*4 matrix */
    Pose3(const Matrix &T) :
      R_(T(0, 0), T(0, 1), T(0, 2), T(1, 0), T(1, 1), T(1, 2), T(2, 0),
          T(2, 1), T(2, 2)), t_(T(0, 3), T(1, 3), T(2, 3)) {}

    /** Constructor from 12D vector */
    Pose3(const Vector &V) :
      R_(V(0), V(3), V(6), V(1), V(4), V(7), V(2), V(5), V(8)),
      t_(V(9), V(10),V(11)) {}

    inline const Rot3& rotation() const { return R_; }
    inline const Point3& translation() const { return t_; }

    inline double x() const { return t_.x(); }
    inline double y() const { return t_.y(); }
    inline double z() const { return t_.z(); }

    /** convert to 4*4 matrix */
    Matrix matrix() const;

    /** print with optional string */
    void print(const std::string& s = "") const;

    /** assert equality up to a tolerance */
    bool equals(const Pose3& pose, double tol = 1e-9) const;

    /** Compose two poses */
    inline Pose3 operator*(const Pose3& T) const {
    	return Pose3(R_*T.R_, t_ + R_*T.t_);
    }

    Pose3 transform_to(const Pose3& pose) const;

    /** dimension of the variable - used to autodetect sizes */
    inline static size_t Dim() { return dimension; }

    /** Lie requirements */

    /** Dimensionality of the tangent space */
    inline size_t dim() const { return dimension; }

    /**
     * Derivative of inverse
     */
    Pose3 inverse(boost::optional<Matrix&> H1=boost::none) const;

    /**
     * composes two poses (first (*this) then p2)
     * with optional derivatives
     */
    Pose3 compose(const Pose3& p2,
  		  	boost::optional<Matrix&> H1=boost::none,
  		  	boost::optional<Matrix&> H2=boost::none) const;

    /** receives the point in Pose coordinates and transforms it to world coordinates */
    Point3 transform_from(const Point3& p,
  		  	boost::optional<Matrix&> H1=boost::none, boost::optional<Matrix&> H2=boost::none) const;

    /** syntactic sugar for transform */
    inline Point3 operator*(const Point3& p) { return transform_from(p); }

    /** receives the point in world coordinates and transforms it to Pose coordinates */
    Point3 transform_to(const Point3& p,
    		  	boost::optional<Matrix&> H1=boost::none, boost::optional<Matrix&> H2=boost::none) const;

    /** Exponential map at identity - create a pose with a translation and
     * rotation (in canonical coordinates). */
    static Pose3 Expmap(const Vector& v);

    /** Log map at identity - return the translation and canonical rotation
     * coordinates of a pose. */
    static Vector Logmap(const Pose3& p);

    /** Exponential map around another pose */
    Pose3 expmap(const Vector& d) const;

    /** Logarithm map around another pose T1 */
    Vector logmap(const Pose3& T2) const;

    /**
     * Return relative pose between p1 and p2, in p1 coordinate frame
     * as well as optionally the derivatives
     */
    Pose3 between(const Pose3& p2,
    		boost::optional<Matrix&> H1=boost::none,
    		boost::optional<Matrix&> H2=boost::none) const;

    /**
     * Calculate Adjoint map
     * Ad_pose is 6*6 matrix that when applied to twist xi, returns Ad_pose(xi)
     */
    Matrix AdjointMap() const;
    inline Vector Adjoint(const Vector& xi) const {return AdjointMap()*xi; }

    /**
     * wedge for Pose3:
     * @param xi 6-dim twist (omega,v) where
     *  omega = (wx,wy,wz) 3D angular velocity
     *  v (vx,vy,vz) = 3D velocity
     * @return xihat, 4*4 element of Lie algebra that can be exponentiated
     */
    static inline Matrix wedge(double wx, double wy, double wz, double vx, double vy, double vz) {
    	return Matrix_(4,4,
    			 0.,-wz,  wy,  vx,
    			 wz,  0.,-wx,  vy,
    			-wy, wx,   0., vz,
    			 0.,  0.,  0.,  0.);
    }

  private:
    /** Serialization function */
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
      ar & BOOST_SERIALIZATION_NVP(R_);
      ar & BOOST_SERIALIZATION_NVP(t_);
    }
  }; // Pose3 class

  /**
   * wedge for Pose3:
   * @param xi 6-dim twist (omega,v) where
   *  omega = 3D angular velocity
   *  v = 3D velocity
   * @return xihat, 4*4 element of Lie algebra that can be exponentiated
   */
  template <>
  inline Matrix wedge<Pose3>(const Vector& xi) {
  	return Pose3::wedge(xi(0),xi(1),xi(2),xi(3),xi(4),xi(5));
  }

} // namespace gtsam
