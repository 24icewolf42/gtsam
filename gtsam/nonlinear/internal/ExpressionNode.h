/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file ExpressionNode.h
 * @date May 10, 2015
 * @author Frank Dellaert
 * @author Paul Furgale
 * @brief ExpressionNode class
 */

#pragma once

#include <gtsam/nonlinear/internal/ExecutionTrace.h>
#include <gtsam/nonlinear/internal/CallRecord.h>
#include <gtsam/nonlinear/Values.h>

#include <typeinfo>       // operator typeid
#include <ostream>
#include <map>

class ExpressionFactorBinaryTest;
// Forward declare for testing

namespace gtsam {
namespace internal {

template<typename T>
T & upAlign(T & value, unsigned requiredAlignment = TraceAlignment) {
  // right now only word sized types are supported.
  // Easy to extend if needed,
  //   by somehow inferring the unsigned integer of same size
  BOOST_STATIC_ASSERT(sizeof(T) == sizeof(size_t));
  size_t & uiValue = reinterpret_cast<size_t &>(value);
  size_t misAlignment = uiValue % requiredAlignment;
  if (misAlignment) {
    uiValue += requiredAlignment - misAlignment;
  }
  return value;
}
template<typename T>
T upAligned(T value, unsigned requiredAlignment = TraceAlignment) {
  return upAlign(value, requiredAlignment);
}

//-----------------------------------------------------------------------------

/**
 * Expression node. The superclass for objects that do the heavy lifting
 * An Expression<T> has a pointer to an ExpressionNode<T> underneath
 * allowing Expressions to have polymorphic behaviour even though they
 * are passed by value. This is the same way boost::function works.
 * http://loki-lib.sourceforge.net/html/a00652.html
 */
template<class T>
class ExpressionNode {

protected:

  size_t traceSize_;

  /// Constructor, traceSize is size of the execution trace of expression rooted here
  ExpressionNode(size_t traceSize = 0) :
      traceSize_(traceSize) {
  }

public:

  /// Destructor
  virtual ~ExpressionNode() {
  }

  /// Streaming
  GTSAM_EXPORT
  friend std::ostream &operator<<(std::ostream &os,
      const ExpressionNode& node) {
    os << "Expression of type " << typeid(T).name();
    if (node.traceSize_ > 0)
      os << ", trace size = " << node.traceSize_;
    os << "\n";
    return os;
  }

  /// Return keys that play in this expression as a set
  virtual std::set<Key> keys() const {
    std::set<Key> keys;
    return keys;
  }

  /// Return dimensions for each argument, as a map
  virtual void dims(std::map<Key, int>& map) const {
  }

  // Return size needed for memory buffer in traceExecution
  size_t traceSize() const {
    return traceSize_;
  }

  /// Return value
  virtual T value(const Values& values) const = 0;

  /// Construct an execution trace for reverse AD
  virtual T traceExecution(const Values& values, ExecutionTrace<T>& trace,
      ExecutionTraceStorage* traceStorage) const = 0;
};

//-----------------------------------------------------------------------------
/// Constant Expression
template<class T>
class ConstantExpression: public ExpressionNode<T> {

  /// The constant value
  T constant_;

  /// Constructor with a value, yielding a constant
  ConstantExpression(const T& value) :
      constant_(value) {
  }

  friend class Expression<T> ;

public:

  /// Return value
  virtual T value(const Values& values) const {
    return constant_;
  }

  /// Construct an execution trace for reverse AD
  virtual T traceExecution(const Values& values, ExecutionTrace<T>& trace,
      ExecutionTraceStorage* traceStorage) const {
    return constant_;
  }
};

//-----------------------------------------------------------------------------
/// Leaf Expression, if no chart is given, assume default chart and value_type is just the plain value
template<typename T>
class LeafExpression: public ExpressionNode<T> {
  typedef T value_type;

  /// The key into values
  Key key_;

  /// Constructor with a single key
  LeafExpression(Key key) :
      key_(key) {
  }
  // todo: do we need a virtual destructor here too?

  friend class Expression<T> ;

public:

  /// Return keys that play in this expression
  virtual std::set<Key> keys() const {
    std::set<Key> keys;
    keys.insert(key_);
    return keys;
  }

