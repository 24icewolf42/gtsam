/**
 * GTSAM Wrap Module Definition
 *
 * These are the current classes available through the matlab toolbox interface,
 * add more functions/classes as they are available.
 *
 * Requirements:
 *   Classes must start with an uppercase letter
 *   	 - Can wrap a typedef
 *   Only one Method/Constructor per line, though methods/constructors can extend across multiple lines
 *   Methods can return
 *     - Eigen types:       Matrix, Vector
 *     - C/C++ basic types: string, bool, size_t, size_t, double, char, unsigned char
 *     - void
 *     - Any class with which be copied with boost::make_shared()
 *     - boost::shared_ptr of any object type
 *   Constructors
 *     - Overloads are supported
 *     - A class with no constructors can be returned from other functions but not allocated directly in MATLAB
 *   Methods
 *     - Constness has no effect
 *     - Specify by-value (not reference) return types, even if C++ method returns reference
 *     - Must start with a lowercase letter
 *     - Overloads are supported
 *   Static methods
 *     - Must start with a letter (upper or lowercase) and use the "static" keyword
 *     - The first letter will be made uppercase in the generated MATLAB interface
 *     - Overloads are supported
 *   Arguments to functions any of
 *   	 - Eigen types:       Matrix, Vector
 *   	 - Eigen types and classes as an optionally const reference
 *     - C/C++ basic types: string, bool, size_t, size_t, double, char, unsigned char
 *     - Any class with which be copied with boost::make_shared() (except Eigen)
 *     - boost::shared_ptr of any object type (except Eigen)
 *   Comments can use either C++ or C style, with multiple lines
 *   Namespace definitions
 *     - Names of namespaces must start with a lowercase letter
 *   	 - start a namespace with "namespace {"
 *   	 - end a namespace with exactly "}"
 *   	 - Namespaces can be nested
 *   Namespace usage
 *   	 - Namespaces can be specified for classes in arguments and return values
 *   	 - In each case, the namespace must be fully specified, e.g., "namespace1::namespace2::ClassName"
 *   Includes in C++ wrappers
 *     - All includes will be collected and added in a single file
 *     - All namespaces must have angle brackets: <path>
 *     - No default includes will be added
 *   Global/Namespace functions
 *     - Functions specified outside of a class are global
 *     - Can be overloaded with different arguments
 *     - Can have multiple functions of the same name in different namespaces
 *   Using classes defined in other modules
 *     - If you are using a class 'OtherClass' not wrapped in this definition file, add "class OtherClass;" to avoid a dependency error
 *   Virtual inheritance
 *     - Specify fully-qualified base classes, i.e. "virtual class Derived : module::Base {"
 *     - Mark with 'virtual' keyword, e.g. "virtual class Base {", and also "virtual class Derived : module::Base {"
 *     - Forward declarations must also be marked virtual, e.g. "virtual class module::Base;" and
 *       also "virtual class module::Derived;"
 *     - Pure virtual (abstract) classes should list no constructors in this interface file
 *     - Virtual classes must have a clone() function in C++ (though it does not have to be included
 *       in the MATLAB interface).  clone() will be called whenever an object copy is needed, instead
 *       of using the copy constructor (which is used for non-virtual objects).
 *   Templates
 *     - Basic templates are supported either with an explicit list of types to instantiate,
 *       e.g. template<T = {gtsam::Pose2, gtsam::Rot2, gtsam::Point3}> class Class1 { ... };
 *       or with typedefs, e.g.
 *       template<T, U> class Class2 { ... };
 *       typedef Class2<Type1, Type2> MyInstantiatedClass;
 *     - In the class definition, appearances of the template argument(s) will be replaced with their
 *       instantiated types, e.g. 'void setValue(const T& value);'.
 *     - To refer to the instantiation of the template class itself, use 'This', i.e. 'static This Create();'
 *     - To create new instantiations in other modules, you must copy-and-paste the whole class definition
 *       into the new module, but use only your new instantiation types.
 *     - When forward-declaring template instantiations, use the generated/typedefed name, e.g.
 *       class gtsam::Class1Pose2;
 *       class gtsam::MyInstantiatedClass;
 */

/**
 * Status:
 *  - TODO: default values for arguments
 *  - TODO: Handle gtsam::Rot3M conversions to quaternions
 */

