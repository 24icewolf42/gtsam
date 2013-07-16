/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file    GaussianConditional.h
 * @brief   Conditional Gaussian Base class
 * @author  Christian Potthast
 */

// \callgraph

#pragma once

#include <boost/utility.hpp>

#include <gtsam/global_includes.h>
#include <gtsam/linear/JacobianFactorUnordered.h>
#include <gtsam/inference/ConditionalUnordered.h>
#include <gtsam/linear/VectorValuesUnordered.h>

namespace gtsam {

  /**
  * A conditional Gaussian functions as the node in a Bayes network
  * It has a set of parents y,z, etc. and implements a probability density on x.
  * The negative log-probability is given by \f$ \frac{1}{2} |Rx - (d - Sy - Tz - ...)|^2 \f$
  */
  class GTSAM_EXPORT GaussianConditionalUnordered :
    public JacobianFactorUnordered,
    public ConditionalUnordered<JacobianFactorUnordered, GaussianConditionalUnordered>
  {
  public:
    typedef GaussianConditionalUnordered This; ///< Typedef to this class
    typedef boost::shared_ptr<This> shared_ptr; ///< shared_ptr to this class
    typedef JacobianFactorUnordered BaseFactor; ///< Typedef to our factor base class
    typedef ConditionalUnordered<BaseFactor, This> BaseConditional; ///< Typedef to our conditional base class

    /** default constructor needed for serialization */
    GaussianConditionalUnordered() {}

    /** constructor with no parents |Rx-d| */
    GaussianConditionalUnordered(Key key, const Vector& d, const Matrix& R,
      const SharedDiagonal& sigmas = SharedDiagonal());

    /** constructor with only one parent |Rx+Sy-d| */
    GaussianConditionalUnordered(Key key, const Vector& d, const Matrix& R,
      Key name1, const Matrix& S, const SharedDiagonal& sigmas = SharedDiagonal());

    /** constructor with two parents |Rx+Sy+Tz-d| */
    GaussianConditionalUnordered(Key key, const Vector& d, const Matrix& R,
      Key name1, const Matrix& S, Key name2, const Matrix& T,
      const SharedDiagonal& sigmas = SharedDiagonal());

    /** Constructor with number of arbitrary parents.  \f$ |Rx+sum(Ai*xi)-d| \f$
     *  @tparam PARENTS A container whose value type is std::pair<Key, Matrix>, specifying the
     *          collection of parent keys and matrices. */
    template<class PARENTS>
    GaussianConditionalUnordered(Key key, const Vector& d,
      const Matrix& R, const PARENTS& parents,
      const SharedDiagonal& sigmas = SharedDiagonal(),
      const typename PARENTS::value_type* = 0);

    /** Constructor with arbitrary number of frontals and parents.
    *   @tparam TERMS A container whose value type is std::pair<Key, Matrix>, specifying the
    *           collection of keys and matrices making up the conditional. */
    template<typename TERMS>
    GaussianConditionalUnordered(const TERMS& terms,
      size_t nrFrontals, const Vector& d,
      const SharedDiagonal& sigmas = SharedDiagonal());

    /** Constructor with arbitrary number keys, and where the augmented matrix is given all together
     *  instead of in block terms.  Note that only the active view of the provided augmented matrix
     *  is used, and that the matrix data is copied into a newly-allocated matrix in the constructed
     *  factor. */
    template<typename KEYS>
    GaussianConditionalUnordered(
      const KEYS& keys, size_t nrFrontals, const VerticalBlockMatrix& augmentedMatrix,
      const SharedDiagonal& sigmas = SharedDiagonal());

    /** Combine several GaussianConditional into a single dense GC.  The conditionals enumerated by
    *   \c first and \c last must be in increasing order, meaning that the parents of any
    *   conditional may not include a conditional coming before it.
    *   @param firstConditional Iterator to the first conditional to combine, must dereference to a
    *          shared_ptr<GaussianConditional>.
    *   @param lastConditional Iterator to after the last conditional to combine, must dereference
    *          to a shared_ptr<GaussianConditional>. */
    template<typename ITERATOR>
    static shared_ptr Combine(ITERATOR firstConditional, ITERATOR lastConditional);

    /** print */
    void print(const std::string& = "GaussianConditional",
      const KeyFormatter& formatter = DefaultKeyFormatter) const;

    /** equals function */
    bool equals(const GaussianConditionalUnordered&cg, double tol = 1e-9) const;

    /** Return a view of the upper-triangular R block of the conditional */
    constABlock get_R() const { return Ab_.range(0, nrFrontals()); }

    /** Get a view of the parent blocks. */
    constABlock get_S() const { return Ab_.range(nrFrontals(), size()); }

    /** Get a view of the S matrix for the variable pointed to by the given key iterator */
    constABlock get_S(const_iterator variable) const { return BaseFactor::getA(variable); }

    /** Get a view of the r.h.s. vector d */
    const constBVector get_d() const { return BaseFactor::getb(); }

    /**
    * Solves a conditional Gaussian and writes the solution into the entries of
    * \c x for each frontal variable of the conditional.  The parents are
    * assumed to have already been solved in and their values are read from \c x.
    * This function works for multiple frontal variables.
    *
    * Given the Gaussian conditional with log likelihood \f$ |R x_f - (d - S x_s)|^2,
    * where \f$ f \f$ are the frontal variables and \f$ s \f$ are the separator
    * variables of this conditional, this solve function computes
    * \f$ x_f = R^{-1} (d - S x_s) \f$ using back-substitution.
    *
    * @param parents VectorValues containing solved parents \f$ x_s \f$.
    */
    VectorValuesUnordered solve(const VectorValuesUnordered& parents) const;

    VectorValuesUnordered solveOtherRHS(const VectorValuesUnordered& parents, const VectorValuesUnordered& rhs) const;

    /** Performs transpose backsubstition in place on values */
    void solveTransposeInPlace(VectorValuesUnordered& gy) const;

    /** Scale the values in \c gy according to the sigmas for the frontal variables in this
     *  conditional. */
    __declspec(deprecated) void scaleFrontalsBySigma(VectorValuesUnordered& gy) const;

  private:

    /** Serialization function */
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
      ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(BaseFactor);
      ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(BaseConditional);
    }
  }; // GaussianConditionalUnordered

} // gtsam

#include <gtsam/linear/GaussianConditionalUnordered-inl.h>