  /// Return dimensions for each argument
  virtual void dims(std::map<Key, int>& map) const {
    map[key_] = traits<T>::dimension;
  }

  /// Return value
  virtual T value(const Values& values) const {
    return values.at<T>(key_);
  }

  /// Construct an execution trace for reverse AD
  virtual T traceExecution(const Values& values, ExecutionTrace<T>& trace,
      ExecutionTraceStorage* traceStorage) const {
    trace.setLeaf(key_);
    return values.at<T>(key_);
  }

};

//-----------------------------------------------------------------------------
/// meta-function to generate fixed-size JacobianTA type
template<class T, class A>
struct Jacobian {
  typedef Eigen::Matrix<double, traits<T>::dimension, traits<A>::dimension> type;
};

// Eigen format for printing Jacobians
static const Eigen::IOFormat kMatlabFormat(0, 1, " ", "; ", "", "", "[", "]");

//-----------------------------------------------------------------------------
/// Unary Function Expression
template<class T, class A1>
class UnaryExpression: public ExpressionNode<T> {

  typedef typename Expression<T>::template UnaryFunction<A1>::type Function;
  boost::shared_ptr<ExpressionNode<A1> > expression1_;
  Function function_;

public:

  /// Constructor with a unary function f, and input argument e1
  UnaryExpression(Function f, const Expression<A1>& e1) :
      expression1_(e1.root()), function_(f) {
    ExpressionNode<T>::traceSize_ = upAligned(sizeof(Record)) + e1.traceSize();
  }

  /// Return value
  virtual T value(const Values& values) const {
    using boost::none;
    return function_(expression1_->value(values), none);
  }

  /// Return keys that play in this expression
  virtual std::set<Key> keys() const {
    return expression1_->keys();
  }

  /// Return dimensions for each argument
  virtual void dims(std::map<Key, int>& map) const {
    expression1_->dims(map);
  }

  // Inner Record Class
  struct Record: public CallRecordImplementor<Record, traits<T>::dimension> {

    A1 value1;
    ExecutionTrace<A1> trace1;
    typename Jacobian<T, A1>::type dTdA1;

    /// Print to std::cout
    void print(const std::string& indent) const {
      std::cout << indent << "UnaryExpression::Record {" << std::endl;
      std::cout << indent << dTdA1.format(kMatlabFormat) << std::endl;
      trace1.print(indent);
      std::cout << indent << "}" << std::endl;
    }

    /// Start the reverse AD process
    void startReverseAD4(JacobianMap& jacobians) const {
      // This is the crucial point where the size of the AD pipeline is selected.
      // One pipeline is started for each argument, but the number of rows in each
      // pipeline is the same, namely the dimension of the output argument T.
      // For example, if the entire expression is rooted by a binary function
      // yielding a 2D result, then the matrix dTdA will have 2 rows.
      // ExecutionTrace::reverseAD1 just passes this on to CallRecord::reverseAD2
      // which calls the correctly sized CallRecord::reverseAD3, which in turn
      // calls reverseAD4 below.
      trace1.reverseAD1(dTdA1, jacobians);
    }

    /// Given df/dT, multiply in dT/dA and continue reverse AD process
    template<typename SomeMatrix>
    void reverseAD4(const SomeMatrix & dFdT, JacobianMap& jacobians) const {
      trace1.reverseAD1(dFdT * dTdA1, jacobians);
    }
  };

  /// Construct an execution trace for reverse AD
  virtual T traceExecution(const Values& values, ExecutionTrace<T>& trace,
      ExecutionTraceStorage* ptr) const {
    assert(reinterpret_cast<size_t>(ptr) % TraceAlignment == 0);

    // Create the record at the start of the traceStorage and advance the pointer
    Record* record = new (ptr) Record();
    ptr += upAligned(sizeof(Record));

    // Record the traces for all arguments
    // After this, the traceStorage pointer is set to after what was written
    // Write an Expression<A> execution trace in record->trace
    // Iff Constant or Leaf, this will not write to traceStorage, only to trace.
    // Iff the expression is functional, write all Records in traceStorage buffer
    // Return value of type T is recorded in record->value
    record->value1 = expression1_->traceExecution(values, record->trace1, ptr);

    // ptr is never modified by traceExecution, but if traceExecution has
    // written in the buffer, the next caller expects we advance the pointer
    ptr += expression1_->traceSize();
    trace.setFunction(record);

    return function_(record->value1, record->dTdA1);
  }
};

//-----------------------------------------------------------------------------
/// Binary Expression
template<class T, class A1, class A2>
class BinaryExpression: public ExpressionNode<T> {

