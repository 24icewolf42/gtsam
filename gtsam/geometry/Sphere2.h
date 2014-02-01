/* ----------------------------------------------------------------------------

 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * GTSAM Copyright 2010, Georgia Tech Research Corporation,
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/*
 * @file Sphere2.h
 * @date Feb 02, 2011
 * @author Can Erdogan
 * @author Frank Dellaert
 * @author Alex Trevor
 * @brief Develop a Sphere2 class - basically a point on a unit sphere
 */

#pragma once

#include <gtsam/geometry/Point3.h>
#include <gtsam/base/DerivedValue.h>

#ifndef SPHERE2_DEFAULT_COORDINATES_MODE
  #define SPHERE2_DEFAULT_COORDINATES_MODE Sphere2::RENORM
#endif

// (Cumbersome) forward declaration for random generator
namespace boost {
namespace random {
template<class UIntType, std::size_t w, std::size_t n, std::size_t m,
    std::size_t r, UIntType a, std::size_t u, UIntType d, std::size_t s,
    UIntType b, std::size_t t, UIntType c, std::size_t l, UIntType f>
class mersenne_twister_engine;
typedef mersenne_twister_engine<uint32_t, 32, 624, 397, 31, 0x9908b0df, 11,
    0xffffffff, 7, 0x9d2c5680, 15, 0xefc60000, 18, 1812433253> mt19937;
}
}

namespace gtsam {

/// Represents a 3D point on a unit sphere.
class Sphere2: public DerivedValue<Sphere2> {

private:

  Point3 p_; ///< The location of the point on the unit sphere
  mutable Matrix B_; ///< Cached basis

public:

  /// @name Constructors
  /// @{

  /// Default constructor
  Sphere2() :
      p_(1.0, 0.0, 0.0) {
  }

  /// Construct from point
  Sphere2(const Point3& p) :
      p_(p / p.norm()) {
  }

  /// Construct from x,y,z
  Sphere2(double x, double y, double z) :
      p_(x, y, z) {
    p_ = p_ / p_.norm();
  }

  /// Named constructor from Point3 with optional Jacobian
  static Sphere2 FromPoint3(const Point3& point,
      boost::optional<Matrix&> H = boost::none);

  /// Random direction, using boost::uniform_on_sphere
  static Sphere2 Random(boost::random::mt19937 & rng);

  /// @}

  /// @name Testable
  /// @{

  /// The print fuction
  void print(const std::string& s = std::string()) const;

  /// The equals function with tolerance
  bool equals(const Sphere2& s, double tol = 1e-9) const {
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
  Matrix basis() const;

  /// Return skew-symmetric associated with 3D point on unit sphere
  Matrix skew() const;

  /// Return unit-norm Point3
  const Point3& point3(boost::optional<Matrix&> H = boost::none) const {
    if (H)
      *H = basis();
    return p_;
  }

  /// Return scaled direction as Point3
  friend Point3 operator*(double s, const Sphere2& d) {
    return s*d.p_;
  }

  /// Signed, vector-valued error between two directions
  Vector error(const Sphere2& q,
      boost::optional<Matrix&> H = boost::none) const;

  /// Distance between two directions
  double distance(const Sphere2& q,
      boost::optional<Matrix&> H = boost::none) const;

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
  Sphere2 retract(const Vector& v, Sphere2::CoordinatesMode mode = SPHERE2_DEFAULT_COORDINATES_MODE) const;

  /// The local coordinates function
  Vector localCoordinates(const Sphere2& s, Sphere2::CoordinatesMode mode = SPHERE2_DEFAULT_COORDINATES_MODE) const;

  /// @}
};

} // namespace gtsam

