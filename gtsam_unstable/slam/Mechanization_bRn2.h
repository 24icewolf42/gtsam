/**
 * @file Mechanization_bRn.h
 * @date Jan 25, 2012
 * @author Chris Beall
 * @author Frank Dellaert
 */

#pragma once

#include <gtsam/geometry/Rot3.h>
#include <gtsam/base/Vector.h>
#include <gtsam_unstable/base/dllexport.h>
#include <list>

namespace gtsam {

class GTSAM_UNSTABLE_EXPORT Mechanization_bRn2 {

private:
  Rot3 bRn_;  ///<   rotation from nav to body
  Vector x_g_; ///<  gyroscope bias
  Vector x_a_; ///<  accelerometer bias

public:

  /**
   * Constructor
   * @param initial_bRn initial rotation from nav to body frame
   * @param initial_x_g initial gyro bias
   * @param r3 Z-axis of rotated frame
   */
  Mechanization_bRn2(const Rot3& initial_bRn = Rot3(),
      const Vector& initial_x_g = zero(3), const Vector& initial_x_a = zero(3)) :
      bRn_(initial_bRn), x_g_(initial_x_g), x_a_(initial_x_a) {
  }

  Vector b_g(double g_e) {
    Vector n_g = Vector_(3, 0.0, 0.0, g_e);
    return (bRn_ * n_g).vector();
  }

  Rot3 bRn() const {return bRn_; }
  Vector x_g() const { return x_g_; }
  Vector x_a() const { return x_a_; }

  /**
   * Initialize the first Mechanization state
   * @param U a list of gyro measurement vectors
   * @param F a list of accelerometer measurement vectors
   * @param g_e gravity magnitude
   * @param flat flag saying whether this is a flat trim init
   */
  static Mechanization_bRn2 initializeVector(const std::list<Vector>& U,
      const std::list<Vector>& F, const double g_e = 0, bool flat=false);

  /// Matrix version of initialize
  static Mechanization_bRn2 initialize(const Matrix& U,
      const Matrix& F, const double g_e = 0, bool flat=false);

  /**
   * Correct AHRS full state given error state dx
   * @param obj The current state
   * @param dx The error used to correct and return a new state
   */
  Mechanization_bRn2 correct(const Vector& dx) const;

  /**
   * Integrate to get new state
   * @param obj The current state
   * @param u gyro measurement to integrate
   * @param dt time elapsed since previous state in seconds
   */
  Mechanization_bRn2 integrate(const Vector& u, const double dt) const;

  /// print
  void print(const std::string& s = "") const {
    bRn_.print(s + ".R");

    gtsam::print(x_g_, s + ".x_g");
    gtsam::print(x_a_, s + ".x_a");
  }

};

} // namespace gtsam