namespace gtsam {

//*************************************************************************
// base
//*************************************************************************

/** gtsam namespace functions */
bool linear_independent(Matrix A, Matrix B, double tol);

virtual class Value {
	// No constructors because this is an abstract class

	// Testable
	void print(string s) const;

	// Manifold
	size_t dim() const;
};

#include <gtsam/base/LieVector.h>
virtual class LieVector : gtsam::Value {
	// Standard constructors
	LieVector();
	LieVector(Vector v);

	// Standard interface
	Vector vector() const;

	// Testable
	void print(string s) const;
	bool equals(const gtsam::LieVector& expected, double tol) const;

	// Group
	static gtsam::LieVector identity();
	gtsam::LieVector inverse() const;
	gtsam::LieVector compose(const gtsam::LieVector& p) const;
	gtsam::LieVector between(const gtsam::LieVector& l2) const;

	// Manifold
	size_t dim() const;
	gtsam::LieVector retract(Vector v) const;
	Vector localCoordinates(const gtsam::LieVector& t2) const;

	// Lie group
	static gtsam::LieVector Expmap(Vector v);
	static Vector Logmap(const gtsam::LieVector& p);
};

#include <gtsam/base/LieMatrix.h>
virtual class LieMatrix : gtsam::Value {
	// Standard constructors
	LieMatrix();
	LieMatrix(Matrix v);

	// Standard interface
	Vector matrix() const;

	// Testable
	void print(string s) const;
	bool equals(const gtsam::LieMatrix& expected, double tol) const;

	// Group
	static gtsam::LieMatrix identity();
	gtsam::LieMatrix inverse() const;
	gtsam::LieMatrix compose(const gtsam::LieMatrix& p) const;
	gtsam::LieMatrix between(const gtsam::LieMatrix& l2) const;

	// Manifold
	size_t dim() const;
	gtsam::LieMatrix retract(Vector v) const;
	Vector localCoordinates(const gtsam::LieMatrix & t2) const;

	// Lie group
	static gtsam::LieMatrix Expmap(Vector v);
	static Vector Logmap(const gtsam::LieMatrix& p);
};

//*************************************************************************
// geometry
//*************************************************************************

virtual class Point2 : gtsam::Value {
  // Standard Constructors
	Point2();
	Point2(double x, double y);
	Point2(Vector v);

  // Testable
  void print(string s) const;
  bool equals(const gtsam::Point2& pose, double tol) const;

  // Group
  static gtsam::Point2 identity();
  gtsam::Point2 inverse() const;
  gtsam::Point2 compose(const gtsam::Point2& p2) const;
  gtsam::Point2 between(const gtsam::Point2& p2) const;

  // Manifold
  static size_t Dim();
  size_t dim() const;
  gtsam::Point2 retract(Vector v) const;
  Vector localCoordinates(const gtsam::Point2& p) const;

  // Lie Group
  static gtsam::Point2 Expmap(Vector v);
	static Vector Logmap(const gtsam::Point2& p);

  // Standard Interface
  double x() const;
	double y() const;
  Vector vector() const;
};

virtual class StereoPoint2 : gtsam::Value {
  // Standard Constructors
  StereoPoint2();
  StereoPoint2(double uL, double uR, double v);

  // Testable
  void print(string s) const;
  bool equals(const gtsam::StereoPoint2& point, double tol) const;

  // Group
  static gtsam::StereoPoint2 identity();
  gtsam::StereoPoint2 inverse() const;
  gtsam::StereoPoint2 compose(const gtsam::StereoPoint2& p2) const;
  gtsam::StereoPoint2 between(const gtsam::StereoPoint2& p2) const;

  // Manifold
  static size_t Dim();
  size_t dim() const;
  gtsam::StereoPoint2 retract(Vector v) const;
  Vector localCoordinates(const gtsam::StereoPoint2& p) const;

  // Lie Group
  static gtsam::StereoPoint2 Expmap(Vector v);
  static Vector Logmap(const gtsam::StereoPoint2& p);

  // Standard Interface
  Vector vector() const;
};

virtual class Point3 : gtsam::Value {
  // Standard Constructors
	Point3();
	Point3(double x, double y, double z);
	Point3(Vector v);

  // Testable
  void print(string s) const;
  bool equals(const gtsam::Point3& p, double tol) const;

  // Group
  static gtsam::Point3 identity();
  gtsam::Point3 inverse() const;
  gtsam::Point3 compose(const gtsam::Point3& p2) const;
  gtsam::Point3 between(const gtsam::Point3& p2) const;

  // Manifold
  static size_t Dim();
  size_t dim() const;
  gtsam::Point3 retract(Vector v) const;
  Vector localCoordinates(const gtsam::Point3& p) const;

  // Lie Group
  static gtsam::Point3 Expmap(Vector v);
	static Vector Logmap(const gtsam::Point3& p);

  // Standard Interface
	Vector vector() const;
	double x() const;
	double y() const;
	double z() const;
};

virtual class Rot2 : gtsam::Value {
  // Standard Constructors and Named Constructors
	Rot2();
	Rot2(double theta);
  static gtsam::Rot2 fromAngle(double theta);
  static gtsam::Rot2 fromDegrees(double theta);
  static gtsam::Rot2 fromCosSin(double c, double s);

  // Testable
  void print(string s) const;
  bool equals(const gtsam::Rot2& rot, double tol) const;

  // Group
  static gtsam::Rot2 identity();
  gtsam::Rot2 inverse();
  gtsam::Rot2 compose(const gtsam::Rot2& p2) const;
  gtsam::Rot2 between(const gtsam::Rot2& p2) const;

  // Manifold
  static size_t Dim();
  size_t dim() const;
  gtsam::Rot2 retract(Vector v) const;
  Vector localCoordinates(const gtsam::Rot2& p) const;

  // Lie Group
  static gtsam::Rot2 Expmap(Vector v);
	static Vector Logmap(const gtsam::Rot2& p);

  // Group Action on Point2
  gtsam::Point2 rotate(const gtsam::Point2& point) const;
  gtsam::Point2 unrotate(const gtsam::Point2& point) const;

  // Standard Interface
	static gtsam::Rot2 relativeBearing(const gtsam::Point2& d); // Ignoring derivative
	static gtsam::Rot2 atan2(double y, double x);
	double theta() const;
	double degrees() const;
	double c() const;
	double s() const;
  Matrix matrix() const;
};

virtual class Rot3 : gtsam::Value {
  // Standard Constructors and Named Constructors
	Rot3();
	Rot3(Matrix R);
	static gtsam::Rot3 Rx(double t);
	static gtsam::Rot3 Ry(double t);
	static gtsam::Rot3 Rz(double t);
  static gtsam::Rot3 RzRyRx(double x, double y, double z); // FIXME: overloaded functions don't work yet
	static gtsam::Rot3 RzRyRx(Vector xyz);
	static gtsam::Rot3 yaw(double t); // positive yaw is to right (as in aircraft heading)
	static gtsam::Rot3 pitch(double t); // positive pitch is up (increasing aircraft altitude)
	static gtsam::Rot3 roll(double t); // positive roll is to right (increasing yaw in aircraft)
	static gtsam::Rot3 ypr(double y, double p, double r);
	static gtsam::Rot3 quaternion(double w, double x, double y, double z);
	static gtsam::Rot3 rodriguez(Vector v);

  // Testable
	void print(string s) const;
	bool equals(const gtsam::Rot3& rot, double tol) const;

  // Group
	static gtsam::Rot3 identity();
  gtsam::Rot3 inverse() const;
	gtsam::Rot3 compose(const gtsam::Rot3& p2) const;
	gtsam::Rot3 between(const gtsam::Rot3& p2) const;

  // Manifold
  static size_t Dim();
  size_t dim() const;
  gtsam::Rot3 retractCayley(Vector v) const; // FIXME, does not exist in both Matrix and Quaternion options
  gtsam::Rot3 retract(Vector v) const;
  Vector localCoordinates(const gtsam::Rot3& p) const;

  // Group Action on Point3
	gtsam::Point3 rotate(const gtsam::Point3& p) const;
	gtsam::Point3 unrotate(const gtsam::Point3& p) const;

  // Standard Interface
	static gtsam::Rot3 Expmap(Vector v);
	static Vector Logmap(const gtsam::Rot3& p);
	Matrix matrix() const;
	Matrix transpose() const;
	gtsam::Point3 column(size_t index) const;
	Vector xyz() const;
	Vector ypr() const;
	Vector rpy() const;
	double roll() const;
	double pitch() const;
	double yaw() const;
//  Vector toQuaternion() const;  // FIXME: Can't cast to Vector properly
};

virtual class Pose2 : gtsam::Value {
  // Standard Constructor
	Pose2();
	Pose2(double x, double y, double theta);
	Pose2(double theta, const gtsam::Point2& t);
	Pose2(const gtsam::Rot2& r, const gtsam::Point2& t);
	Pose2(Vector v);

  // Testable
  void print(string s) const;
  bool equals(const gtsam::Pose2& pose, double tol) const;

  // Group
  static gtsam::Pose2 identity();
  gtsam::Pose2 inverse() const;
  gtsam::Pose2 compose(const gtsam::Pose2& p2) const;
  gtsam::Pose2 between(const gtsam::Pose2& p2) const;

  // Manifold
  static size_t Dim();
  size_t dim() const;
  gtsam::Pose2 retract(Vector v) const;
  Vector localCoordinates(const gtsam::Pose2& p) const;

  // Lie Group
	static gtsam::Pose2 Expmap(Vector v);
	static Vector Logmap(const gtsam::Pose2& p);
  Matrix adjointMap() const;
  Vector adjoint(const Vector& xi) const;
  static Matrix wedge(double vx, double vy, double w);

  // Group Actions on Point2
  gtsam::Point2 transform_from(const gtsam::Point2& p) const;
  gtsam::Point2 transform_to(const gtsam::Point2& p) const;

  // Standard Interface
	double x() const;
	double y() const;
	double theta() const;
	gtsam::Rot2 bearing(const gtsam::Point2& point) const;
	double range(const gtsam::Point2& point) const;
	gtsam::Point2 translation() const;
	gtsam::Rot2 rotation() const;
  Matrix matrix() const;
};

virtual class Pose3 : gtsam::Value {
	// Standard Constructors
	Pose3();
	Pose3(const gtsam::Pose3& pose);
	Pose3(const gtsam::Rot3& r, const gtsam::Point3& t);
	Pose3(const gtsam::Pose2& pose2); // FIXME: shadows Pose3(Pose3 pose)
	Pose3(Matrix t);

	// Testable
	void print(string s) const;
	bool equals(const gtsam::Pose3& pose, double tol) const;

	// Group
	static gtsam::Pose3 identity();
	gtsam::Pose3 inverse() const;
	gtsam::Pose3 compose(const gtsam::Pose3& p2) const;
	gtsam::Pose3 between(const gtsam::Pose3& p2) const;

	// Manifold
	static size_t Dim();
	size_t dim() const;
	gtsam::Pose3 retract(Vector v) const;
	gtsam::Pose3 retractFirstOrder(Vector v) const;
	Vector localCoordinates(const gtsam::Pose3& T2) const;

	// Lie Group
	static gtsam::Pose3 Expmap(Vector v);
	static Vector Logmap(const gtsam::Pose3& p);
	Matrix adjointMap() const;
	Vector adjoint(const Vector& xi) const;
	static Matrix wedge(double wx, double wy, double wz, double vx, double vy, double vz);

	// Group Action on Point3
	gtsam::Point3 transform_from(const gtsam::Point3& p) const;
	gtsam::Point3 transform_to(const gtsam::Point3& p) const;

	// Standard Interface
	gtsam::Rot3 rotation() const;
	gtsam::Point3 translation() const;
	double x() const;
	double y() const;
	double z() const;
	Matrix matrix() const;
	gtsam::Pose3 transform_to(const gtsam::Pose3& pose) const; // FIXME: shadows other transform_to()
	double range(const gtsam::Point3& point);
	double range(const gtsam::Pose3& pose);
};

virtual class Cal3_S2 : gtsam::Value {
  // Standard Constructors
  Cal3_S2();
  Cal3_S2(double fx, double fy, double s, double u0, double v0);
	Cal3_S2(Vector v);

  // Testable
  void print(string s) const;
  bool equals(const gtsam::Cal3_S2& rhs, double tol) const;

  // Manifold
  static size_t Dim();
  size_t dim() const;
  gtsam::Cal3_S2 retract(Vector v) const;
  Vector localCoordinates(const gtsam::Cal3_S2& c) const;

  // Action on Point2
  gtsam::Point2 calibrate(const gtsam::Point2& p) const;
  gtsam::Point2 uncalibrate(const gtsam::Point2& p) const;

  // Standard Interface
  double fx() const;
  double fy() const;
  double skew() const;
  double px() const;
  double py() const;
  gtsam::Point2 principalPoint() const;
  Vector vector() const;
  Matrix matrix() const;
  Matrix matrix_inverse() const;
};

#include <gtsam/geometry/Cal3DS2.h>
virtual class Cal3DS2 : gtsam::Value {
	// Standard Constructors
	Cal3DS2();
	Cal3DS2(double fx, double fy, double s, double u0, double v0, double k1, double k2, double k3, double k4);
	Cal3DS2(Vector v);

	// Testable
	void print(string s) const;
	bool equals(const gtsam::Cal3DS2& rhs, double tol) const;

	// Manifold
	static size_t Dim();
	size_t dim() const;
	gtsam::Cal3DS2 retract(Vector v) const;
	Vector localCoordinates(const gtsam::Cal3DS2& c) const;

	// Action on Point2
	gtsam::Point2 uncalibrate(const gtsam::Point2& p) const;
	// TODO: D2d functions that start with an uppercase letter

	// Standard Interface
	double fx() const;
	double fy() const;
	double skew() const;
	double px() const;
	double py() const;
	Vector vector() const;
	Vector k() const;
	//Matrix K() const; //FIXME: Uppercase
};

class Cal3_S2Stereo {
  // Standard Constructors
  Cal3_S2Stereo();
  Cal3_S2Stereo(double fx, double fy, double s, double u0, double v0, double b);

