/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 *  @file  ImuFactor.h
 *  @author Luca Carlone
 *  @author Stephen Williams
 *  @author Richard Roberts
 *  @author Vadim Indelman
 *  @author David Jensen
 *  @author Frank Dellaert
 **/

#pragma once

/* GTSAM includes */
#include <gtsam/nonlinear/NonlinearFactor.h>
#include <gtsam/navigation/PreintegrationBase.h>
#include <gtsam/base/debug.h>

namespace gtsam {

/**
 *
 * @addtogroup SLAM
 *
 * If you are using the factor, please cite:
 * L. Carlone, Z. Kira, C. Beall, V. Indelman, F. Dellaert, Eliminating conditionally
 * independent sets in factor graphs: a unifying perspective based on smart factors,
 * Int. Conf. on Robotics and Automation (ICRA), 2014.
 *
 ** REFERENCES:
 * [1] G.S. Chirikjian, "Stochastic Models, Information Theory, and Lie Groups", Volume 2, 2008.
 * [2] T. Lupton and S.Sukkarieh, "Visual-Inertial-Aided Navigation for High-Dynamic Motion in Built
 * Environments Without Initial Conditions", TRO, 28(1):61-76, 2012.
 * [3] L. Carlone, S. Williams, R. Roberts, "Preintegrated IMU factor: Computation of the Jacobian Matrices", Tech. Report, 2013.
 */

/**
 * Struct to hold return variables by the Predict Function
 */
struct PoseVelocity {
  Pose3 pose;
  Vector3 velocity;
  PoseVelocity(const Pose3& _pose, const Vector3& _velocity) :
    pose(_pose), velocity(_velocity) {
  }
};

/**
 * ImuFactor is a 5-ways factor involving previous state (pose and velocity of the vehicle at previous time step),
 * current state (pose and velocity at current time step), and the bias estimate. According to the
 * preintegration scheme proposed in [2], the ImuFactor includes many IMU measurements, which are
 * "summarized" using the PreintegratedMeasurements class.
 * Note that this factor does not force "temporal consistency" of the biases (which are usually
 * slowly varying quantities), see also CombinedImuFactor for more details.
 */
class ImuFactor: public NoiseModelFactor5<Pose3,Vector3,Pose3,Vector3,imuBias::ConstantBias> {
public:

  /**
   * PreintegratedMeasurements accumulates (integrates) the IMU measurements
   * (rotation rates and accelerations) and the corresponding covariance matrix.
   * The measurements are then used to build the Preintegrated IMU factor (ImuFactor).
   * Integration is done incrementally (ideally, one integrates the measurement as soon as it is received
   * from the IMU) so as to avoid costly integration at time of factor construction.
   */
  class PreintegratedMeasurements: public PreintegrationBase {

    friend class ImuFactor;

  protected:

    Eigen::Matrix<double,9,9> measurementCovariance_; ///< (continuous-time uncertainty) "Covariance" of
    ///< the vector [integrationError measuredAcc measuredOmega] in R^(9X9)

    Eigen::Matrix<double,9,9> preintMeasCov_; ///< COVARIANCE OF: [PreintPOSITION PreintVELOCITY PreintROTATION]
    ///< (first-order propagation from *measurementCovariance*).

  public:

    /**
     *  Default constructor, initializes the class with no measurements
     *  @param bias Current estimate of acceleration and rotation rate biases
     *  @param measuredAccCovariance      Covariance matrix of measuredAcc
     *  @param measuredOmegaCovariance    Covariance matrix of measured Angular Rate
     *  @param integrationErrorCovariance Covariance matrix of integration errors (velocity -> position)
     *  @param use2ndOrderIntegration     Controls the order of integration
     *  (if false: p(t+1) = p(t) + v(t) deltaT ; if true: p(t+1) = p(t) + v(t) deltaT + 0.5 * acc(t) deltaT^2)
     */
    PreintegratedMeasurements(const imuBias::ConstantBias& bias,
        const Matrix3& measuredAccCovariance, const Matrix3& measuredOmegaCovariance,
        const Matrix3& integrationErrorCovariance, const bool use2ndOrderIntegration = false);

    /// print
    void print(const std::string& s = "Preintegrated Measurements:") const;

    /// equals
    bool equals(const PreintegratedMeasurements& expected, double tol=1e-9) const;

    /// Re-initialize PreintegratedMeasurements
    void resetIntegration();

    /**
     * Add a single IMU measurement to the preintegration.
     * @param measuredAcc Measured acceleration (in body frame, as given by the sensor)
     * @param measuredOmega Measured angular velocity (as given by the sensor)
     * @param deltaT Time interval between two consecutive IMU measurements
     * @param body_P_sensor Optional sensor frame (pose of the IMU in the body frame)
     */
    void integrateMeasurement(const Vector3& measuredAcc, const Vector3& measuredOmega, double deltaT,
        boost::optional<const Pose3&> body_P_sensor = boost::none);

