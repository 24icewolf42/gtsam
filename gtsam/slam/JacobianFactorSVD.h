/*
 * @file  JacobianFactorSVD.h
 * @date  Oct 27, 2013
 * @uthor Frank Dellaert
 */

#pragma once
#include <gtsam/linear/RegularJacobianFactor.h>

namespace gtsam {
/**
 * JacobianFactor for Schur complement that uses Q noise model
 */
template<size_t D, size_t ZDim>
class JacobianFactorSVD: public RegularJacobianFactor<D> {

  typedef RegularJacobianFactor<D> Base;
  typedef Eigen::Matrix<double, ZDim, D> MatrixZD; // e.g 2 x 6 with Z=Point2
  typedef std::pair<Key, MatrixZD> KeyMatrixZD;
  typedef std::pair<Key, Matrix> KeyMatrix;

public:

  /// Default constructor
  JacobianFactorSVD() {
  }

  /// Empty constructor with keys
  JacobianFactorSVD(const FastVector<Key>& keys, //
      const SharedDiagonal& model = SharedDiagonal()) :
      Base() {
    Matrix zeroMatrix = Matrix::Zero(0, D);
    Vector zeroVector = Vector::Zero(0);
    std::vector<KeyMatrix> QF;
    QF.reserve(keys.size());
    BOOST_FOREACH(const Key& key, keys)
      QF.push_back(KeyMatrix(key, zeroMatrix));
    JacobianFactor::fillTerms(QF, zeroVector, model);
  }

  /// Constructor
  JacobianFactorSVD(const std::vector<KeyMatrixZD>& Fblocks,
      const Matrix& Enull, const Vector& b, //
      const SharedDiagonal& model = SharedDiagonal()) :
      Base() {
    size_t numKeys = Enull.rows() / ZDim;
    size_t j = 0, m2 = ZDim * numKeys - 3;
    // PLAIN NULL SPACE TRICK
    // Matrix Q = Enull * Enull.transpose();
    // BOOST_FOREACH(const KeyMatrixZD& it, Fblocks)
    //   QF.push_back(KeyMatrix(it.first, Q.block(0, 2 * j++, m2, 2) * it.second));
    // JacobianFactor factor(QF, Q * b);
    std::vector<KeyMatrix> QF;
    QF.reserve(numKeys);
    BOOST_FOREACH(const KeyMatrixZD& it, Fblocks)
      QF.push_back(
          KeyMatrix(it.first,
              (Enull.transpose()).block(0, ZDim * j++, m2, ZDim) * it.second));
    JacobianFactor::fillTerms(QF, Enull.transpose() * b, model);
  }
};

}