  // Testable
  void print(string s) const;
  bool equals(const gtsam::Cal3_S2Stereo& pose, double tol) const;

  // Standard Interface
  double fx() const;
  double fy() const;
  double skew() const;
  double px() const;
  double py() const;
  gtsam::Point2 principalPoint() const;
  double baseline() const;
};

virtual class CalibratedCamera : gtsam::Value {
  // Standard Constructors and Named Constructors
	CalibratedCamera();
	CalibratedCamera(const gtsam::Pose3& pose);
	CalibratedCamera(const Vector& v);
  static gtsam::CalibratedCamera Level(const gtsam::Pose2& pose2, double height);

  // Testable
	void print(string s) const;
	bool equals(const gtsam::CalibratedCamera& camera, double tol) const;

  // Manifold
  static size_t Dim();
  size_t dim() const;
  gtsam::CalibratedCamera retract(const Vector& d) const;
  Vector localCoordinates(const gtsam::CalibratedCamera& T2) const;

  // Group
  gtsam::CalibratedCamera compose(const gtsam::CalibratedCamera& c) const;
  gtsam::CalibratedCamera inverse() const;

  // Action on Point3
  gtsam::Point2 project(const gtsam::Point3& point) const;
  static gtsam::Point2 project_to_camera(const gtsam::Point3& cameraPoint);

  // Standard Interface
	gtsam::Pose3 pose() const;
  double range(const gtsam::Point3& p) const; // TODO: Other overloaded range methods
};

virtual class SimpleCamera : gtsam::Value {
  // Standard Constructors and Named Constructors
	SimpleCamera();
  SimpleCamera(const gtsam::Pose3& pose);
  SimpleCamera(const gtsam::Pose3& pose, const gtsam::Cal3_S2& K);
  static gtsam::SimpleCamera Level(const gtsam::Cal3_S2& K,
      const gtsam::Pose2& pose, double height);
  static gtsam::SimpleCamera Level(const gtsam::Pose2& pose, double height); // FIXME overload
  static gtsam::SimpleCamera Lookat(const gtsam::Point3& eye,
      const gtsam::Point3& target, const gtsam::Point3& upVector,
      const gtsam::Cal3_S2& K);

  // Testable
	void print(string s) const;
	bool equals(const gtsam::SimpleCamera& camera, double tol) const;

  // Standard Interface
  gtsam::Pose3 pose() const;
  gtsam::Cal3_S2 calibration();

	// Manifold
  gtsam::SimpleCamera retract(const Vector& d) const;
  Vector localCoordinates(const gtsam::SimpleCamera& T2) const;
  size_t dim() const;
  static size_t Dim();

  // Transformations and measurement functions
  static gtsam::Point2 project_to_camera(const gtsam::Point3& cameraPoint);
  pair<gtsam::Point2,bool> projectSafe(const gtsam::Point3& pw) const;
	gtsam::Point2 project(const gtsam::Point3& point);
	gtsam::Point3 backproject(const gtsam::Point2& p, double depth) const;
  double range(const gtsam::Point3& point);
  double range(const gtsam::Pose3& point); // FIXME, overload
};

//*************************************************************************
// inference
//*************************************************************************
class Permutation {
	// Standard Constructors and Named Constructors
	Permutation();
	Permutation(size_t nVars);
	static gtsam::Permutation Identity(size_t nVars);
	// FIXME: Cannot currently wrap std::vector
	//static gtsam::Permutation PullToFront(const vector<size_t>& toFront, size_t size, bool filterDuplicates);
	//static gtsam::Permutation PushToBack(const vector<size_t>& toBack, size_t size, bool filterDuplicates = false);

	// Testable
	void print(string s) const;
	bool equals(const gtsam::Permutation& rhs, double tol) const;

	// Standard interface
	size_t at(size_t variable) const;
	size_t size() const;
	bool empty() const;
	void resize(size_t newSize);
	gtsam::Permutation* permute(const gtsam::Permutation& permutation) const;
	gtsam::Permutation* inverse() const;
};

class IndexFactor {
  // Standard Constructors and Named Constructors
  IndexFactor();
  IndexFactor(size_t j);
  IndexFactor(size_t j1, size_t j2);
  IndexFactor(size_t j1, size_t j2, size_t j3);
  IndexFactor(size_t j1, size_t j2, size_t j3, size_t j4);
  IndexFactor(size_t j1, size_t j2, size_t j3, size_t j4, size_t j5);
  IndexFactor(size_t j1, size_t j2, size_t j3, size_t j4, size_t j5, size_t j6);
  // FIXME: Must wrap std::set<Index> for this to work
  //IndexFactor(const std::set<Index>& js);

  // From Factor
  size_t size() const;
  void print(string s) const;
  bool equals(const gtsam::IndexFactor& other, double tol) const;
  // FIXME: Need to wrap std::vector<KeyType>
  //std::vector<KeyType>& keys();
};

class IndexConditional {
  // Standard Constructors and Named Constructors
  IndexConditional();
  IndexConditional(size_t key);
  IndexConditional(size_t key, size_t parent);
  IndexConditional(size_t key, size_t parent1, size_t parent2);
  IndexConditional(size_t key, size_t parent1, size_t parent2, size_t parent3);
  // FIXME: Must wrap std::vector<KeyType> for this to work
  //IndexFactor(size_t key, const std::vector<KeyType>& parents);
  //IndexConditional(const std::vector<Index>& keys, size_t nrFrontals);
  //template<class KEYS> static shared_ptr FromKeys(const KEYS& keys, size_t nrFrontals);

  // Testable
  void print(string s) const;
  bool equals(const gtsam::IndexConditional& other, double tol) const;

  // Standard interface
  size_t nrFrontals() const;
  size_t nrParents() const;
  gtsam::IndexFactor* toFactor() const;
};

#include <gtsam/inference/SymbolicFactorGraph.h>
class SymbolicBayesNet {
  // Standard Constructors and Named Constructors
  SymbolicBayesNet();
  SymbolicBayesNet(const gtsam::SymbolicBayesNet& bn);
  SymbolicBayesNet(const gtsam::IndexConditional* conditional);

  // Testable
  void print(string s) const;
  bool equals(const gtsam::SymbolicBayesNet& other, double tol) const;

  // Standard interface
  size_t size() const;
  void push_back(const gtsam::IndexConditional* conditional);
  // FIXME: cannot overload functions
  //void push_back(const SymbolicBayesNet bn);
  void push_front(const gtsam::IndexConditional* conditional);
  // FIXME: cannot overload functions
  //void push_front(const SymbolicBayesNet bn);
  void pop_front();
  void permuteWithInverse(const gtsam::Permutation& inversePermutation);
  bool permuteSeparatorWithInverse(const gtsam::Permutation& inversePermutation);
};

#include <gtsam/inference/SymbolicFactorGraph.h>
class SymbolicBayesTree {
  // Standard Constructors and Named Constructors
  SymbolicBayesTree();
  SymbolicBayesTree(const gtsam::SymbolicBayesNet& bn);
  SymbolicBayesTree(const gtsam::SymbolicBayesTree& other);
  // FIXME: wrap needs to understand std::list
  //SymbolicBayesTree(const gtsam::SymbolicBayesNet& bayesNet, std::list<gtsam::SymbolicBayesTree> subtrees);

  // Testable
  void print(string s) const;
  bool equals(const gtsam::SymbolicBayesTree& other, double tol) const;

  // Standard interface
  size_t size() const;
  void saveGraph(string s) const;
  void clear();
  // TODO: There are many other BayesTree member functions which might be of use
};

class SymbolicFactorGraph {
  // Standard Constructors and Named Constructors
  SymbolicFactorGraph();
  SymbolicFactorGraph(const gtsam::SymbolicBayesNet& bayesNet);
  SymbolicFactorGraph(const gtsam::SymbolicBayesTree& bayesTree);

  // From FactorGraph
  void push_back(gtsam::IndexFactor* factor);
  void print(string s) const;
  bool equals(const gtsam::SymbolicFactorGraph& rhs, double tol) const;
  size_t size() const;

  // Standard interface
  // FIXME: Must wrap FastSet<Index> for this to work
  //FastSet<Index> keys() const;
};

#include <gtsam/inference/SymbolicSequentialSolver.h>
class SymbolicSequentialSolver {
  // Standard Constructors and Named Constructors
  SymbolicSequentialSolver(const gtsam::SymbolicFactorGraph& factorGraph);
  SymbolicSequentialSolver(const gtsam::SymbolicFactorGraph* factorGraph, const gtsam::VariableIndex* variableIndex);