  typedef typename Expression<T>::template BinaryFunction<A1, A2>::type Function;
  boost::shared_ptr<ExpressionNode<A1> > expression1_;
  boost::shared_ptr<ExpressionNode<A2> > expression2_;
  Function function_;

public:

  /// Constructor with a binary function f, and two input arguments
  BinaryExpression(Function f, const Expression<A1>& e1,
      const Expression<A2>& e2) :
      expression1_(e1.root()), expression2_(e2.root()), function_(f) {
    ExpressionNode<T>::traceSize_ = //
        upAligned(sizeof(Record)) + e1.traceSize() + e2.traceSize();
  }

  friend class ::ExpressionFactorBinaryTest;

  /// Return value
  virtual T value(const Values& values) const {
    using boost::none;
    return function_(expression1_->value(values), expression2_->value(values),
        none, none);
  }

  /// Return keys that play in this expression
  virtual std::set<Key> keys() const {
    std::set<Key> keys = expression1_->keys();
    std::set<Key> myKeys = expression2_->keys();
    keys.insert(myKeys.begin(), myKeys.end());
    return keys;
  }

  /// Return dimensions for each argument
  virtual void dims(std::map<Key, int>& map) const {
    expression1_->dims(map);
    expression2_->dims(map);
  }

  // Inner Record Class
  struct Record: public CallRecordImplementor<Record, traits<T>::dimension> {

    A1 value1;
    ExecutionTrace<A1> trace1;
    typename Jacobian<T, A1>::type dTdA1;

    A2 value2;
    ExecutionTrace<A2> trace2;
    typename Jacobian<T, A2>::type dTdA2;

    /// Print to std::cout
    void print(const std::string& indent) const {
      std::cout << indent << "BinaryExpression::Record {" << std::endl;
      std::cout << indent << dTdA1.format(kMatlabFormat) << std::endl;
      trace1.print(indent);
      std::cout << indent << dTdA2.format(kMatlabFormat) << std::endl;
      trace2.print(indent);
      std::cout << indent << "}" << std::endl;
    }

    /// Start the reverse AD process, see comments in Base
    void startReverseAD4(JacobianMap& jacobians) const {
      trace1.reverseAD1(dTdA1, jacobians);
      trace2.reverseAD1(dTdA2, jacobians);
    }

    /// Given df/dT, multiply in dT/dA and continue reverse AD process
    template<typename SomeMatrix>
    void reverseAD4(const SomeMatrix & dFdT, JacobianMap& jacobians) const {
      trace1.reverseAD1(dFdT * dTdA1, jacobians);
      trace2.reverseAD1(dFdT * dTdA2, jacobians);
    }
  };

  /// Construct an execution trace for reverse AD, see UnaryExpression for explanation
  virtual T traceExecution(const Values& values, ExecutionTrace<T>& trace,
      ExecutionTraceStorage* ptr) const {
    assert(reinterpret_cast<size_t>(ptr) % TraceAlignment == 0);
    Record* record = new (ptr) Record();
    ptr += upAligned(sizeof(Record));
    record->value1 = expression1_->traceExecution(values, record->trace1, ptr);
    record->value2 = expression2_->traceExecution(values, record->trace2, ptr);
    ptr += expression1_->traceSize() + expression2_->traceSize();
    trace.setFunction(record);
    return function_(record->value1, record->value2, record->dTdA1,
        record->dTdA2);
  }
};

//-----------------------------------------------------------------------------
/// Ternary Expression
template<class T, class A1, class A2, class A3>
class TernaryExpression: public ExpressionNode<T> {