    /// methods to access class variables
    Matrix measurementCovariance() const {return measurementCovariance_;}
    Matrix preintMeasCov() const { return preintMeasCov_;}

  private:

    /// Serialization function
    friend class boost::serialization::access;
    template<class ARCHIVE>
    void serialize(ARCHIVE & ar, const unsigned int version) {
      ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(PreintegrationBase);
      ar & BOOST_SERIALIZATION_NVP(measurementCovariance_);
      ar & BOOST_SERIALIZATION_NVP(preintMeasCov_);
    }
  };

  private:

  typedef ImuFactor This;
  typedef NoiseModelFactor5<Pose3,Vector3,Pose3,Vector3,imuBias::ConstantBias> Base;

  PreintegratedMeasurements preintegratedMeasurements_;
  Vector3 gravity_;
  Vector3 omegaCoriolis_;
  boost::optional<Pose3> body_P_sensor_;        ///< The pose of the sensor in the body frame

  bool use2ndOrderCoriolis_; ///< Controls whether higher order terms are included when calculating the Coriolis Effect

  public:

  /** Shorthand for a smart pointer to a factor */
#if !defined(_MSC_VER) && __GNUC__ == 4 && __GNUC_MINOR__ > 5
  typedef typename boost::shared_ptr<ImuFactor> shared_ptr;
#else
  typedef boost::shared_ptr<ImuFactor> shared_ptr;
#endif

  /** Default constructor - only use for serialization */
  ImuFactor();

  /**
   * Constructor
   * @param pose_i Previous pose key
   * @param vel_i  Previous velocity key
   * @param pose_j Current pose key
   * @param vel_j  Current velocity key
   * @param bias   Previous bias key
   * @param preintegratedMeasurements Preintegrated IMU measurements
   * @param gravity Gravity vector expressed in the global frame
   * @param omegaCoriolis Rotation rate of the global frame w.r.t. an inertial frame
   * @param body_P_sensor Optional pose of the sensor frame in the body frame
   * @param use2ndOrderCoriolis When true, the second-order term is used in the calculation of the Coriolis Effect
   */
  ImuFactor(Key pose_i, Key vel_i, Key pose_j, Key vel_j, Key bias,
      const PreintegratedMeasurements& preintegratedMeasurements,
      const Vector3& gravity, const Vector3& omegaCoriolis,
      boost::optional<const Pose3&> body_P_sensor = boost::none, const bool use2ndOrderCoriolis = false);

  virtual ~ImuFactor() {}

  /// @return a deep copy of this factor
  virtual gtsam::NonlinearFactor::shared_ptr clone() const;

  /** implement functions needed for Testable */

  /// print
  virtual void print(const std::string& s, const KeyFormatter& keyFormatter = DefaultKeyFormatter) const;

  /// equals
  virtual bool equals(const NonlinearFactor& expected, double tol=1e-9) const;

  /** Access the preintegrated measurements. */

  const PreintegratedMeasurements& preintegratedMeasurements() const {
    return preintegratedMeasurements_; }

  const Vector3& gravity() const { return gravity_; }

  const Vector3& omegaCoriolis() const { return omegaCoriolis_; }

  /** implement functions needed to derive from Factor */

  /// vector of errors
  Vector evaluateError(const Pose3& pose_i, const Vector3& vel_i, const Pose3& pose_j, const Vector3& vel_j,
      const imuBias::ConstantBias& bias,
      boost::optional<Matrix&> H1 = boost::none,
      boost::optional<Matrix&> H2 = boost::none,
      boost::optional<Matrix&> H3 = boost::none,
      boost::optional<Matrix&> H4 = boost::none,
      boost::optional<Matrix&> H5 = boost::none) const;

  /// predicted states from IMU
  static PoseVelocity Predict(const Pose3& pose_i, const Vector3& vel_i,
      const imuBias::ConstantBias& bias, const PreintegratedMeasurements preintegratedMeasurements,
      const Vector3& gravity, const Vector3& omegaCoriolis, const bool use2ndOrderCoriolis = false);

  private:

  /** Serialization function */
  friend class boost::serialization::access;
  template<class ARCHIVE>
  void serialize(ARCHIVE & ar, const unsigned int version) {
    ar & boost::serialization::make_nvp("NoiseModelFactor5",
        boost::serialization::base_object<Base>(*this));
    ar & BOOST_SERIALIZATION_NVP(preintegratedMeasurements_);
    ar & BOOST_SERIALIZATION_NVP(gravity_);
    ar & BOOST_SERIALIZATION_NVP(omegaCoriolis_);
    ar & BOOST_SERIALIZATION_NVP(body_P_sensor_);
  }
}; // class ImuFactor

typedef ImuFactor::PreintegratedMeasurements ImuFactorPreintegratedMeasurements;

} /// namespace gtsam