  // Testable
  void print(string s) const;
  bool equals(const gtsam::SymbolicSequentialSolver& rhs, double tol) const;

  // Standard interface
  gtsam::SymbolicBayesNet* eliminate() const;
};

#include <gtsam/inference/SymbolicMultifrontalSolver.h>
class SymbolicMultifrontalSolver {
  // Standard Constructors and Named Constructors
  SymbolicMultifrontalSolver(const gtsam::SymbolicFactorGraph& factorGraph);
  SymbolicMultifrontalSolver(const gtsam::SymbolicFactorGraph* factorGraph, const gtsam::VariableIndex* variableIndex);

  // Testable
  void print(string s) const;
  bool equals(const gtsam::SymbolicMultifrontalSolver& rhs, double tol) const;

  // Standard interface
  gtsam::SymbolicBayesTree* eliminate() const;
};

#include <gtsam/inference/SymbolicFactorGraph.h>
class VariableIndex {
  // Standard Constructors and Named Constructors
  VariableIndex();
  // FIXME: Handle templates somehow
  //template<class FactorGraph> VariableIndex(const FactorGraph& factorGraph, size_t nVariables);
  //template<class FactorGraph> VariableIndex(const FactorGraph& factorGraph);
  VariableIndex(const gtsam::SymbolicFactorGraph& factorGraph);
  VariableIndex(const gtsam::SymbolicFactorGraph& factorGraph, size_t nVariables);
//  VariableIndex(const gtsam::GaussianFactorGraph& factorGraph);
//  VariableIndex(const gtsam::GaussianFactorGraph& factorGraph, size_t nVariables);
//  VariableIndex(const gtsam::NonlinearFactorGraph& factorGraph);
//  VariableIndex(const gtsam::NonlinearFactorGraph& factorGraph, size_t nVariables);
  VariableIndex(const gtsam::VariableIndex& other);

  // Testable
  bool equals(const gtsam::VariableIndex& other, double tol) const;
  void print(string s) const;

  // Standard interface
  size_t size() const;
  size_t nFactors() const;
  size_t nEntries() const;
  void permuteInPlace(const gtsam::Permutation& permutation);
};

//*************************************************************************
// linear
//*************************************************************************

namespace noiseModel {
#include <gtsam/linear/NoiseModel.h>
virtual class Base {
};

virtual class Gaussian : gtsam::noiseModel::Base {
	static gtsam::noiseModel::Gaussian* SqrtInformation(Matrix R);
	static gtsam::noiseModel::Gaussian* Covariance(Matrix R);
//	Matrix R() const;		// FIXME: cannot parse!!!
	void print(string s) const;
};

virtual class Diagonal : gtsam::noiseModel::Gaussian {
	static gtsam::noiseModel::Diagonal* Sigmas(Vector sigmas);
	static gtsam::noiseModel::Diagonal* Variances(Vector variances);
	static gtsam::noiseModel::Diagonal* Precisions(Vector precisions);
//	Matrix R() const;		// FIXME: cannot parse!!!
	void print(string s) const;
};

virtual class Isotropic : gtsam::noiseModel::Diagonal {
	static gtsam::noiseModel::Isotropic* Sigma(size_t dim, double sigma);
	static gtsam::noiseModel::Isotropic* Variance(size_t dim, double varianace);
	static gtsam::noiseModel::Isotropic* Precision(size_t dim, double precision);
	void print(string s) const;
};

virtual class Unit : gtsam::noiseModel::Isotropic {
	static gtsam::noiseModel::Unit* Create(size_t dim);
	void print(string s) const;
};
}///\namespace noiseModel

#include <gtsam/linear/Sampler.h>
class Sampler {
	Sampler(gtsam::noiseModel::Diagonal* model, int seed);
	Sampler(Vector sigmas, int seed);
	Sampler(int seed);

	size_t dim() const;
	Vector sigmas() const;
	gtsam::noiseModel::Diagonal* model() const;

	Vector sample();
	Vector sampleNewModel(gtsam::noiseModel::Diagonal* model);
};

class VectorValues {
	VectorValues();
	VectorValues(size_t nVars, size_t varDim);
	void print(string s) const;
	bool equals(const gtsam::VectorValues& expected, double tol) const;
	size_t size() const;
	void insert(size_t j, Vector value);
};

class GaussianConditional {
	GaussianConditional(size_t key, Vector d, Matrix R, Vector sigmas);
	GaussianConditional(size_t key, Vector d, Matrix R, size_t name1, Matrix S,
			Vector sigmas);
	GaussianConditional(size_t key, Vector d, Matrix R, size_t name1, Matrix S,
			size_t name2, Matrix T, Vector sigmas);
	void print(string s) const;
	bool equals(const gtsam::GaussianConditional &cg, double tol) const;
};

class GaussianDensity {
	GaussianDensity(size_t key, Vector d, Matrix R, Vector sigmas);
	void print(string s) const;
	Vector mean() const;
	Matrix information() const;
	Matrix covariance() const;
};

class GaussianBayesNet {
	GaussianBayesNet();
	void print(string s) const;
	bool equals(const gtsam::GaussianBayesNet& cbn, double tol) const;
	void push_back(gtsam::GaussianConditional* conditional);
	void push_front(gtsam::GaussianConditional* conditional);
};

virtual class GaussianFactor {
	void print(string s) const;
	bool equals(const gtsam::GaussianFactor& lf, double tol) const;
	double error(const gtsam::VectorValues& c) const;
	gtsam::GaussianFactor* negate() const;
	size_t size() const;
};

virtual class JacobianFactor : gtsam::GaussianFactor {
	JacobianFactor();
	JacobianFactor(Vector b_in);
	JacobianFactor(size_t i1, Matrix A1, Vector b,
			const gtsam::noiseModel::Diagonal* model);
	JacobianFactor(size_t i1, Matrix A1, size_t i2, Matrix A2, Vector b,
			const gtsam::noiseModel::Diagonal* model);
	JacobianFactor(size_t i1, Matrix A1, size_t i2, Matrix A2, size_t i3, Matrix A3,
			Vector b, const gtsam::noiseModel::Diagonal* model);
	JacobianFactor(const gtsam::GaussianFactor& factor);
	void print(string s) const;
	bool equals(const gtsam::GaussianFactor& lf, double tol) const;
	bool empty() const;
	size_t size() const;
	Vector getb() const;
	double error(const gtsam::VectorValues& c) const;
	gtsam::GaussianConditional* eliminateFirst();
	gtsam::GaussianFactor* negate() const;
};

virtual class HessianFactor : gtsam::GaussianFactor {
	HessianFactor(const gtsam::HessianFactor& gf);
	HessianFactor();
	HessianFactor(size_t j, Matrix G, Vector g, double f);
	HessianFactor(size_t j, Vector mu, Matrix Sigma);
	HessianFactor(size_t j1, size_t j2, Matrix G11, Matrix G12, Vector g1, Matrix G22,
			Vector g2, double f);
	HessianFactor(size_t j1, size_t j2, size_t j3, Matrix G11, Matrix G12, Matrix G13,
			Vector g1, Matrix G22, Matrix G23, Vector g2, Matrix G33, Vector g3,
			double f);
	HessianFactor(const gtsam::GaussianConditional& cg);
	HessianFactor(const gtsam::GaussianFactor& factor);
	size_t size() const;
	void print(string s) const;
	bool equals(const gtsam::GaussianFactor& lf, double tol) const;
	double error(const gtsam::VectorValues& c) const;
	gtsam::GaussianFactor* negate() const;
};

class GaussianFactorGraph {
	GaussianFactorGraph();
	GaussianFactorGraph(const gtsam::GaussianBayesNet& CBN);

	// From FactorGraph
	void print(string s) const;
	bool equals(const gtsam::GaussianFactorGraph& lfgraph, double tol) const;
	size_t size() const;
	gtsam::GaussianFactor* at(size_t idx) const;

	// Building the graph
	void push_back(gtsam::GaussianFactor* factor);
	void add(Vector b);
	void add(size_t key1, Matrix A1, Vector b, const gtsam::noiseModel::Diagonal* model);
	void add(size_t key1, Matrix A1, size_t key2, Matrix A2, Vector b,
			const gtsam::noiseModel::Diagonal* model);
	void add(size_t key1, Matrix A1, size_t key2, Matrix A2, size_t key3, Matrix A3,
			Vector b, const gtsam::noiseModel::Diagonal* model);

	// error and probability
	double error(const gtsam::VectorValues& c) const;
	double probPrime(const gtsam::VectorValues& c) const;

	// combining
	static gtsam::GaussianFactorGraph combine2(
			const gtsam::GaussianFactorGraph& lfg1,
			const gtsam::GaussianFactorGraph& lfg2);
	void combine(const gtsam::GaussianFactorGraph& lfg);