  typedef typename Expression<T>::template TernaryFunction<A1, A2, A3>::type Function;
  boost::shared_ptr<ExpressionNode<A1> > expression1_;
  boost::shared_ptr<ExpressionNode<A2> > expression2_;
  boost::shared_ptr<ExpressionNode<A3> > expression3_;
  Function function_;

public:

  /// Constructor with a ternary function f, and two input arguments
  TernaryExpression(Function f, const Expression<A1>& e1,
      const Expression<A2>& e2, const Expression<A3>& e3) :
      expression1_(e1.root()), expression2_(e2.root()), expression3_(e3.root()), //
      function_(f) {
    ExpressionNode<T>::traceSize_ = upAligned(sizeof(Record)) + //
        e1.traceSize() + e2.traceSize() + e3.traceSize();
  }

  /// Return value
  virtual T value(const Values& values) const {
    using boost::none;
    return function_(expression1_->value(values), expression2_->value(values),
        expression3_->value(values), none, none, none);
  }

  /// Return keys that play in this expression
  virtual std::set<Key> keys() const {
    std::set<Key> keys = expression1_->keys();
    std::set<Key> myKeys = expression2_->keys();
    keys.insert(myKeys.begin(), myKeys.end());
    myKeys = expression3_->keys();
    keys.insert(myKeys.begin(), myKeys.end());
    return keys;
  }

  /// Return dimensions for each argument
  virtual void dims(std::map<Key, int>& map) const {
    expression1_->dims(map);
    expression2_->dims(map);
    expression3_->dims(map);
  }

  // Inner Record Class
  struct Record: public CallRecordImplementor<Record, traits<T>::dimension> {

    A1 value1;
    ExecutionTrace<A1> trace1;
    typename Jacobian<T, A1>::type dTdA1;

    A2 value2;
    ExecutionTrace<A2> trace2;
    typename Jacobian<T, A2>::type dTdA2;

    A3 value3;
    ExecutionTrace<A3> trace3;
    typename Jacobian<T, A3>::type dTdA3;

    /// Print to std::cout
    void print(const std::string& indent) const {
      std::cout << indent << "TernaryExpression::Record {" << std::endl;
      std::cout << indent << dTdA1.format(kMatlabFormat) << std::endl;
      trace1.print(indent);
      std::cout << indent << dTdA2.format(kMatlabFormat) << std::endl;
      trace2.print(indent);
      std::cout << indent << dTdA3.format(kMatlabFormat) << std::endl;
      trace3.print(indent);
      std::cout << indent << "}" << std::endl;
    }

    /// Start the reverse AD process, see comments in Base
    void startReverseAD4(JacobianMap& jacobians) const {
      trace1.reverseAD1(dTdA1, jacobians);
      trace2.reverseAD1(dTdA2, jacobians);
      trace3.reverseAD1(dTdA3, jacobians);
    }

    /// Given df/dT, multiply in dT/dA and continue reverse AD process
    template<typename SomeMatrix>
    void reverseAD4(const SomeMatrix & dFdT, JacobianMap& jacobians) const {
      trace1.reverseAD1(dFdT * dTdA1, jacobians);
      trace2.reverseAD1(dFdT * dTdA2, jacobians);
      trace3.reverseAD1(dFdT * dTdA3, jacobians);
    }
  };

  /// Construct an execution trace for reverse AD, see UnaryExpression for explanation
  virtual T traceExecution(const Values& values, ExecutionTrace<T>& trace,
      ExecutionTraceStorage* ptr) const {
    assert(reinterpret_cast<size_t>(ptr) % TraceAlignment == 0);
    Record* record = new (ptr) Record();
    ptr += upAligned(sizeof(Record));
    record->value1 = expression1_->traceExecution(values, record->trace1, ptr);
    record->value2 = expression2_->traceExecution(values, record->trace2, ptr);
    record->value3 = expression3_->traceExecution(values, record->trace3, ptr);
    ptr += expression1_->traceSize() + expression2_->traceSize()
        + expression3_->traceSize();
    trace.setFunction(record);
    return function_(record->value1, record->value2, record->value3,
        record->dTdA1, record->dTdA2, record->dTdA3);
  }
};

} // namespace internal
} // namespace gtsam
