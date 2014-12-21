/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file LieVector.h
 * @brief A wrapper around vector providing Lie compatibility
 * @author Alex Cunningham
 */

#pragma once

#ifdef _MSC_VER
#pragma message("LieVector.h is deprecated. Please use Eigen::Vector instead.")
#else
#warning "LieVector.h is deprecated. Please use Eigen::Vector instead."
#endif

#include <gtsam/base/Lie.h>
#include <gtsam/base/Vector.h>
#include <gtsam/base/DerivedValue.h>

namespace gtsam {

/**
 * @deprecated: LieScalar, LieVector and LieMatrix are obsolete in GTSAM 4.0 as
 * we can directly add double, Vector, and Matrix into values now, because of
 * gtsam::traits.
 */
struct LieVector : public Vector {

  enum { dimension = Eigen::Dynamic };

  /** default constructor - should be unnecessary */
  LieVector() {}

  /** initialize from a normal vector */
  LieVector(const Vector& v) : Vector(v) {}
  
  template <class V>
  LieVector(const V& v) : Vector(v) {}

// Currently TMP constructor causes ICE on MSVS 2013
#if (_MSC_VER < 1800)
  /** initialize from a fixed size normal vector */
  template<int N>
  LieVector(const Eigen::Matrix<double, N, 1>& v) : Vector(v) {}
#endif

  /** wrap a double */
  LieVector(double d) : Vector((Vector(1) << d).finished()) {}

  /** constructor with size and initial data, row order ! */
  GTSAM_EXPORT LieVector(size_t m, const double* const data);

  /** get the underlying vector */
  Vector vector() const {
    return static_cast<Vector>(*this);
  }

  /** print @param name optional string naming the object */
  GTSAM_EXPORT void print(const std::string& name="") const;

  /** equality up to tolerance */
  bool equals(const LieVector& expected, double tol=1e-5) const {
    return gtsam::equal(vector(), expected.vector(), tol);
  }

  // Manifold requirements

  /** Returns dimensionality of the tangent space */
  size_t dim() const { return this->size(); }

  /** Update the LieVector with a tangent space update */
  LieVector retract(const Vector& v) const { return LieVector(vector() + v); }

  /** @return the local coordinates of another object */
  Vector localCoordinates(const LieVector& t2) const { return LieVector(t2 - vector()); }

  // Group requirements

  /** identity - NOTE: no known size at compile time - so zero length */
  static LieVector identity() {
    throw std::runtime_error("LieVector::identity(): Don't use this function");
    return LieVector();
  }

  // Note: Manually specifying the 'gtsam' namespace for the optional Matrix arguments
  // This is a work-around for linux g++ 4.6.1 that incorrectly selects the Eigen::Matrix class
  // instead of the gtsam::Matrix class. This is related to deriving this class from an Eigen Vector
  // as the other geometry objects (Point3, Rot3, etc.) have this problem
  /** compose with another object */
  LieVector compose(const LieVector& p,
      OptionalJacobian<-1,-1> H1 = boost::none,
      OptionalJacobian<-1,-1> H2 = boost::none) const {
    if(H1) *H1 = eye(dim());
    if(H2) *H2 = eye(p.dim());

    return LieVector(vector() + p);
  }

  /** between operation */
  LieVector between(const LieVector& l2,
      OptionalJacobian<-1,-1> H1 = boost::none,
      OptionalJacobian<-1,-1> H2 = boost::none) const {
    if(H1) *H1 = -eye(dim());
    if(H2) *H2 = eye(l2.dim());
    return LieVector(l2.vector() - vector());
  }

  /** invert the object and yield a new one */
  LieVector inverse(OptionalJacobian<-1,-1> H=boost::none) const {
    if(H) *H = -eye(dim());

    return LieVector(-1.0 * vector());
  }

  // Lie functions

  /** Expmap around identity */
  static LieVector Expmap(const Vector& v, OptionalJacobian<-1,-1> H = boost::none) {
    if (H) { CONCEPT_NOT_IMPLEMENTED; }
    return LieVector(v);
  }

  /** Logmap around identity - just returns with default cast back */
  static Vector Logmap(const LieVector& p, OptionalJacobian<-1,-1> H = boost::none) {
    if (H) { CONCEPT_NOT_IMPLEMENTED; }
    return p;
  }

private:

  // Serialization function
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar & boost::serialization::make_nvp("Vector",
       boost::serialization::base_object<Vector>(*this));
  }
};


template<>
struct traits_x<LieVector> : public internal::LieGroup<LieVector> {};

} // \namespace gtsam