	// Conversion to matrices
	Matrix sparseJacobian_() const;
	Matrix denseJacobian() const;
	Matrix denseHessian() const;
};

class GaussianISAM {
  GaussianISAM();
  void saveGraph(string s) const;
  gtsam::GaussianFactor* marginalFactor(size_t j) const;
  gtsam::GaussianBayesNet* marginalBayesNet(size_t key) const;
  Matrix marginalCovariance(size_t key) const;
  gtsam::GaussianBayesNet* jointBayesNet(size_t key1, size_t key2) const;
};

#include <gtsam/linear/GaussianSequentialSolver.h>
class GaussianSequentialSolver {
	GaussianSequentialSolver(const gtsam::GaussianFactorGraph& graph,
			bool useQR);
	gtsam::GaussianBayesNet* eliminate() const;
	gtsam::VectorValues* optimize() const;
	gtsam::GaussianFactor* marginalFactor(size_t j) const;
	Matrix marginalCovariance(size_t j) const;
};

#include <gtsam/linear/KalmanFilter.h>
class KalmanFilter {
	KalmanFilter(size_t n);
	// gtsam::GaussianDensity* init(Vector x0, const gtsam::SharedDiagonal& P0);
	gtsam::GaussianDensity* init(Vector x0, Matrix P0);
	void print(string s) const;
	static size_t step(gtsam::GaussianDensity* p);
	gtsam::GaussianDensity* predict(gtsam::GaussianDensity* p, Matrix F,
			Matrix B, Vector u, const gtsam::noiseModel::Diagonal* modelQ);
	gtsam::GaussianDensity* predictQ(gtsam::GaussianDensity* p, Matrix F,
			Matrix B, Vector u, Matrix Q);
	gtsam::GaussianDensity* predict2(gtsam::GaussianDensity* p, Matrix A0,
			Matrix A1, Vector b, const gtsam::noiseModel::Diagonal* model);
	gtsam::GaussianDensity* update(gtsam::GaussianDensity* p, Matrix H,
			Vector z, const gtsam::noiseModel::Diagonal* model);
	gtsam::GaussianDensity* updateQ(gtsam::GaussianDensity* p, Matrix H,
			Vector z, Matrix Q);
};

//*************************************************************************
// nonlinear
//*************************************************************************

#include <gtsam/nonlinear/Symbol.h>
class Symbol {
	Symbol(char c, size_t j);
	Symbol(size_t k);
	void print(string s) const;
	size_t key() const;
	size_t index() const;
	char chr() const;
};

#include <gtsam/nonlinear/Ordering.h>
class Ordering {
  // Standard Constructors and Named Constructors
  Ordering();

	// Testable
	void print(string s) const;
	bool equals(const gtsam::Ordering& ord, double tol) const;

	// Standard interface
	size_t nVars() const;
  size_t size() const;
  size_t at(size_t key) const;
  bool exists(size_t key) const;
  void insert(size_t key, size_t order);
  void push_back(size_t key);
  void permuteWithInverse(const gtsam::Permutation& inversePermutation);
  gtsam::InvertedOrdering invert() const;
};

class InvertedOrdering {
	InvertedOrdering();

	// FIXME: add bracket operator overload

	bool empty() const;
	size_t size() const;
	bool count(size_t index) const; // Use as a boolean function with implicit cast

	void clear();
};

class NonlinearFactorGraph {
	NonlinearFactorGraph();
  void print(string s) const;
  size_t size() const;
	double error(const gtsam::Values& c) const;
	double probPrime(const gtsam::Values& c) const;
	gtsam::NonlinearFactor* at(size_t i) const;
	void add(const gtsam::NonlinearFactor* factor);
	gtsam::Ordering* orderingCOLAMD(const gtsam::Values& c) const;
	// Ordering* orderingCOLAMDConstrained(const gtsam::Values& c, const std::map<gtsam::Key,int>& constraints) const;
	gtsam::GaussianFactorGraph* linearize(const gtsam::Values& c, const gtsam::Ordering& ordering) const;
	gtsam::NonlinearFactorGraph clone() const;
};

virtual class NonlinearFactor {
	void print(string s) const;
	void equals(const gtsam::NonlinearFactor& other, double tol) const;
	gtsam::KeyVector keys() const;
	size_t size() const;
	size_t dim() const;
	double error(const gtsam::Values& c) const;
	bool active(const gtsam::Values& c) const;
	gtsam::GaussianFactor* linearize(const gtsam::Values& c, const gtsam::Ordering& ordering) const;
	gtsam::NonlinearFactor* clone() const;
	// gtsam::NonlinearFactor* rekey(const gtsam::KeyVector& newKeys) const; //FIXME: Conversion from KeyVector to std::vector does not happen
};

class Values {
	Values();
	size_t size() const;
	void print(string s) const;
	void insert(size_t j, const gtsam::Value& value);
	bool exists(size_t j) const;
	gtsam::Value at(size_t j) const;
	gtsam::KeyList keys() const;
};

// Actually a FastList<Key>
#include <gtsam/nonlinear/Key.h>
class KeyList {
	KeyList();
	KeyList(const gtsam::KeyList& other);

	// Note: no print function

	// common STL methods
	size_t size() const;
	bool empty() const;
	void clear();

	// structure specific methods
	size_t front() const;
	size_t back() const;
	void push_back(size_t key);
	void push_front(size_t key);
	void sort();
	void remove(size_t key);
};

// Actually a FastSet<Key>
#include <gtsam/nonlinear/Key.h>
class KeySet {
	KeySet();
	KeySet(const gtsam::KeySet& other);

	// Testable
	void print(string s) const;
	bool equals(const gtsam::KeySet& other) const;

	// common STL methods
	size_t size() const;
	bool empty() const;
	void clear();

	// structure specific methods
	void insert(size_t key);
	bool erase(size_t key); // returns true if value was removed
	bool count(size_t key) const; // returns true if value exists
};

// Actually a FastVector<Key>
#include <gtsam/nonlinear/Key.h>
class KeyVector {
	KeyVector();
	KeyVector(const gtsam::KeyVector& other);
	KeyVector(const gtsam::KeySet& other);
	KeyVector(const gtsam::KeyList& other);

	// Note: no print function

	// common STL methods
	size_t size() const;
	bool empty() const;
	void clear();

	// structure specific methods
	size_t at(size_t i) const;
	size_t front() const;
	size_t back() const;
	void push_back(size_t key) const;
};

#include <gtsam/nonlinear/Marginals.h>
class Marginals {
	Marginals(const gtsam::NonlinearFactorGraph& graph,
			const gtsam::Values& solution);
	void print(string s) const;
	Matrix marginalCovariance(size_t variable) const;
	Matrix marginalInformation(size_t variable) const;
	gtsam::JointMarginal jointMarginalCovariance(const gtsam::KeyVector& variables) const;
	gtsam::JointMarginal jointMarginalInformation(const gtsam::KeyVector& variables) const;
};

class JointMarginal {
	Matrix at(size_t iVariable, size_t jVariable) const;
	void print(string s) const;
	void print() const;
};

//*************************************************************************
// Nonlinear optimizers
//*************************************************************************

#include <gtsam/nonlinear/NonlinearOptimizer.h>
virtual class NonlinearOptimizerParams {
	NonlinearOptimizerParams();
	void print(string s) const;

	size_t getMaxIterations() const;
	double getRelativeErrorTol() const;
	double getAbsoluteErrorTol() const;
	double getErrorTol() const;
	string getVerbosity() const;

	void setMaxIterations(size_t value);
	void setRelativeErrorTol(double value);
	void setAbsoluteErrorTol(double value);
	void setErrorTol(double value);
	void setVerbosity(string s);
};

#include <gtsam/nonlinear/SuccessiveLinearizationOptimizer.h>
virtual class SuccessiveLinearizationParams : gtsam::NonlinearOptimizerParams {
  SuccessiveLinearizationParams();

	void setOrdering(const gtsam::Ordering& ordering);
  bool isMultifrontal() const;
  bool isSequential() const;
  bool isCholmod() const;
  bool isCG() const;
};

#include <gtsam/nonlinear/LevenbergMarquardtOptimizer.h>
virtual class LevenbergMarquardtParams : gtsam::SuccessiveLinearizationParams {
	LevenbergMarquardtParams();

  double getlambdaInitial() const;
  double getlambdaFactor() const;
  double getlambdaUpperBound() const;
  string getVerbosityLM() const;

  void setlambdaInitial(double value);
  void setlambdaFactor(double value);
  void setlambdaUpperBound(double value);
  void setVerbosityLM(string s);
};

#include <gtsam/nonlinear/DoglegOptimizer.h>
virtual class DoglegParams : gtsam::SuccessiveLinearizationParams {
	DoglegParams();

	double getDeltaInitial() const;
	string getVerbosityDL() const;

	void setDeltaInitial(double deltaInitial) const;
	void setVerbosityDL(string verbosityDL) const;
};

virtual class NonlinearOptimizer {
	gtsam::Values optimizeSafely();
	double error() const;
	int iterations() const;
	gtsam::Values values() const;
	void iterate() const;
};

virtual class DoglegOptimizer : gtsam::NonlinearOptimizer {
	DoglegOptimizer(const gtsam::NonlinearFactorGraph& graph, const gtsam::Values& initialValues);
	DoglegOptimizer(const gtsam::NonlinearFactorGraph& graph, const gtsam::Values& initialValues, const gtsam::DoglegParams& params);
	double getDelta() const;
};

virtual class LevenbergMarquardtOptimizer : gtsam::NonlinearOptimizer {
	LevenbergMarquardtOptimizer(const gtsam::NonlinearFactorGraph& graph, const gtsam::Values& initialValues);
	LevenbergMarquardtOptimizer(const gtsam::NonlinearFactorGraph& graph, const gtsam::Values& initialValues, const gtsam::LevenbergMarquardtParams& params);
	double lambda() const;
};

#include <gtsam/nonlinear/ISAM2.h>
class ISAM2GaussNewtonParams {
  ISAM2GaussNewtonParams();

