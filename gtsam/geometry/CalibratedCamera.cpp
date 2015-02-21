/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file CalibratedCamera.cpp
 * @brief Calibrated camera for which only pose is unknown
 * @date Aug 17, 2009
 * @author Frank Dellaert
 */

#include <gtsam/geometry/Pose2.h>
#include <gtsam/geometry/CalibratedCamera.h>

namespace gtsam {

/* ************************************************************************* */
CalibratedCamera::CalibratedCamera(const Pose3& pose) :
    pose_(pose) {
}

/* ************************************************************************* */
CalibratedCamera::CalibratedCamera(const Vector &v) :
    pose_(Pose3::Expmap(v)) {
}

/* ************************************************************************* */
Point2 CalibratedCamera::project_to_camera(const Point3& P,
    OptionalJacobian<2, 3> H1) {
  if (H1) {
    double d = 1.0 / P.z(), d2 = d * d;
    *H1 << d, 0.0, -P.x() * d2, 0.0, d, -P.y() * d2;
  }
  return Point2(P.x() / P.z(), P.y() / P.z());
}

/* ************************************************************************* */
Point3 CalibratedCamera::backproject_from_camera(const Point2& p,
    const double scale) {
  return Point3(p.x() * scale, p.y() * scale, scale);
}

/* ************************************************************************* */
Pose3 CalibratedCamera::LevelPose(const Pose2& pose2, double height) {
  const double st = sin(pose2.theta()), ct = cos(pose2.theta());
  const Point3 x(st, -ct, 0), y(0, 0, -1), z(ct, st, 0);
  const Rot3 wRc(x, y, z);
  const Point3 t(pose2.x(), pose2.y(), height);
  return Pose3(wRc, t);
}

/* ************************************************************************* */
CalibratedCamera CalibratedCamera::Level(const Pose2& pose2, double height) {
  return CalibratedCamera(LevelPose(pose2, height));
}

/* ************************************************************************* */
Pose3 CalibratedCamera::LookatPose(const Point3& eye, const Point3& target,
    const Point3& upVector) {
  Point3 zc = target - eye;
  zc = zc / zc.norm();
  Point3 xc = (-upVector).cross(zc); // minus upVector since yc is pointing down
  xc = xc / xc.norm();
  Point3 yc = zc.cross(xc);
  return Pose3(Rot3(xc, yc, zc), eye);
}

/* ************************************************************************* */
CalibratedCamera CalibratedCamera::Lookat(const Point3& eye,
    const Point3& target, const Point3& upVector) {
  return CalibratedCamera(LookatPose(eye, target, upVector));
}

/* ************************************************************************* */
Point2 CalibratedCamera::project(const Point3& point,
    OptionalJacobian<2, 6> Dpose, OptionalJacobian<2, 3> Dpoint) const {

#ifdef CALIBRATEDCAMERA_CHAIN_RULE
  Matrix36 Dpose_;
  Matrix3 Dpoint_;
  Point3 q = pose_.transform_to(point, Dpose ? Dpose_ : 0, Dpoint ? Dpoint_ : 0);
#else
  Point3 q = pose_.transform_to(point);
#endif
  Point2 intrinsic = project_to_camera(q);

  // Check if point is in front of camera
  if (q.z() <= 0)
    throw CheiralityException();

  if (Dpose || Dpoint) {
#ifdef CALIBRATEDCAMERA_CHAIN_RULE
    // just implement chain rule
    if(Dpose && Dpoint) {
      Matrix23 H;
      project_to_camera(q,H);
      if (Dpose) *Dpose = H * (*Dpose_);
      if (Dpoint) *Dpoint = H * (*Dpoint_);
    }
#else
    // optimized version, see CalibratedCamera.nb
    const double z = q.z(), d = 1.0 / z;
    const double u = intrinsic.x(), v = intrinsic.y(), uv = u * v;
    if (Dpose)
      *Dpose << uv, -(1. + u * u), v, -d, 0., d * u, (1. + v * v), -uv, -u, 0., -d, d
          * v;
    if (Dpoint) {
      const Matrix3 R(pose_.rotation().matrix());
      Matrix23 Dpoint_;
      Dpoint_ << R(0, 0) - u * R(0, 2), R(1, 0) - u * R(1, 2), R(2, 0)
          - u * R(2, 2), R(0, 1) - v * R(0, 2), R(1, 1) - v * R(1, 2), R(2, 1)
          - v * R(2, 2);
      *Dpoint = d * Dpoint_;
    }
#endif
  }
  return intrinsic;
}

/* ************************************************************************* */
CalibratedCamera CalibratedCamera::retract(const Vector& d) const {
  return CalibratedCamera(pose().retract(d));
}

/* ************************************************************************* */
Vector CalibratedCamera::localCoordinates(const CalibratedCamera& T2) const {
  return pose().localCoordinates(T2.pose());
}

/* ************************************************************************* */
}
