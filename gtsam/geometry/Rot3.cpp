/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file    Rot3.cpp
 * @brief   Rotation, common code between Rotation matrix and Quaternion
 * @author  Alireza Fathi
 * @author  Christian Potthast
 * @author  Frank Dellaert
 * @author  Richard Roberts
 */

#include <gtsam/geometry/Rot3.h>
#include <boost/math/constants/constants.hpp>
#include <boost/random.hpp>
#include <cmath>

using namespace std;

namespace gtsam {

/* ************************************************************************* */
void Rot3::print(const std::string& s) const {
  gtsam::print((Matrix)matrix(), s);
}

/* ************************************************************************* */
Rot3 Rot3::rodriguez(const Point3& w, double theta) {
  return rodriguez((Vector)w.vector(),theta);
}

/* ************************************************************************* */
Rot3 Rot3::rodriguez(const Unit3& w, double theta) {
  return rodriguez(w.point3(),theta);
}

/* ************************************************************************* */
Rot3 Rot3::Random(boost::mt19937 & rng) {
  // TODO allow any engine without including all of boost :-(
  Unit3 w = Unit3::Random(rng);
  boost::uniform_real<double> randomAngle(-M_PI,M_PI);
  double angle = randomAngle(rng);
  return rodriguez(w,angle);
}

/* ************************************************************************* */
Rot3 Rot3::rodriguez(const Vector3& w) {
  double t = w.norm();
  if (t < 1e-10) return Rot3();
  return rodriguez(w/t, t);
}

/* ************************************************************************* */
Rot3 Rot3::retract(const Vector& omega, OptionalJacobian<3, 3> Hthis,
    OptionalJacobian<3, 3> Hv, Rot3::CoordinatesMode mode) const {
  if (Hthis || Hv) CONCEPT_NOT_IMPLEMENTED;
  return retract(omega, mode);
}

/* ************************************************************************* */
Vector3 Rot3::localCoordinates(const Rot3& R2, OptionalJacobian<3, 3> Horigin,
    OptionalJacobian<3, 3> H2, Rot3::CoordinatesMode mode) const {
  if (Horigin || H2) CONCEPT_NOT_IMPLEMENTED;
  return localCoordinates(R2, mode);
}

/* ************************************************************************* */
bool Rot3::equals(const Rot3 & R, double tol) const {
  return equal_with_abs_tol(matrix(), R.matrix(), tol);
}

/* ************************************************************************* */
Point3 Rot3::operator*(const Point3& p) const {
  return rotate(p);
}

/* ************************************************************************* */
Unit3 Rot3::rotate(const Unit3& p,
    OptionalJacobian<2,3> HR, OptionalJacobian<2,2> Hp) const {
  Matrix32 Dp;
  Unit3 q = Unit3(rotate(p.point3(Hp ? &Dp : 0)));
  if (Hp) *Hp = q.basis().transpose() * matrix() * Dp;
  if (HR) *HR = -q.basis().transpose() * matrix() * p.skew();
  return q;
}

/* ************************************************************************* */
Unit3 Rot3::unrotate(const Unit3& p,
    OptionalJacobian<2,3> HR, OptionalJacobian<2,2> Hp) const {
  Matrix32 Dp;
  Unit3 q = Unit3(unrotate(p.point3(Dp)));
  if (Hp) *Hp = q.basis().transpose() * matrix().transpose () * Dp;
  if (HR) *HR = q.basis().transpose() * q.skew();
  return q;
}

/* ************************************************************************* */
Unit3 Rot3::operator*(const Unit3& p) const {
  return rotate(p);
}

/* ************************************************************************* */
// see doc/math.lyx, SO(3) section
Point3 Rot3::unrotate(const Point3& p, OptionalJacobian<3,3> H1,
    OptionalJacobian<3,3> H2) const {
  const Matrix3& Rt = transpose();
  Point3 q(Rt * p.vector()); // q = Rt*p
  const double wx = q.x(), wy = q.y(), wz = q.z();
  if (H1)
    *H1 << 0.0, -wz, +wy, +wz, 0.0, -wx, -wy, +wx, 0.0;
  if (H2)
    *H2 = Rt;
  return q;
}

/* ************************************************************************* */
/// Follow Iserles05an, B10, pg 147, with a sign change in the second term (left version)
Matrix3 Rot3::dexpL(const Vector3& v) {
  if(zero(v)) return eye(3);
  Matrix3 x = skewSymmetric(v);
  Matrix3 x2 = x*x;
  double theta = v.norm(), vi = theta/2.0;
  double s1 = sin(vi)/vi;
  double s2 = (theta - sin(theta))/(theta*theta*theta);
  Matrix3 res = I_3x3 - 0.5*s1*s1*x + s2*x2;
  return res;
}

/* ************************************************************************* */
/// Follow Iserles05an, B11, pg 147, with a sign change in the second term (left version)
Matrix3 Rot3::dexpInvL(const Vector3& v) {
  if(zero(v)) return eye(3);
  Matrix3 x = skewSymmetric(v);
  Matrix3 x2 = x*x;
  double theta = v.norm(), vi = theta/2.0;
  double s2 = (theta*tan(M_PI_2-vi) - 2)/(2*theta*theta);
  Matrix3 res = I_3x3 + 0.5*x - s2*x2;
  return res;
}


/* ************************************************************************* */
Point3 Rot3::column(int index) const{
  if(index == 3)
    return r3();
  else if(index == 2)
    return r2();
  else if(index == 1)
    return r1(); // default returns r1
  else
    throw invalid_argument("Argument to Rot3::column must be 1, 2, or 3");
}

/* ************************************************************************* */
Vector3 Rot3::xyz() const {
  Matrix3 I;Vector3 q;
  boost::tie(I,q)=RQ(matrix());
  return q;
}

/* ************************************************************************* */
Vector3 Rot3::ypr() const {
  Vector3 q = xyz();
  return Vector3(q(2),q(1),q(0));
}

/* ************************************************************************* */
Vector3 Rot3::rpy() const {
  return xyz();
}

/* ************************************************************************* */
Vector Rot3::quaternion() const {
  Quaternion q = toQuaternion();
  Vector v(4);
  v(0) = q.w();
  v(1) = q.x();
  v(2) = q.y();
  v(3) = q.z();
  return v;
}

/* ************************************************************************* */
Matrix3 Rot3::rightJacobianExpMapSO3(const Vector3& x)    {
  // x is the axis-angle representation (exponential coordinates) for a rotation
  double normx = norm_2(x); // rotation angle
  Matrix3 Jr;
  if (normx < 10e-8){
    Jr = I_3x3;
  }
  else{
    const Matrix3 X = skewSymmetric(x); // element of Lie algebra so(3): X = x^
    Jr = I_3x3 - ((1-cos(normx))/(normx*normx)) * X +
        ((normx-sin(normx))/(normx*normx*normx)) * X * X; // right Jacobian
  }
  return Jr;
}

/* ************************************************************************* */
Matrix3 Rot3::rightJacobianExpMapSO3inverse(const Vector3& x)    {
  // x is the axis-angle representation (exponential coordinates) for a rotation
  double normx = norm_2(x); // rotation angle
  Matrix3 Jrinv;

  if (normx < 10e-8){
    Jrinv = I_3x3;
  }
  else{
    const Matrix3 X = skewSymmetric(x); // element of Lie algebra so(3): X = x^
    Jrinv = I_3x3 +
        0.5 * X + (1/(normx*normx) - (1+cos(normx))/(2*normx * sin(normx))   ) * X * X;
  }
  return Jrinv;
}

/* ************************************************************************* */
pair<Matrix3, Vector3> RQ(const Matrix3& A) {

  double x = -atan2(-A(2, 1), A(2, 2));
  Rot3 Qx = Rot3::Rx(-x);
  Matrix3 B = A * Qx.matrix();

  double y = -atan2(B(2, 0), B(2, 2));
  Rot3 Qy = Rot3::Ry(-y);
  Matrix3 C = B * Qy.matrix();

  double z = -atan2(-C(1, 0), C(1, 1));
  Rot3 Qz = Rot3::Rz(-z);
  Matrix3 R = C * Qz.matrix();

  Vector xyz = Vector3(x, y, z);
  return make_pair(R, xyz);
}

/* ************************************************************************* */
ostream &operator<<(ostream &os, const Rot3& R) {
  os << "\n";
  os << '|' << R.r1().x() << ", " << R.r2().x() << ", " << R.r3().x() << "|\n";
  os << '|' << R.r1().y() << ", " << R.r2().y() << ", " << R.r3().y() << "|\n";
  os << '|' << R.r1().z() << ", " << R.r2().z() << ", " << R.r3().z() << "|\n";
  return os;
}

/* ************************************************************************* */
Rot3 Rot3::slerp(double t, const Rot3& other) const {
  // amazingly simple in GTSAM :-)
  assert(t>=0 && t<=1);
  Vector3 omega = localCoordinates(other, Rot3::EXPMAP);
  return retract(t * omega, Rot3::EXPMAP);
}

/* ************************************************************************* */

} // namespace gtsam