  void print(string str) const;

  /** Getters and Setters for all properties */
  double getWildfireThreshold() const;
  void setWildfireThreshold(double wildfireThreshold);
};

class ISAM2DoglegParams {
  ISAM2DoglegParams();

  void print(string str) const;

  /** Getters and Setters for all properties */
  double getWildfireThreshold() const;
  void setWildfireThreshold(double wildfireThreshold);
  double getInitialDelta() const;
  void setInitialDelta(double initialDelta);
  string getAdaptationMode() const;
  void setAdaptationMode(string adaptationMode);
  bool isVerbose() const;
  void setVerbose(bool verbose);
};

class ISAM2Params {
  ISAM2Params();

  void print(string str) const;

  /** Getters and Setters for all properties */
  void setOptimizationParams(const gtsam::ISAM2GaussNewtonParams& params);
  void setOptimizationParams(const gtsam::ISAM2DoglegParams& params);
  void setRelinearizeThreshold(double relinearizeThreshold);
  // TODO: wrap this
  //void setRelinearizeThreshold(const FastMap<char,Vector>& relinearizeThreshold);
  int getRelinearizeSkip() const;
  void setRelinearizeSkip(int relinearizeSkip);
  bool isEnableRelinearization() const;
  void setEnableRelinearization(bool enableRelinearization);
  bool isEvaluateNonlinearError() const;
  void setEvaluateNonlinearError(bool evaluateNonlinearError);
  string getFactorization() const;
  void setFactorization(string factorization);
  bool isCacheLinearizedFactors() const;
  void setCacheLinearizedFactors(bool cacheLinearizedFactors);
  bool isEnableDetailedResults() const;
  void setEnableDetailedResults(bool enableDetailedResults);
  bool isEnablePartialRelinearizationCheck() const;
  void setEnablePartialRelinearizationCheck(bool enablePartialRelinearizationCheck);
};

class ISAM2Result {
  ISAM2Result();

  void print(string str) const;

  /** Getters and Setters for all properties */
  size_t getVariablesRelinearized() const;
  size_t getVariablesReeliminated() const;
  size_t getCliques() const;
};

class ISAM2 {
  ISAM2();
  ISAM2(const gtsam::ISAM2Params& params);

  bool equals(const gtsam::ISAM2& other, double tol) const;
  void print(string s) const;

  gtsam::ISAM2Result update();
  gtsam::ISAM2Result update(const gtsam::NonlinearFactorGraph& newFactors, const gtsam::Values& newTheta);
  gtsam::ISAM2Result update(const gtsam::NonlinearFactorGraph& newFactors, const gtsam::Values& newTheta, const gtsam::KeyVector& removeFactorIndices);
  // TODO: wrap the full version of update
  //void update(const gtsam::NonlinearFactorGraph& newFactors, const gtsam::Values& newTheta, const gtsam::KeyVector& removeFactorIndices, FastMap<Key,int>& constrainedKeys);
  //void update(const gtsam::NonlinearFactorGraph& newFactors, const gtsam::Values& newTheta, const gtsam::KeyVector& removeFactorIndices, FastMap<Key,int>& constrainedKeys, bool force_relinearize);

  gtsam::Values getLinearizationPoint() const;
  gtsam::Values calculateEstimate() const;
  gtsam::Values calculateBestEstimate() const;
  gtsam::VectorValues getDelta() const;
  gtsam::NonlinearFactorGraph getFactorsUnsafe() const;
  gtsam::Ordering getOrdering() const;
  gtsam::VariableIndex getVariableIndex() const;
  gtsam::ISAM2Params params() const;
};

//*************************************************************************
// Nonlinear factor types
//*************************************************************************
#include <gtsam/geometry/Cal3_S2.h>
#include <gtsam/geometry/Cal3DS2.h>
#include <gtsam/geometry/Cal3_S2Stereo.h>
#include <gtsam/geometry/SimpleCamera.h>
#include <gtsam/geometry/CalibratedCamera.h>
#include <gtsam/geometry/StereoPoint2.h>

#include <gtsam/slam/PriorFactor.h>
template<T = {gtsam::LieVector, gtsam::LieMatrix, gtsam::Point2, gtsam::StereoPoint2, gtsam::Point3, gtsam::Rot2, gtsam::Rot3, gtsam::Pose2, gtsam::Pose3, gtsam::Cal3_S2, gtsam::CalibratedCamera, gtsam::SimpleCamera}>
virtual class PriorFactor : gtsam::NonlinearFactor {
	PriorFactor(size_t key, const T& prior, const gtsam::noiseModel::Base* noiseModel);
};


#include <gtsam/slam/BetweenFactor.h>
template<T = {gtsam::LieVector, gtsam::LieMatrix, gtsam::Point2, gtsam::Point3, gtsam::Rot2, gtsam::Rot3, gtsam::Pose2, gtsam::Pose3}>
virtual class BetweenFactor : gtsam::NonlinearFactor {
	BetweenFactor(size_t key1, size_t key2, const T& relativePose, const gtsam::noiseModel::Base* noiseModel);
};


#include <gtsam/nonlinear/NonlinearEquality.h>
template<T = {gtsam::LieVector, gtsam::LieMatrix, gtsam::Point2, gtsam::StereoPoint2, gtsam::Point3, gtsam::Rot2, gtsam::Rot3, gtsam::Pose2, gtsam::Pose3, gtsam::Cal3_S2, gtsam::CalibratedCamera, gtsam::SimpleCamera}>
virtual class NonlinearEquality : gtsam::NonlinearFactor {
	// Constructor - forces exact evaluation
	NonlinearEquality(size_t j, const T& feasible);
	// Constructor - allows inexact evaluation
	NonlinearEquality(size_t j, const T& feasible, double error_gain);
};


#include <gtsam/slam/RangeFactor.h>
template<POSE, POINT>
virtual class RangeFactor : gtsam::NonlinearFactor {
	RangeFactor(size_t key1, size_t key2, double measured, const gtsam::noiseModel::Base* noiseModel);
};

typedef gtsam::RangeFactor<gtsam::Pose2, gtsam::Point2> RangeFactorPosePoint2;
typedef gtsam::RangeFactor<gtsam::Pose3, gtsam::Point3> RangeFactorPosePoint3;
typedef gtsam::RangeFactor<gtsam::Pose2, gtsam::Pose2> RangeFactorPose2;
typedef gtsam::RangeFactor<gtsam::Pose3, gtsam::Pose3> RangeFactorPose3;
typedef gtsam::RangeFactor<gtsam::CalibratedCamera, gtsam::Point3> RangeFactorCalibratedCameraPoint;
typedef gtsam::RangeFactor<gtsam::SimpleCamera, gtsam::Point3> RangeFactorSimpleCameraPoint;
typedef gtsam::RangeFactor<gtsam::CalibratedCamera, gtsam::CalibratedCamera> RangeFactorCalibratedCamera;
typedef gtsam::RangeFactor<gtsam::SimpleCamera, gtsam::SimpleCamera> RangeFactorSimpleCamera;

template<POSE, POINT, ROTATION>
virtual class BearingFactor : gtsam::NonlinearFactor {
	BearingFactor(size_t key1, size_t key2, const ROTATION& measured, const gtsam::noiseModel::Base* noiseModel);
};

typedef gtsam::BearingFactor<gtsam::Pose2, gtsam::Point2, gtsam::Rot2> BearingFactor2D;


template<POSE, POINT, ROTATION>
virtual class BearingRangeFactor : gtsam::NonlinearFactor {
	BearingRangeFactor(size_t poseKey, size_t pointKey, const ROTATION& measuredBearing, double measuredRange, const gtsam::noiseModel::Base* noiseModel);
};

typedef gtsam::BearingRangeFactor<gtsam::Pose2, gtsam::Point2, gtsam::Rot2> BearingRangeFactor2D;


#include <gtsam/slam/ProjectionFactor.h>
template<POSE, LANDMARK, CALIBRATION>
virtual class GenericProjectionFactor : gtsam::NonlinearFactor {
	GenericProjectionFactor(const gtsam::Point2& measured, const gtsam::noiseModel::Base* noiseModel,
		size_t poseKey, size_t pointKey, const CALIBRATION* k);
	gtsam::Point2 measured() const;
	CALIBRATION* calibration() const;
};
typedef gtsam::GenericProjectionFactor<gtsam::Pose3, gtsam::Point3, gtsam::Cal3_S2> GenericProjectionFactorCal3_S2;
typedef gtsam::GenericProjectionFactor<gtsam::Pose3, gtsam::Point3, gtsam::Cal3DS2> GenericProjectionFactorCal3DS2;

} //\namespace gtsam

