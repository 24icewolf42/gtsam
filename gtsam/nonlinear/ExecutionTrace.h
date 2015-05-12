/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file ExecutionTrace.h
 * @date September 18, 2014
 * @author Frank Dellaert
 * @brief Used in Expression.h, not for general consumption
 */

#pragma once

#include <Eigen/Core>
#include <gtsam/inference/Key.h>
//#include <gtsam/base/Matrix.h>
//#include <gtsam/nonlinear/ExpressionNode.h>
//#include <gtsam/base/Lie.h>
//
//#include <boost/foreach.hpp>
//#include <boost/tuple/tuple.hpp>
//#include <boost/bind.hpp>
//#include <boost/type_traits/aligned_storage.hpp>
//
//#include <map>

namespace gtsam {

template<int T> struct CallRecord;

namespace internal {

template<bool UseBlock, typename Derived>
struct UseBlockIf {
  static void addToJacobian(const Eigen::MatrixBase<Derived>& dTdA,
      JacobianMap& jacobians, Key key) {
    // block makes HUGE difference
    jacobians(key).block<Derived::RowsAtCompileTime, Derived::ColsAtCompileTime>(
        0, 0) += dTdA;
  }
};

/// Handle Leaf Case for Dynamic Matrix type (slower)
template<typename Derived>
struct UseBlockIf<false, Derived> {
  static void addToJacobian(const Eigen::MatrixBase<Derived>& dTdA,
      JacobianMap& jacobians, Key key) {
    jacobians(key) += dTdA;
  }
};
}

/// Handle Leaf Case: reverse AD ends here, by writing a matrix into Jacobians
template<typename Derived>
void handleLeafCase(const Eigen::MatrixBase<Derived>& dTdA,
    JacobianMap& jacobians, Key key) {
  internal::UseBlockIf<
      Derived::RowsAtCompileTime != Eigen::Dynamic
          && Derived::ColsAtCompileTime != Eigen::Dynamic, Derived>::addToJacobian(
      dTdA, jacobians, key);
}

/**
 * The ExecutionTrace class records a tree-structured expression's execution.
 *
 * The class looks a bit complicated but it is so for performance.
 * It is a tagged union that obviates the need to create
 * a ExecutionTrace subclass for Constants and Leaf Expressions. Instead
 * the key for the leaf is stored in the space normally used to store a
 * CallRecord*. Nothing is stored for a Constant.
 *
 * A full execution trace of a Binary(Unary(Binary(Leaf,Constant)),Leaf) would be:
 * Trace(Function) ->
 *   BinaryRecord with two traces in it
 *     trace1(Function) ->
 *       UnaryRecord with one trace in it
 *         trace1(Function) ->
 *           BinaryRecord with two traces in it
 *             trace1(Leaf)
 *             trace2(Constant)
 *     trace2(Leaf)
 * Hence, there are three Record structs, written to memory by traceExecution
 */
template<class T>
class ExecutionTrace {
  static const int Dim = traits<T>::dimension;
  enum {
    Constant, Leaf, Function
  } kind;
  union {
    Key key;
    CallRecord<Dim>* ptr;
  } content;
public:
  /// Pointer always starts out as a Constant
  ExecutionTrace() :
      kind(Constant) {
  }
  /// Change pointer to a Leaf Record
  void setLeaf(Key key) {
    kind = Leaf;
    content.key = key;
  }
  /// Take ownership of pointer to a Function Record
  void setFunction(CallRecord<Dim>* record) {
    kind = Function;
    content.ptr = record;
  }
  /// Print
  void print(const std::string& indent = "") const {
    if (kind == Constant)
      std::cout << indent << "Constant" << std::endl;
    else if (kind == Leaf)
      std::cout << indent << "Leaf, key = " << content.key << std::endl;
    else if (kind == Function) {
      std::cout << indent << "Function" << std::endl;
      content.ptr->print(indent + "  ");
    }
  }
  /// Return record pointer, quite unsafe, used only for testing
  template<class Record>
  boost::optional<Record*> record() {
    if (kind != Function)
      return boost::none;
    else {
      Record* p = dynamic_cast<Record*>(content.ptr);
      return p ? boost::optional<Record*>(p) : boost::none;
    }
  }
  /**
   *  *** This is the main entry point for reverse AD, called from Expression ***
   * Called only once, either inserts I into Jacobians (Leaf) or starts AD (Function)
   */
  typedef Eigen::Matrix<double, Dim, Dim> JacobianTT;
  void startReverseAD1(JacobianMap& jacobians) const {
    if (kind == Leaf) {
      // This branch will only be called on trivial Leaf expressions, i.e. Priors
      static const JacobianTT I = JacobianTT::Identity();
      handleLeafCase(I, jacobians, content.key);
    } else if (kind == Function)
      // This is the more typical entry point, starting the AD pipeline
      // Inside startReverseAD2 the correctly dimensioned pipeline is chosen.
      content.ptr->startReverseAD2(jacobians);
  }
  // Either add to Jacobians (Leaf) or propagate (Function)
  template<typename DerivedMatrix>
  void reverseAD1(const Eigen::MatrixBase<DerivedMatrix> & dTdA,
      JacobianMap& jacobians) const {
    if (kind == Leaf)
      handleLeafCase(dTdA, jacobians, content.key);
    else if (kind == Function)
      content.ptr->reverseAD2(dTdA, jacobians);
  }

  /// Define type so we can apply it as a meta-function
  typedef ExecutionTrace<T> type;
};

} // namespace gtsam
