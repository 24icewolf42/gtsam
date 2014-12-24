/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file LieScalar.h
 * @brief A wrapper around scalar providing Lie compatibility
 * @author Kai Ni
 */

#pragma once

#ifdef _MSC_VER
#pragma message("LieScalar.h is deprecated. Please use double/float instead.")
#else
  #warning "LieScalar.h is deprecated. Please use double/float instead."
#endif

#include <gtsam/dllexport.h>
#include <gtsam/base/DerivedValue.h>
#include <gtsam/base/Lie.h>

namespace gtsam {

  /**
   * @deprecated: LieScalar, LieVector and LieMatrix are obsolete in GTSAM 4.0 as
   * we can directly add double, Vector, and Matrix into values now, because of
   * gtsam::traits.
   */
  struct GTSAM_EXPORT LieScalar {

    /** default constructor */
    LieScalar() : d_(0.0) {}

    /** wrap a double */
    explicit LieScalar(double d) : d_(d) {}

    /** access the underlying value */
    double value() const { return d_; }

    /** Automatic conversion to underlying value */
    operator double() const { return d_; }

    /** print @param name optional string naming the object */
    void print(const std::string& name="") const;

    /** equality up to tolerance */
    bool equals(const LieScalar& expected, double tol=1e-5) const {
      return fabs(expected.d_ - d_) <= tol;
    }

    // Manifold requirements

    /** Returns dimensionality of the tangent space */
    size_t dim() const { return 1; }
    static size_t Dim() { return 1; }

    /** Update the LieScalar with a tangent space update */
    LieScalar retract(const Vector& v) const { return LieScalar(value() + v(0)); }

    /** @return the local coordinates of another object */
    Vector localCoordinates(const LieScalar& t2) const { return (Vector(1) << (t2.value() - value())).finished(); }

    // Group requirements

    /** identity */
    static LieScalar identity() {
      return LieScalar();
    }

    /** compose with another object */
    LieScalar compose(const LieScalar& p,
        boost::optional<Matrix&> H1=boost::none,
        boost::optional<Matrix&> H2=boost::none) const {
      if(H1) *H1 = eye(1);
      if(H2) *H2 = eye(1);
      return LieScalar(d_ + p.d_);
    }

    /** between operation */
    LieScalar between(const LieScalar& l2,
        boost::optional<Matrix&> H1=boost::none,
        boost::optional<Matrix&> H2=boost::none) const {
      if(H1) *H1 = -eye(1);
      if(H2) *H2 = eye(1);
      return LieScalar(l2.value() - value());
    }

    /** invert the object and yield a new one */
    LieScalar inverse() const {
      return LieScalar(-1.0 * value());
    }

    // Lie functions

    /** Expmap around identity */
    static LieScalar Expmap(const Vector& v) { return LieScalar(v(0)); }

    /** Logmap around identity - just returns with default cast back */
    static Vector Logmap(const LieScalar& p) { return (Vector(1) << p.value()).finished(); }

    /// Left-trivialized derivative of the exponential map
    static Matrix ExpmapDerivative(const Vector& v) {
      return eye(1);
    }

    /// Left-trivialized derivative inverse of the exponential map
    static Matrix LogmapDerivative(const Vector& v) {
      return eye(1);
    }

  private:
      double d_;
  };

  // Define GTSAM traits
  namespace traits {

  template<>
  struct is_group<LieScalar> : public boost::true_type {
  };

  template<>
  struct is_manifold<LieScalar> : public boost::true_type {
  };

  template<>
  struct dimension<LieScalar> : public boost::integral_constant<int, 1> {
  };

  }

} // \namespace gtsam