//*************************************************************************
// Pose2SLAM
//*************************************************************************

namespace pose2SLAM {

#include <gtsam/slam/pose2SLAM.h>
class Values {
	Values();
  Values(const pose2SLAM::Values& values);
	size_t size() const;
	void print(string s) const;
  bool exists(size_t key);
  gtsam::KeyVector keys() const; // Note the switch to KeyVector, rather than KeyList

	static pose2SLAM::Values Circle(size_t n, double R);
	void insertPose(size_t key, const gtsam::Pose2& pose);
	void updatePose(size_t key, const gtsam::Pose2& pose);
	gtsam::Pose2 pose(size_t i);
  Matrix poses() const;
};

#include <gtsam/slam/pose2SLAM.h>
class Graph {
	Graph();
  Graph(const gtsam::NonlinearFactorGraph& graph);
  Graph(const pose2SLAM::Graph& graph);

	// FactorGraph
	void print(string s) const;
	bool equals(const pose2SLAM::Graph& fg, double tol) const;
	size_t size() const;
	bool empty() const;
	void remove(size_t i);
	size_t nrFactors() const;
	gtsam::NonlinearFactor* at(size_t i) const;

	// NonlinearFactorGraph
	double error(const pose2SLAM::Values& values) const;
	double probPrime(const pose2SLAM::Values& values) const;
	gtsam::Ordering* orderingCOLAMD(const pose2SLAM::Values& values) const;
	gtsam::GaussianFactorGraph* linearize(const pose2SLAM::Values& values,
			const gtsam::Ordering& ordering) const;

	// pose2SLAM-specific
	void addPoseConstraint(size_t key, const gtsam::Pose2& pose);
	void addPosePrior(size_t key, const gtsam::Pose2& pose, const gtsam::noiseModel::Base* noiseModel);
	void addRelativePose(size_t key1, size_t key2, const gtsam::Pose2& relativePoseMeasurement, const gtsam::noiseModel::Base* noiseModel);
	pose2SLAM::Values optimize(const pose2SLAM::Values& initialEstimate, size_t verbosity) const;
	pose2SLAM::Values optimizeSPCG(const pose2SLAM::Values& initialEstimate, size_t verbosity) const;
	gtsam::Marginals marginals(const pose2SLAM::Values& solution) const;
};

} //\namespace pose2SLAM

//*************************************************************************
// Pose3SLAM
//*************************************************************************

namespace pose3SLAM {

#include <gtsam/slam/pose3SLAM.h>
class Values {
	Values();
  Values(const pose3SLAM::Values& values);
	size_t size() const;
	void print(string s) const;
  bool exists(size_t key);
  gtsam::KeyVector keys() const; // Note the switch to KeyVector, rather than KeyList

	static pose3SLAM::Values Circle(size_t n, double R);
	void insertPose(size_t key, const gtsam::Pose3& pose);
	void updatePose(size_t key, const gtsam::Pose3& pose);
	gtsam::Pose3 pose(size_t i);
  Matrix translations() const;
};

#include <gtsam/slam/pose3SLAM.h>
class Graph {
	Graph();
  Graph(const gtsam::NonlinearFactorGraph& graph);
  Graph(const pose3SLAM::Graph& graph);

	// FactorGraph
	void print(string s) const;
	bool equals(const pose3SLAM::Graph& fg, double tol) const;
	size_t size() const;
	bool empty() const;
	void remove(size_t i);
	size_t nrFactors() const;
	gtsam::NonlinearFactor* at(size_t i) const;

	// NonlinearFactorGraph
	double error(const pose3SLAM::Values& values) const;
	double probPrime(const pose3SLAM::Values& values) const;
	gtsam::Ordering* orderingCOLAMD(const pose3SLAM::Values& values) const;
	gtsam::GaussianFactorGraph* linearize(const pose3SLAM::Values& values,
			const gtsam::Ordering& ordering) const;

	// pose3SLAM-specific
	void addPoseConstraint(size_t i, const gtsam::Pose3& p);
	void addPosePrior(size_t key, const gtsam::Pose3& p, const gtsam::noiseModel::Base* model);
	void addRelativePose(size_t key1, size_t key2, const gtsam::Pose3& z, const gtsam::noiseModel::Base* model);
	pose3SLAM::Values optimize(const pose3SLAM::Values& initialEstimate, size_t verbosity) const;
	// FIXME gtsam::LevenbergMarquardtOptimizer optimizer(const pose3SLAM::Values& initialEstimate, const gtsam::LevenbergMarquardtParams& parameters) const;
	gtsam::Marginals marginals(const pose3SLAM::Values& solution) const;
};

} //\namespace pose3SLAM

//*************************************************************************
// planarSLAM
//*************************************************************************

namespace planarSLAM {

#include <gtsam/slam/planarSLAM.h>
class Values {
	Values();
  Values(const planarSLAM::Values& values);
	size_t size() const;
	void print(string s) const;
  bool exists(size_t key);
  gtsam::KeyVector keys() const; // Note the switch to KeyVector, rather than KeyList

  // inherited from pose2SLAM
  static planarSLAM::Values Circle(size_t n, double R);
	void insertPose(size_t key, const gtsam::Pose2& pose);
	void updatePose(size_t key, const gtsam::Pose2& pose);
	gtsam::Pose2 pose(size_t i);
  Matrix poses() const;

  // Access to poses
  planarSLAM::Values allPoses() const;
  size_t nrPoses() const;
  gtsam::KeyVector poseKeys() const; // Note the switch to KeyVector, rather than KeyList

  // Access to points
  planarSLAM::Values allPoints() const;
  size_t nrPoints() const;
  gtsam::KeyVector pointKeys() const; // Note the switch to KeyVector, rather than KeyList

  void insertPoint(size_t key, const gtsam::Point2& point);
	void updatePoint(size_t key, const gtsam::Point2& point);
	gtsam::Point2 point(size_t key) const;
  Matrix points() const;
};

#include <gtsam/slam/planarSLAM.h>
class Graph {
	Graph();
  Graph(const gtsam::NonlinearFactorGraph& graph);
  Graph(const pose2SLAM::Graph& graph);
  Graph(const planarSLAM::Graph& graph);

	// FactorGraph
	void print(string s) const;
	bool equals(const planarSLAM::Graph& fg, double tol) const;
	size_t size() const;
	bool empty() const;
	void remove(size_t i);
	size_t nrFactors() const;
	gtsam::NonlinearFactor* at(size_t i) const;

	// NonlinearFactorGraph
	double error(const planarSLAM::Values& values) const;
	double probPrime(const planarSLAM::Values& values) const;
	gtsam::Ordering* orderingCOLAMD(const planarSLAM::Values& values) const;
	gtsam::GaussianFactorGraph* linearize(const planarSLAM::Values& values,
			const gtsam::Ordering& ordering) const;

	// pose2SLAM-inherited
	void addPoseConstraint(size_t key, const gtsam::Pose2& pose);
	void addPosePrior(size_t key, const gtsam::Pose2& pose, const gtsam::noiseModel::Base* noiseModel);
	void addRelativePose(size_t key1, size_t key2, const gtsam::Pose2& relativePoseMeasurement, const gtsam::noiseModel::Base* noiseModel);
	planarSLAM::Values optimize(const planarSLAM::Values& initialEstimate, size_t verbosity) const;
	planarSLAM::Values optimizeSPCG(const planarSLAM::Values& initialEstimate, size_t verbosity) const;
	gtsam::Marginals marginals(const planarSLAM::Values& solution) const;

	// planarSLAM-specific
  void addPointConstraint(size_t pointKey, const gtsam::Point2& p);
  void addPointPrior(size_t pointKey, const gtsam::Point2& p, const gtsam::noiseModel::Base* model);
	void addBearing(size_t poseKey, size_t pointKey, const gtsam::Rot2& bearing, const gtsam::noiseModel::Base* noiseModel);
	void addRange(size_t poseKey, size_t pointKey, double range, const gtsam::noiseModel::Base* noiseModel);
	void addBearingRange(size_t poseKey, size_t pointKey, const gtsam::Rot2& bearing,double range, const gtsam::noiseModel::Base* noiseModel);
};

#include <gtsam/slam/planarSLAM.h>
class Odometry {
	Odometry(size_t key1, size_t key2, const gtsam::Pose2& measured,
			const gtsam::noiseModel::Base* model);
	void print(string s) const;
	gtsam::GaussianFactor* linearize(const planarSLAM::Values& center,
			const gtsam::Ordering& ordering) const;
};

} //\namespace planarSLAM

