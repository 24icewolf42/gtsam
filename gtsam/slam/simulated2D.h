/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file    simulated2D.h
 * @brief   measurement functions and derivatives for simulated 2D robot
 * @author  Frank Dellaert
 */

// \callgraph
#pragma once

#include <gtsam/geometry/Point2.h>
#include <gtsam/nonlinear/NonlinearFactor.h>
#include <gtsam/nonlinear/NonlinearFactorGraph.h>

// \namespace

namespace simulated2D {

  using namespace gtsam;

  // Simulated2D robots have no orientation, just a position

  /// Convenience function for constructing a pose key
  inline Symbol PoseKey(Index j) { return Symbol('x', j); }

  /// Convenience function for constructing a landmark key
  inline Symbol PointKey(Index j) { return Symbol('l', j); }

  /**
   *  Custom Values class that holds poses and points, mainly used as a convenience for MATLAB wrapper
   */
  class Values: public gtsam::Values {
  private:
  	int nrPoses_, nrPoints_;

  public:
    typedef gtsam::Values Base;  ///< base class
    typedef boost::shared_ptr<Point2> sharedPoint;  ///< shortcut to shared Point type

    /// Constructor
    Values() : nrPoses_(0), nrPoints_(0) {
    }

    /// Copy constructor
    Values(const Base& base) :
        Base(base), nrPoses_(0), nrPoints_(0) {
    }

    /// Insert a pose
    void insertPose(Index j, const Point2& p) {
      insert(PoseKey(j), p);
      nrPoses_++;
    }

    /// Insert a point
    void insertPoint(Index j, const Point2& p) {
      insert(PointKey(j), p);
      nrPoints_++;
    }

    /// Number of poses
    int nrPoses() const {
      return nrPoses_;
    }

    /// Number of points
    int nrPoints() const {
      return nrPoints_;
    }

    /// Return pose i
    Point2 pose(Index j) const {
      return at<Point2>(PoseKey(j));
    }

    /// Return point j
    Point2 point(Index j) const {
      return at<Point2>(PointKey(j));
    }
  };


  /// Prior on a single pose
  inline Point2 prior(const Point2& x) {
    return x;
  }

  /// Prior on a single pose, optionally returns derivative
  Point2 prior(const Point2& x, boost::optional<Matrix&> H = boost::none);

  /// odometry between two poses
  inline Point2 odo(const Point2& x1, const Point2& x2) {
    return x2 - x1;
  }

  /// odometry between two poses, optionally returns derivative
  Point2 odo(const Point2& x1, const Point2& x2, boost::optional<Matrix&> H1 =
      boost::none, boost::optional<Matrix&> H2 = boost::none);

  /// measurement between landmark and pose
  inline Point2 mea(const Point2& x, const Point2& l) {
    return l - x;
  }

  /// measurement between landmark and pose, optionally returns derivative
  Point2 mea(const Point2& x, const Point2& l, boost::optional<Matrix&> H1 =
      boost::none, boost::optional<Matrix&> H2 = boost::none);

  /**
   *  Unary factor encoding a soft prior on a vector
   */
  template<class VALUE = Point2>
  class GenericPrior: public NonlinearFactor1<VALUE> {
  public:
    typedef NonlinearFactor1<VALUE> Base;  ///< base class
    typedef boost::shared_ptr<GenericPrior<VALUE> > shared_ptr;
    typedef VALUE Pose; ///< shortcut to Pose type

    Pose measured_; ///< prior mean

    /// Create generic prior
    GenericPrior(const Pose& z, const SharedNoiseModel& model, const Symbol& key) :
      Base(model, key), measured_(z) {
    }

    /// Return error and optional derivative
    Vector evaluateError(const Pose& x, boost::optional<Matrix&> H = boost::none) const {
      return (prior(x, H) - measured_).vector();
    }

  private:

    /// Default constructor
    GenericPrior() {
    }

    /// Serialization function
    friend class boost::serialization::access;
    template<class ARCHIVE>
    void serialize(ARCHIVE & ar, const unsigned int version) {
      ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Base);
      ar & BOOST_SERIALIZATION_NVP(measured_);
    }
  };

  /**
   * Binary factor simulating "odometry" between two Vectors
   */
  template<class VALUE = Point2>
  class GenericOdometry: public NonlinearFactor2<VALUE, VALUE> {
  public:
    typedef NonlinearFactor2<VALUE, VALUE> Base; ///< base class
    typedef boost::shared_ptr<GenericOdometry<VALUE> > shared_ptr;
    typedef VALUE Pose; ///< shortcut to Pose type

    Pose measured_; ///< odometry measurement

    /// Create odometry
    GenericOdometry(const Pose& measured, const SharedNoiseModel& model, const Symbol& key1, const Symbol& key2) :
          Base(model, key1, key2), measured_(measured) {
    }

    /// Evaluate error and optionally return derivatives
    Vector evaluateError(const Pose& x1, const Pose& x2,
        boost::optional<Matrix&> H1 = boost::none,
        boost::optional<Matrix&> H2 = boost::none) const {
      return (odo(x1, x2, H1, H2) - measured_).vector();
    }

  private:

    /// Default constructor
    GenericOdometry() {
    }

    /// Serialization function
    friend class boost::serialization::access;
    template<class ARCHIVE>
    void serialize(ARCHIVE & ar, const unsigned int version) {
      ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Base);
      ar & BOOST_SERIALIZATION_NVP(measured_);
    }
  };

  /**
   * Binary factor simulating "measurement" between two Vectors
   */
  template<class POSE, class LANDMARK>
  class GenericMeasurement: public NonlinearFactor2<POSE, LANDMARK> {
  public:
    typedef NonlinearFactor2<POSE, LANDMARK> Base;  ///< base class
    typedef boost::shared_ptr<GenericMeasurement<POSE, LANDMARK> > shared_ptr;
    typedef POSE Pose; ///< shortcut to Pose type
    typedef LANDMARK Landmark; ///< shortcut to Landmark type

    Landmark measured_; ///< Measurement

    /// Create measurement factor
    GenericMeasurement(const Landmark& measured, const SharedNoiseModel& model, const Symbol& poseKey, const Symbol& landmarkKey) :
          Base(model, poseKey, landmarkKey), measured_(measured) {
    }

    /// Evaluate error and optionally return derivatives
    Vector evaluateError(const Pose& x1, const Landmark& x2,
        boost::optional<Matrix&> H1 = boost::none,
        boost::optional<Matrix&> H2 = boost::none) const {
      return (mea(x1, x2, H1, H2) - measured_).vector();
    }

  private:

    /// Default constructor
    GenericMeasurement() {
    }

    /// Serialization function
    friend class boost::serialization::access;
    template<class ARCHIVE>
    void serialize(ARCHIVE & ar, const unsigned int version) {
      ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Base);
      ar & BOOST_SERIALIZATION_NVP(measured_);
    }
  };

  /** Typedefs for regular use */
  typedef GenericPrior<Point2> Prior;
  typedef GenericOdometry<Point2> Odometry;
  typedef GenericMeasurement<Point2, Point2> Measurement;

  // Specialization of a graph for this example domain
  // TODO: add functions to add factor types
  class Graph : public NonlinearFactorGraph {
  public:
    Graph() {}
  };

} // namespace simulated2D
