/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation,
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file    cholesky.cpp
 * @brief   Efficient incomplete Cholesky on rank-deficient matrices, todo: constrained Cholesky
 * @author  Richard Roberts
 * @date    Nov 5, 2010
 */

#include <gtsam/base/debug.h>
#include <gtsam/base/cholesky.h>
#include <gtsam/base/timing.h>

#include <gtsam/3rdparty/Eigen/Eigen/Core>
#include <gtsam/3rdparty/Eigen/Eigen/Dense>

#include <boost/format.hpp>
#include <boost/bind.hpp>

using namespace std;

namespace gtsam {

  static const double negativePivotThreshold = -1e-1;
  static const double zeroPivotThreshold = 1e-6;
  static const double underconstrainedPrior = 1e-5;

/* ************************************************************************* */
static inline bool choleskyStep(Matrix& ATA, size_t k, size_t order) {

  // Get pivot value
  double alpha = ATA(k,k);

  // Correct negative pivots from round-off error
  if(alpha < negativePivotThreshold) {
    cout << "pivot = " << alpha << endl;
    print(ATA, "Partially-factorized matrix: ");
    throw(invalid_argument("The matrix was found to be non-positive-semidefinite when factoring with careful Cholesky."));
  } else if(alpha < 0.0)
    alpha = 0.0;
    
  const double beta = sqrt(alpha);

  if(beta > zeroPivotThreshold) {
    const double betainv = 1.0 / beta;

    // Update k,k
    ATA(k,k) = beta;

    if(k < (order-1)) {
      // Update A(k,k+1:end) <- A(k,k+1:end) / beta
    	typedef typeof(ATA.row(k).segment(k+1, order-(k+1))) BlockRow;
    	BlockRow V = ATA.row(k).segment(k+1, order-(k+1));
    	V *= betainv;

      // Update A(k+1:end, k+1:end) <- A(k+1:end, k+1:end) - v*v' / alpha
      ATA.block(k+1, k+1, order-(k+1), order-(k+1)) -= V.transpose() * V;
    }

    return true;
  } else {
    // For zero pivots, add the underconstrained variable prior
    ATA(k,k) = underconstrainedPrior;
    for(size_t j=k+1; j<order; ++j)
      ATA(k,j) = 0.0;
    return false;
  }
}

/* ************************************************************************* */
pair<size_t,bool> choleskyCareful(Matrix& ATA, int order) {

  const bool debug = ISDEBUG("choleskyCareful");

  // Check that the matrix is square (we do not check for symmetry)
  assert(ATA.rows() == ATA.cols());

  // Number of rows/columns
  const size_t n = ATA.rows();

  // Negative order means factor the entire matrix
  if(order < 0)
    order = n;

  assert(size_t(order) <= n);

  // The index of the row after the last non-zero row of the square-root factor
  size_t maxrank = 0;
  bool fullRank = true;

  // Factor row-by-row
  for(size_t k = 0; k < size_t(order); ++k) {
    if(choleskyStep(ATA, k, size_t(order))) {
      if(debug) cout << "choleskyCareful:  Factored through " << k << endl;
      if(debug) print(ATA, "ATA: ");
      maxrank = k+1;
    } else {
      fullRank = false;
      if(debug) cout << "choleskyCareful:  Skipping " << k << endl;
    }
  }

  return make_pair(maxrank, fullRank);
}

/* ************************************************************************* */
void choleskyPartial(Matrix& ABC, size_t nFrontal) {

  const bool debug = ISDEBUG("choleskyPartial");

  assert(ABC.rows() == ABC.cols());
  assert(ABC.rows() >= 0 && nFrontal <= size_t(ABC.rows()));

  const size_t n = ABC.rows();

  // Compute Cholesky factorization of A, overwrites A.
  tic(1, "lld");
  ABC.block(0,0,nFrontal,nFrontal).triangularView<Eigen::Upper>() =
      ABC.block(0,0,nFrontal,nFrontal).selfadjointView<Eigen::Upper>().llt().matrixU();
  assert(ABC.topLeftCorner(nFrontal,nFrontal).triangularView<Eigen::Upper>().toDenseMatrix().unaryExpr(&isfinite<double>).all());
  toc(1, "lld");

  if(debug) cout << "R:\n" << Eigen::MatrixXd(ABC.topLeftCorner(nFrontal,nFrontal).triangularView<Eigen::Upper>()) << endl;

  // Compute S = inv(R') * B
  tic(2, "compute S");
  if(n - nFrontal > 0) {
    ABC.topLeftCorner(nFrontal,nFrontal).triangularView<Eigen::Upper>().transpose().solveInPlace(
        ABC.topRightCorner(nFrontal, n-nFrontal));
  }
  assert(ABC.topRightCorner(nFrontal, n-nFrontal).unaryExpr(&isfinite<double>).all());
  if(debug) cout << "S:\n" << ABC.topRightCorner(nFrontal, n-nFrontal) << endl;
  toc(2, "compute S");

  // Compute L = C - S' * S
  tic(3, "compute L");
  if(debug) cout << "C:\n" << Eigen::MatrixXd(ABC.bottomRightCorner(n-nFrontal,n-nFrontal).selfadjointView<Eigen::Upper>()) << endl;
  if(n - nFrontal > 0)
    ABC.bottomRightCorner(n-nFrontal,n-nFrontal).selfadjointView<Eigen::Upper>().rankUpdate(
        ABC.topRightCorner(nFrontal, n-nFrontal).transpose(), -1.0);
  assert(ABC.bottomRightCorner(n-nFrontal,n-nFrontal).selfadjointView<Eigen::Upper>().toDenseMatrix().unaryExpr(&isfinite<double>).all());
  if(debug) cout << "L:\n" << Eigen::MatrixXd(ABC.bottomRightCorner(n-nFrontal,n-nFrontal).selfadjointView<Eigen::Upper>()) << endl;
  toc(3, "compute L");
}

/* ************************************************************************* */
Eigen::LDLT<Matrix>::TranspositionType ldlPartial(Matrix& ABC, size_t nFrontal) {

  const bool debug = ISDEBUG("ldlPartial");

  assert(ABC.rows() == ABC.cols());
  assert(ABC.rows() >= 0 && nFrontal <= size_t(ABC.rows()));

  const size_t n = ABC.rows();

  // Compute Cholesky factorization of A, overwrites A = sqrt(D)*R
  //  tic(1, "ldl");
  Eigen::LDLT<Matrix> ldlt;
  ldlt.compute(ABC.block(0,0,nFrontal,nFrontal).selfadjointView<Eigen::Upper>());

  if(ldlt.vectorD().unaryExpr(boost::bind(less<double>(), _1, 0.0)).any()) {
    if(debug) {
      gtsam::print(Matrix(ldlt.matrixU()), "U: ");
      gtsam::print(Vector(ldlt.vectorD()), "D: ");
    }
    throw NegativeMatrixException();
  }

  Vector sqrtD = ldlt.vectorD().cwiseSqrt();
  if (debug) cout << "Dsqrt: " << sqrtD << endl;

  // U = sqrtD * L^
  Matrix U = ldlt.matrixU();

  // we store the permuted upper triangular matrix
  ABC.block(0,0,nFrontal,nFrontal)  = sqrtD.asDiagonal() * U;
  if(debug) cout << "R:\n" << ABC.topLeftCorner(nFrontal,nFrontal) << endl;
  //  toc(1, "ldl");

  // Compute S = inv(R') * B = inv(P'U')*B = inv(U')*P*B
  //  tic(2, "compute S");
  if(n - nFrontal > 0) {
    ABC.topRightCorner(nFrontal, n-nFrontal) = ldlt.transpositionsP() * ABC.topRightCorner(nFrontal, n-nFrontal);
    ABC.block(0,0,nFrontal,nFrontal).triangularView<Eigen::Upper>().transpose().solveInPlace(ABC.topRightCorner(nFrontal, n-nFrontal));
  }
  if(debug) cout << "S:\n" << ABC.topRightCorner(nFrontal, n-nFrontal) << endl;
  //  toc(2, "compute S");

  // Compute L = C - S' * S
  //  tic(3, "compute L");
  if(debug) cout << "C:\n" << Eigen::MatrixXd(ABC.bottomRightCorner(n-nFrontal,n-nFrontal).selfadjointView<Eigen::Upper>()) << endl;
  if(n - nFrontal > 0)
    ABC.bottomRightCorner(n-nFrontal,n-nFrontal).selfadjointView<Eigen::Upper>().rankUpdate(
        ABC.topRightCorner(nFrontal, n-nFrontal).transpose(), -1.0);
  if(debug) cout << "L:\n" << Eigen::MatrixXd(ABC.bottomRightCorner(n-nFrontal,n-nFrontal).selfadjointView<Eigen::Upper>()) << endl;
  //  toc(3, "compute L");

  return ldlt.transpositionsP();
}

}