//*************************************************************************
// VisualSLAM
//*************************************************************************

namespace visualSLAM {

#include <gtsam/slam/visualSLAM.h>
class Values {
  Values();
  Values(const visualSLAM::Values& values);
  size_t size() const;
  void print(string s) const;
  bool exists(size_t key);
  gtsam::KeyVector keys() const; // Note the switch to KeyVector, rather than KeyList

  // pose3SLAM inherited
	static visualSLAM::Values Circle(size_t n, double R);
	void insertPose(size_t key, const gtsam::Pose3& pose);
	void updatePose(size_t key, const gtsam::Pose3& pose);
	gtsam::Pose3 pose(size_t i);
  Matrix translations() const;

  // Access to poses
  visualSLAM::Values allPoses() const;
  size_t nrPoses() const;
  gtsam::KeyVector poseKeys() const; // Note the switch to KeyVector, rather than KeyList

  // Access to points
  visualSLAM::Values allPoints() const;
  size_t nrPoints() const;
  gtsam::KeyVector pointKeys() const; // Note the switch to KeyVector, rather than KeyList

  void insertPoint(size_t key, const gtsam::Point3& pose);
  void updatePoint(size_t key, const gtsam::Point3& pose);
  gtsam::Point3 point(size_t j);
  void insertBackprojections(const gtsam::SimpleCamera& c, Vector J, Matrix Z, double depth);
  void perturbPoints(double sigma, size_t seed);
  Matrix points() const;
};

#include <gtsam/slam/visualSLAM.h>
class Graph {
  Graph();
  Graph(const gtsam::NonlinearFactorGraph& graph);
  Graph(const pose3SLAM::Graph& graph);
  Graph(const visualSLAM::Graph& graph);

	// FactorGraph
	void print(string s) const;
	bool equals(const visualSLAM::Graph& fg, double tol) const;
	size_t size() const;
	bool empty() const;
	void remove(size_t i);
	size_t nrFactors() const;
	gtsam::NonlinearFactor* at(size_t i) const;

  double error(const visualSLAM::Values& values) const;
  gtsam::Ordering* orderingCOLAMD(const visualSLAM::Values& values) const;
  gtsam::GaussianFactorGraph* linearize(const visualSLAM::Values& values,
      const gtsam::Ordering& ordering) const;

	// pose3SLAM-inherited
	void addPoseConstraint(size_t i, const gtsam::Pose3& p);
	void addPosePrior(size_t key, const gtsam::Pose3& p, const gtsam::noiseModel::Base* model);
	void addRelativePose(size_t key1, size_t key2, const gtsam::Pose3& z, const gtsam::noiseModel::Base* model);
	visualSLAM::Values optimize(const visualSLAM::Values& initialEstimate, size_t verbosity) const;
	visualSLAM::LevenbergMarquardtOptimizer optimizer(const visualSLAM::Values& initialEstimate, const gtsam::LevenbergMarquardtParams& parameters) const;
	gtsam::Marginals marginals(const visualSLAM::Values& solution) const;

	// Priors and constraints
  void addPointConstraint(size_t pointKey, const gtsam::Point3& p);
  void addPointPrior(size_t pointKey, const gtsam::Point3& p, const gtsam::noiseModel::Base* model);
  void addRangeFactor(size_t poseKey, size_t pointKey, double range, const gtsam::noiseModel::Base* model);

  // Measurements
    void addMeasurement(const gtsam::Point2& measured,
        const gtsam::noiseModel::Base* model, size_t poseKey, size_t pointKey,
        const gtsam::Cal3_S2* K);
    void addMeasurements(size_t i, Vector J, Matrix Z,
        const gtsam::noiseModel::Base* model, const gtsam::Cal3_S2* K);
    void addStereoMeasurement(const gtsam::StereoPoint2& measured,
        const gtsam::noiseModel::Base* model, size_t poseKey, size_t pointKey,
        const gtsam::Cal3_S2Stereo* K);

  // Information
  Matrix reprojectionErrors(const visualSLAM::Values& values) const;

};

#include <gtsam/slam/visualSLAM.h>
class ISAM {
	ISAM();
	ISAM(int reorderInterval);
  void print(string s) const;
  void printStats() const;
  void saveGraph(string s) const;
	visualSLAM::Values estimate() const;
  Matrix marginalCovariance(size_t key) const;
  int reorderInterval() const;
  int reorderCounter() const;
  void update(const visualSLAM::Graph& newFactors, const visualSLAM::Values& initialValues);
  void reorder_relinearize();
  void addKey(size_t key);
  void setOrdering(const gtsam::Ordering& new_ordering);

  // These might be expensive as instead of a reference the wrapper will make a copy
  gtsam::GaussianISAM bayesTree() const;
  visualSLAM::Values getLinearizationPoint() const;
  gtsam::Ordering getOrdering() const;
  gtsam::NonlinearFactorGraph getFactorsUnsafe() const;
};

#include <gtsam/slam/visualSLAM.h>
class LevenbergMarquardtOptimizer {
  double lambda() const;
  void iterate();
  double error() const;
  size_t iterations() const;
  visualSLAM::Values optimize();
  visualSLAM::Values optimizeSafely();
  visualSLAM::Values values() const;
};

} //\namespace visualSLAM

//************************************************************************
// sparse BA
//************************************************************************

namespace sparseBA {

#include <gtsam/slam/sparseBA.h>
class Values {
  Values();
  Values(const sparseBA::Values& values);
  size_t size() const;
  void print(string s) const;
  bool exists(size_t key);
  gtsam::KeyVector keys() const;

  // Access to cameras
  sparseBA::Values allSimpleCameras() const ;
  size_t  nrSimpleCameras() const ;
  gtsam::KeyVector simpleCameraKeys() const ;
  void insertSimpleCamera(size_t j, const gtsam::SimpleCamera& camera);
  void updateSimpleCamera(size_t j, const gtsam::SimpleCamera& camera);
  gtsam::SimpleCamera simpleCamera(size_t j) const;

  // Access to points, inherited from visualSLAM
  sparseBA::Values allPoints() const;
  size_t nrPoints() const;
  gtsam::KeyVector pointKeys() const; // Note the switch to KeyVector, rather than KeyList
  void insertPoint(size_t key, const gtsam::Point3& pose);
  void updatePoint(size_t key, const gtsam::Point3& pose);
  gtsam::Point3 point(size_t j);
  Matrix points() const;
};

#include <gtsam/slam/sparseBA.h>
class Graph {
  Graph();
  Graph(const gtsam::NonlinearFactorGraph& graph);
  Graph(const sparseBA::Graph& graph);

  // Information
  Matrix reprojectionErrors(const sparseBA::Values& values) const;

  // inherited from FactorGraph
  void print(string s) const;
  bool equals(const sparseBA::Graph& fg, double tol) const;
  size_t size() const;
  bool empty() const;
  void remove(size_t i);
  size_t nrFactors() const;
  gtsam::NonlinearFactor* at(size_t i) const;

  double error(const sparseBA::Values& values) const;
  gtsam::Ordering* orderingCOLAMD(const sparseBA::Values& values) const;
  gtsam::GaussianFactorGraph* linearize(const sparseBA::Values& values, const gtsam::Ordering& ordering) const;

  sparseBA::Values optimize(const sparseBA::Values& initialEstimate, size_t verbosity) const;
  sparseBA::LevenbergMarquardtOptimizer optimizer(const sparseBA::Values& initialEstimate, const gtsam::LevenbergMarquardtParams& parameters) const;
  gtsam::Marginals marginals(const sparseBA::Values& solution) const;

  // inherited from visualSLAM
  void addPointConstraint(size_t pointKey, const gtsam::Point3& p);
  void addPointPrior(size_t pointKey, const gtsam::Point3& p, const gtsam::noiseModel::Base* model);

  // add factors
  void addSimpleCameraPrior(size_t cameraKey, const gtsam::SimpleCamera &camera, gtsam::noiseModel::Base* model);
  void addSimpleCameraConstraint(size_t cameraKey, const gtsam::SimpleCamera &camera);
  void addSimpleCameraMeasurement(const gtsam::Point2 &z, gtsam::noiseModel::Base* model, size_t cameraKey, size_t pointKey);
};

#include <gtsam/slam/sparseBA.h>
class LevenbergMarquardtOptimizer {
  double lambda() const;
  void iterate();
  double error() const;
  size_t iterations() const;
  sparseBA::Values optimize();
  sparseBA::Values optimizeSafely();
  sparseBA::Values values() const;
};
} //\namespace sparseBA

