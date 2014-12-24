/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file  Pose3.cpp
 * @brief 3D Pose
 */

#include <gtsam/geometry/Pose3.h>
#include <gtsam/geometry/Pose2.h>
#include <gtsam/geometry/concepts.h>
#include <gtsam/base/Lie-inl.h>
#include <boost/foreach.hpp>
#include <iostream>
#include <cmath>

using namespace std;

namespace gtsam {

/** Explicit instantiation of base class to export members */
INSTANTIATE_LIE(Pose3);

/** instantiate concept checks */
GTSAM_CONCEPT_POSE_INST(Pose3);

/* ************************************************************************* */
Pose3::Pose3(const Pose2& pose2) :
    R_(Rot3::rodriguez(0, 0, pose2.theta())), t_(
        Point3(pose2.x(), pose2.y(), 0)) {
}

/* ************************************************************************* */
// Calculate Adjoint map
// Ad_pose is 6*6 matrix that when applied to twist xi, returns Ad_pose(xi)
// Experimental - unit tests of derivatives based on it do not check out yet
Matrix6 Pose3::AdjointMap() const {
  const Matrix3 R = R_.matrix();
  const Vector3 t = t_.vector();
  Matrix3 A = skewSymmetric(t) * R;
  Matrix6 adj;
  adj << R, Z_3x3, A, R;
  return adj;
}

/* ************************************************************************* */
Matrix6 Pose3::adjointMap(const Vector6& xi) {
  Matrix3 w_hat = skewSymmetric(xi(0), xi(1), xi(2));
  Matrix3 v_hat = skewSymmetric(xi(3), xi(4), xi(5));
  Matrix6 adj;
  adj << w_hat, Z_3x3, v_hat, w_hat;

  return adj;
}

/* ************************************************************************* */
Vector6 Pose3::adjoint(const Vector6& xi, const Vector6& y,
    OptionalJacobian<6,6> H) {
  if (H) {
    H->setZero();
    for (int i = 0; i < 6; ++i) {
      Vector6 dxi;
      dxi.setZero();
      dxi(i) = 1.0;
      Matrix6 Gi = adjointMap(dxi);
      H->col(i) = Gi * y;
    }
  }
  return adjointMap(xi) * y;
}

/* ************************************************************************* */
Vector6 Pose3::adjointTranspose(const Vector6& xi, const Vector6& y,
    OptionalJacobian<6,6> H) {
  if (H) {
    H->setZero();
    for (int i = 0; i < 6; ++i) {
      Vector6 dxi;
      dxi.setZero();
      dxi(i) = 1.0;
      Matrix6 GTi = adjointMap(dxi).transpose();
      H->col(i) = GTi * y;
    }
  }
  return adjointMap(xi).transpose() * y;
}

/* ************************************************************************* */
void Pose3::print(const string& s) const {
  cout << s;
  R_.print("R:\n");
  t_.print("t: ");
}

/* ************************************************************************* */
bool Pose3::equals(const Pose3& pose, double tol) const {
  return R_.equals(pose.R_, tol) && t_.equals(pose.t_, tol);
}

/* ************************************************************************* */
/** Modified from Murray94book version (which assumes w and v normalized?) */
Pose3 Pose3::Expmap(const Vector& xi) {

  // get angular velocity omega and translational velocity v from twist xi
  Point3 w(xi(0), xi(1), xi(2)), v(xi(3), xi(4), xi(5));

  double theta = w.norm();
  if (theta < 1e-10) {
    static const Rot3 I;
    return Pose3(I, v);
  } else {
    Point3 n(w / theta); // axis unit vector
    Rot3 R = Rot3::rodriguez(n.vector(), theta);
    double vn = n.dot(v); // translation parallel to n
    Point3 n_cross_v = n.cross(v); // points towards axis
    Point3 t = (n_cross_v - R * n_cross_v) / theta + vn * n;
    return Pose3(R, t);
  }
}

/* ************************************************************************* */
Vector6 Pose3::Logmap(const Pose3& p) {
  Vector3 w = Rot3::Logmap(p.rotation()), T = p.translation().vector();
  double t = w.norm();
  if (t < 1e-10) {
    Vector6 log;
    log << w, T;
    return log;
  } else {
    Matrix3 W = skewSymmetric(w / t);
    // Formula from Agrawal06iros, equation (14)
    // simplified with Mathematica, and multiplying in T to avoid matrix math
    double Tan = tan(0.5 * t);
    Vector3 WT = W * T;
    Vector3 u = T - (0.5 * t) * WT + (1 - t / (2. * Tan)) * (W * WT);
    Vector6 log;
    log << w, u;
    return log;
  }
}

/* ************************************************************************* */
Pose3 Pose3::retractFirstOrder(const Vector& xi) const {
  Vector3 omega(sub(xi, 0, 3));
  Point3 v(sub(xi, 3, 6));
  Rot3 R = R_.retract(omega); // R is done exactly
  Point3 t = t_ + R_ * v; // First order t approximation
  return Pose3(R, t);
}

/* ************************************************************************* */
// Different versions of retract
Pose3 Pose3::retract(const Vector& xi, Pose3::CoordinatesMode mode) const {
  if (mode == Pose3::EXPMAP) {
    // Lie group exponential map, traces out geodesic
    return compose(Expmap(xi));
  } else if (mode == Pose3::FIRST_ORDER) {
    // First order
    return retractFirstOrder(xi);
  } else {
    // Point3 t = t_.retract(v.vector()); // Incorrect version retracts t independently
    // Point3 t = t_ + R_ * (v+Point3(omega).cross(v)/2); // Second order t approximation
    assert(false);
    exit(1);
  }
}

/* ************************************************************************* */
// different versions of localCoordinates
Vector6 Pose3::localCoordinates(const Pose3& T,
    Pose3::CoordinatesMode mode) const {
  if (mode == Pose3::EXPMAP) {
    // Lie group logarithm map, exact inverse of exponential map
    return Logmap(between(T));
  } else if (mode == Pose3::FIRST_ORDER) {
    // R is always done exactly in all three retract versions below
    Vector3 omega = R_.localCoordinates(T.rotation());

    // Incorrect version
    // Independently computes the logmap of the translation and rotation
    // Vector v = t_.localCoordinates(T.translation());

    // Correct first order t inverse
    Point3 d = R_.unrotate(T.translation() - t_);

    // TODO: correct second order t inverse
    Vector6 local;
    local << omega(0), omega(1), omega(2), d.x(), d.y(), d.z();
    return local;
  } else {
    assert(false);
    exit(1);
  }
}

/* ************************************************************************* */
Matrix6 Pose3::ExpmapDerivative(const Vector6& xi) {
  Vector3 w(sub(xi, 0, 3));
  Matrix3 Jw = Rot3::ExpmapDerivative(w);

  Vector3 v(sub(xi, 3, 6));
  Matrix3 V = skewSymmetric(v);
  Matrix3 W = skewSymmetric(w);

#ifdef NUMERICAL_EXPMAP_DERIV
  Matrix3 Qj = Z_3x3;
  double invFac = 1.0;
  Matrix3 Q = Z_3x3;
  Matrix3 Wj = I_3x3;
  for (size_t j=1; j<10; ++j) {
    Qj = Qj*W + Wj*V;
    invFac = -invFac/(j+1);
    Q = Q + invFac*Qj;
    Wj = Wj*W;
  }
#else
  // The closed-form formula in Barfoot14tro eq. (102)
  double phi = w.norm(), sinPhi = sin(phi), cosPhi = cos(phi), phi2 = phi * phi,
      phi3 = phi2 * phi, phi4 = phi3 * phi, phi5 = phi4 * phi;
  // Invert the sign of odd-order terms to have the right Jacobian
  Matrix3 Q = -0.5*V + (phi-sinPhi)/phi3*(W*V + V*W - W*V*W)
          + (1-phi2/2-cosPhi)/phi4*(W*W*V + V*W*W - 3*W*V*W)
          - 0.5*((1-phi2/2-cosPhi)/phi4 - 3*(phi-sinPhi-phi3/6)/phi5)*(W*V*W*W + W*W*V*W);
#endif

  Matrix6 J;
  J = (Matrix(6,6) << Jw, Z_3x3, Q, Jw).finished();
  return J;
}

/* ************************************************************************* */
Matrix6 Pose3::LogmapDerivative(const Vector6& xi) {
  Vector3 w(sub(xi, 0, 3));
  Matrix3 Jw = Rot3::LogmapDerivative(w);

  Vector3 v(sub(xi, 3, 6));
  Matrix3 V = skewSymmetric(v);
  Matrix3 W = skewSymmetric(w);

  // The closed-form formula in Barfoot14tro eq. (102)
  double phi = w.norm(), sinPhi = sin(phi), cosPhi = cos(phi), phi2 = phi * phi,
      phi3 = phi2 * phi, phi4 = phi3 * phi, phi5 = phi4 * phi;
  // Invert the sign of odd-order terms to have the right Jacobian
  Matrix3 Q = -0.5*V + (phi-sinPhi)/phi3*(W*V + V*W - W*V*W)
                        + (1-phi2/2-cosPhi)/phi4*(W*W*V + V*W*W - 3*W*V*W)
                        - 0.5*((1-phi2/2-cosPhi)/phi4 - 3*(phi-sinPhi-phi3/6)/phi5)*(W*V*W*W + W*W*V*W);
  Matrix3 Q2 = -Jw*Q*Jw;

  Matrix6 J;
  J = (Matrix(6,6) << Jw, Z_3x3, Q2, Jw).finished();
  return J;
}

/* ************************************************************************* */
Matrix4 Pose3::matrix() const {
  const Matrix3 R = R_.matrix();
  const Vector3 T = t_.vector();
  Matrix14 A14;
  A14 << 0.0, 0.0, 0.0, 1.0;
  Matrix4 mat;
  mat << R, T, A14;
  return mat;
}

/* ************************************************************************* */
Pose3 Pose3::transform_to(const Pose3& pose) const {
  Rot3 cRv = R_ * Rot3(pose.R_.inverse());
  Point3 t = pose.transform_to(t_);
  return Pose3(cRv, t);
}

/* ************************************************************************* */
Point3 Pose3::transform_from(const Point3& p, OptionalJacobian<3,6> Dpose,
    OptionalJacobian<3,3> Dpoint) const {
  if (Dpose) {
    const Matrix3 R = R_.matrix();
    Matrix3 DR = R * skewSymmetric(-p.x(), -p.y(), -p.z());
    (*Dpose) << DR, R;
  }
  if (Dpoint)
    *Dpoint = R_.matrix();
  return R_ * p + t_;
}

/* ************************************************************************* */
Point3 Pose3::transform_to(const Point3& p, OptionalJacobian<3,6> Dpose,
    OptionalJacobian<3,3> Dpoint) const {
  // Only get transpose once, to avoid multiple allocations,
  // as well as multiple conversions in the Quaternion case
  const Matrix3 Rt = R_.transpose();
  const Point3 q(Rt*(p - t_).vector());
  if (Dpose) {
    const double wx = q.x(), wy = q.y(), wz = q.z();
    (*Dpose) <<
        0.0, -wz, +wy,-1.0, 0.0, 0.0,
        +wz, 0.0, -wx, 0.0,-1.0, 0.0,
        -wy, +wx, 0.0, 0.0, 0.0,-1.0;
  }
  if (Dpoint)
    *Dpoint = Rt;
  return q;
}

/* ************************************************************************* */
Pose3 Pose3::compose(const Pose3& p2, OptionalJacobian<6,6> H1,
    OptionalJacobian<6,6> H2) const {
  if (H1) *H1 = p2.inverse().AdjointMap();
  if (H2) *H2 = I_6x6;
  return (*this) * p2;
}

/* ************************************************************************* */
Pose3 Pose3::inverse(OptionalJacobian<6,6> H1) const {
  if (H1) *H1 = -AdjointMap();
  Rot3 Rt = R_.inverse();
  return Pose3(Rt, Rt * (-t_));
}

/* ************************************************************************* */
// between = compose(p2,inverse(p1));
Pose3 Pose3::between(const Pose3& p2, OptionalJacobian<6,6> H1,
    OptionalJacobian<6,6> H2) const {
  Pose3 result = inverse() * p2;
  if (H1) *H1 = -result.inverse().AdjointMap();
  if (H2) *H2 = I_6x6;
  return result;
}

/* ************************************************************************* */
double Pose3::range(const Point3& point, OptionalJacobian<1, 6> H1,
    OptionalJacobian<1, 3> H2) const {
  if (!H1 && !H2)
    return transform_to(point).norm();
  else {
    Matrix36 D1;
    Matrix3 D2;
    Point3 d = transform_to(point, H1 ? &D1 : 0, H2 ? &D2 : 0);
    const double x = d.x(), y = d.y(), z = d.z(), d2 = x * x + y * y + z * z,
        n = sqrt(d2);
    Matrix13 D_result_d;
    D_result_d << x / n, y / n, z / n;
    if (H1) *H1 = D_result_d * D1;
    if (H2) *H2 = D_result_d * D2;
    return n;
  }
}

/* ************************************************************************* */
double Pose3::range(const Pose3& pose, OptionalJacobian<1,6> H1,
    OptionalJacobian<1,6> H2) const {
  Matrix13 D2;
  double r = range(pose.translation(), H1, H2? &D2 : 0);
  if (H2) {
    Matrix13 H2_ = D2 * pose.rotation().matrix();
    *H2 << Matrix13::Zero(), H2_;
  }
  return r;
}

/* ************************************************************************* */
boost::optional<Pose3> align(const vector<Point3Pair>& pairs) {
  const size_t n = pairs.size();
  if (n < 3)
    return boost::none; // we need at least three pairs

  // calculate centroids
  Vector cp = zero(3), cq = zero(3);
  BOOST_FOREACH(const Point3Pair& pair, pairs){
  cp += pair.first.vector();
  cq += pair.second.vector();
}
  double f = 1.0 / n;
  cp *= f;
  cq *= f;

  // Add to form H matrix
  Matrix3 H = Eigen::Matrix3d::Zero();
  BOOST_FOREACH(const Point3Pair& pair, pairs){
  Vector dp = pair.first.vector() - cp;
  Vector dq = pair.second.vector() - cq;
  H += dp * dq.transpose();
}

// Compute SVD
  Matrix U,V;
  Vector S;
  svd(H, U, S, V);

  // Recover transform with correction from Eggert97machinevisionandapplications
  Matrix3 UVtranspose = U * V.transpose();
  Matrix3 detWeighting = I_3x3;
  detWeighting(2, 2) = UVtranspose.determinant();
  Rot3 R(Matrix(V * detWeighting * U.transpose()));
  Point3 t = Point3(cq) - R * Point3(cp);
  return Pose3(R, t);
}

/* ************************************************************************* */
std::ostream &operator<<(std::ostream &os, const Pose3& pose) {
  os << pose.rotation() << "\n" << pose.translation() << endl;
  return os;
}

} // namespace gtsam
