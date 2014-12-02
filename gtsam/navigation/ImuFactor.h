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
#include <gtsam/navigation/ImuBias.h>
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

class ImuFactor: public NoiseModelFactor5<Pose3,Vector3,Pose3,Vector3,imuBias::ConstantBias> {
public:

  /**
   * PreintegratedMeasurements accumulates (integrates) the IMU measurements
   * (rotation rates and accelerations) and the corresponding covariance matrix.
   * The measurements are then used to build the Preintegrated IMU factor (ImuFactor).
   * Can be built incrementally so as to avoid costly integration at time of
   * factor construction.
   */
  class PreintegratedMeasurements {

    friend class ImuFactor;

  protected:
    imuBias::ConstantBias biasHat_; ///< Acceleration and angular rate bias values used during preintegration
    Eigen::Matrix<double,9,9> measurementCovariance_; ///< (continuous-time uncertainty) "Covariance" of the vector [integrationError measuredAcc measuredOmega] in R^(9X9)

    Vector3 deltaPij_; ///< Preintegrated relative position (does not take into account velocity at time i, see deltap+, in [2]) (in frame i)
    Vector3 deltaVij_; ///< Preintegrated relative velocity (in global frame)
    Rot3 deltaRij_;    ///< Preintegrated relative orientation (in frame i)
    double deltaTij_;  ///< Time interval from i to j

    Matrix3 delPdelBiasAcc_;   ///< Jacobian of preintegrated position w.r.t. acceleration bias
    Matrix3 delPdelBiasOmega_; ///< Jacobian of preintegrated position w.r.t. angular rate bias
    Matrix3 delVdelBiasAcc_;   ///< Jacobian of preintegrated velocity w.r.t. acceleration bias
    Matrix3 delVdelBiasOmega_; ///< Jacobian of preintegrated velocity w.r.t. angular rate bias
    Matrix3 delRdelBiasOmega_; ///< Jacobian of preintegrated rotation w.r.t. angular rate bias

    Eigen::Matrix<double,9,9> PreintMeasCov_; ///< Covariance matrix of the preintegrated measurements (first-order propagation from *measurementCovariance*)

    bool use2ndOrderIntegration_; ///< Controls the order of integration

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

    /// methods to access class variables
    Matrix measurementCovariance() const {return measurementCovariance_;}
    Matrix deltaRij() const {return deltaRij_.matrix();}
    double deltaTij() const{return deltaTij_;}
    Vector deltaPij() const {return deltaPij_;}
    Vector deltaVij() const {return deltaVij_;}
    Vector biasHat() const { return biasHat_.vector();}
    Matrix delPdelBiasAcc() const { return delPdelBiasAcc_;}
    Matrix delPdelBiasOmega() const { return delPdelBiasOmega_;}
    Matrix delVdelBiasAcc() const { return delVdelBiasAcc_;}
    Matrix delVdelBiasOmega() const { return delVdelBiasOmega_;}
    Matrix delRdelBiasOmega() const{ return delRdelBiasOmega_;}
    Matrix preintMeasCov() const { return PreintMeasCov_;}

    /// Re-initialize PreintegratedMeasurements
    void resetIntegration();

    /**
     * Add a single IMU measurement to the preintegration.
     * @param measuredAcc Measured acceleration (in body frame)
     * @param measuredOmega Measured angular velocity
     * @param deltaT Time interval between two consecutive IMU measurements
     * @param body_P_sensor Optional sensor frame (pose of the IMU in the body frame)
     */
    void integrateMeasurement(const Vector3& measuredAcc, const Vector3& measuredOmega, double deltaT,
        boost::optional<const Pose3&> body_P_sensor = boost::none);

    /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
    // This function is only used for test purposes (compare numerical derivatives wrt analytic ones)
    static inline Vector PreIntegrateIMUObservations_delta_vel(const Vector& msr_gyro_t, const Vector& msr_acc_t, const double msr_dt,
        const Vector3& delta_angles, const Vector& delta_vel_in_t0){
      // Note: all delta terms refer to an IMU\sensor system at t0
      Vector body_t_a_body = msr_acc_t;
      Rot3 R_t_to_t0 = Rot3::Expmap(delta_angles);
      return delta_vel_in_t0 + R_t_to_t0.matrix() * body_t_a_body * msr_dt;
    }

    // This function is only used for test purposes (compare numerical derivatives wrt analytic ones)
    static inline Vector PreIntegrateIMUObservations_delta_angles(const Vector& msr_gyro_t, const double msr_dt,
        const Vector3& delta_angles){
      // Note: all delta terms refer to an IMU\sensor system at t0
      // Calculate the corrected measurements using the Bias object
      Vector body_t_omega_body= msr_gyro_t;
      Rot3 R_t_to_t0 = Rot3::Expmap(delta_angles);
      R_t_to_t0    = R_t_to_t0 * Rot3::Expmap( body_t_omega_body*msr_dt );
      return Rot3::Logmap(R_t_to_t0);
    }
    /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

    private:
      /** Serialization function */
      friend class boost::serialization::access;
      template<class ARCHIVE>
      void serialize(ARCHIVE & ar, const unsigned int version) {
        ar & BOOST_SERIALIZATION_NVP(biasHat_);
        ar & BOOST_SERIALIZATION_NVP(measurementCovariance_);
        ar & BOOST_SERIALIZATION_NVP(deltaPij_);
        ar & BOOST_SERIALIZATION_NVP(deltaVij_);
        ar & BOOST_SERIALIZATION_NVP(deltaRij_);
        ar & BOOST_SERIALIZATION_NVP(deltaTij_);
        ar & BOOST_SERIALIZATION_NVP(delPdelBiasAcc_);
        ar & BOOST_SERIALIZATION_NVP(delPdelBiasOmega_);
        ar & BOOST_SERIALIZATION_NVP(delVdelBiasAcc_);
        ar & BOOST_SERIALIZATION_NVP(delVdelBiasOmega_);
        ar & BOOST_SERIALIZATION_NVP(delRdelBiasOmega_);
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

    // predicted states from IMU
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
