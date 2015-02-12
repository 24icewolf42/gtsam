/* ----------------------------------------------------------------------------

 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * GTSAM Copyright 2010, Georgia Tech Research Corporation,
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/*
 * @file Unit3.h
 * @date Feb 02, 2011
 * @author Can Erdogan
 * @author Frank Dellaert
 * @author Alex Trevor
 * @brief Develop a Unit3 class - basically a point on a unit sphere
 */

#pragma once

#include <gtsam/base/Manifold.h>
#include <gtsam/geometry/Point3.h>
#include <gtsam/base/DerivedValue.h>
#include <boost/random/mersenne_twister.hpp>
#include <boost/optional.hpp>

namespace gtsam {

/// Represents a 3D point on a unit sphere.
class GTSAM_EXPORT Unit3 {

private:

  Point3 p_; ///< The location of the point on the unit sphere
  mutable boost::optional<Matrix32> B_; ///< Cached basis

public:

  enum {
    dimension = 2
  };

  /// @name Constructors
  /// @{

  /// Default constructor
  Unit3() :
      p_(1.0, 0.0, 0.0) {
  }

  /// Construct from point
  explicit Unit3(const Point3& p) :
      p_(p / p.norm()) {
  }

  /// Construct from a vector3
  explicit Unit3(const Vector3& p) :
      p_(p / p.norm()) {
  }

  /// Construct from x,y,z
  Unit3(double x, double y, double z) :
      p_(x, y, z) {
    p_ = p_ / p_.norm();
  }

  /// Named constructor from Point3 with optional Jacobian
  static Unit3 FromPoint3(const Point3& point, //
      OptionalJacobian<2, 3> H = boost::none);

  /// Random direction, using boost::uniform_on_sphere
  static Unit3 Random(boost::mt19937 & rng);

  /// @}

  /// @name Testable
  /// @{

  /// The print fuction
  void print(const std::string& s = std::string()) const;

  /// The equals function with tolerance
  bool equals(const Unit3& s, double tol = 1e-9) const {
    return p_.equals(s.p_, tol);
  }
  /// @}

  /// @name Other functionality
  /// @{

  /**
   * Returns the local coordinate frame to tangent plane
   * It is a 3*2 matrix [b1 b2] composed of two orthogonal directions
   * tangent to the sphere at the current direction.
   */
  const Matrix32& basis() const;

  /// Return skew-symmetric associated with 3D point on unit sphere
  Matrix3 skew() const;

  /// Return unit-norm Point3
  const Point3& point3(OptionalJacobian<3, 2> H = boost::none) const {
    if (H)
      *H = basis();
    return p_;
  }

  /// Return unit-norm Vector
  Vector unitVector(boost::optional<Matrix&> H = boost::none) const {
    if (H)
      *H = basis();
    return (p_.vector());
  }

  /// Return scaled direction as Point3
  friend Point3 operator*(double s, const Unit3& d) {
    return s * d.p_;
  }

  /// Signed, vector-valued error between two directions
  Vector error(const Unit3& q, OptionalJacobian<2, 2> H = boost::none) const;

  /// Distance between two directions
  double distance(const Unit3& q, OptionalJacobian<1, 2> H = boost::none) const;

  /// @}

  /// @name Manifold
  /// @{

  /// Dimensionality of tangent space = 2 DOF
  inline static size_t Dim() {
    return 2;
  }

  /// Dimensionality of tangent space = 2 DOF
  inline size_t dim() const {
    return 2;
  }

  enum CoordinatesMode {
    EXPMAP, ///< Use the exponential map to retract
    RENORM ///< Retract with vector addtion and renormalize.
  };

  /// The retract function
  Unit3 retract(const Vector2& v) const;

  /// The local coordinates function
  Vector2 localCoordinates(const Unit3& s) const;

  /// @}

private:

  /// @name Advanced Interface
  /// @{
  /** Serialization function */
  friend class boost::serialization::access;
  template<class ARCHIVE>
  void serialize(ARCHIVE & ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_NVP(p_);
    // homebrew serialize Eigen Matrix
    ar & boost::serialization::make_nvp("B11", (*B_)(0, 0));
    ar & boost::serialization::make_nvp("B12", (*B_)(0, 1));
    ar & boost::serialization::make_nvp("B21", (*B_)(1, 0));
    ar & boost::serialization::make_nvp("B22", (*B_)(1, 1));
    ar & boost::serialization::make_nvp("B31", (*B_)(2, 0));
    ar & boost::serialization::make_nvp("B32", (*B_)(2, 1));
  }

  /// @}

};

// Define GTSAM traits
template<> struct traits<Unit3> : public internal::Manifold<Unit3> {
};

template<> struct traits<const Unit3> : public internal::Manifold<Unit3> {
};

} // namespace gtsam

