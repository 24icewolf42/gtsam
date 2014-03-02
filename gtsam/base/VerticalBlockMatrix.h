/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file    VerticalBlockMatrix.h
 * @brief   A matrix with column blocks of pre-defined sizes.  Used in JacobianFactor and
 *          GaussianConditional.
 * @author  Richard Roberts
 * @date    Sep 18, 2010 */
#pragma once

#include <gtsam/base/Matrix.h>
#include <gtsam/base/FastVector.h>

namespace gtsam {

// Forward declarations
class SymmetricBlockMatrix;

/**
 * This class stores a dense matrix and allows it to be accessed as a collection
 * of vertical blocks.
 *
 * The dimensions of the blocks are provided when constructing this class.
 *
 * This class also has three parameters that can be changed after construction
 * that change the apparent view of the matrix without any reallocation or data
 * copying. firstBlock() determines the block that has index 0 for all operations
 * (except for re-setting firstBlock()).  rowStart() determines the apparent
 * first row of the matrix for all operations (except for setting rowStart() and
 * rowEnd()).  rowEnd() determines the apparent exclusive (one-past-the-last)
 * last row for all operations.  To include all rows, rowEnd() should be set to
 * the number of rows in the matrix (i.e. one after the last true row index).
 *
 * @addtogroup base
 */
class GTSAM_EXPORT VerticalBlockMatrix {
public:
  typedef VerticalBlockMatrix This;
  typedef Eigen::Block<Matrix> Block;
  typedef Eigen::Block<const Matrix> constBlock;

protected:
  Matrix matrix_; ///< The full matrix

  /// the starting columns of each block (0-based)
  FastVector<DenseIndex> variableColOffsets_;

  DenseIndex rowStart_; ///< Changes apparent matrix view, see class comments.
  DenseIndex rowEnd_; ///< Changes apparent matrix view, see class comments.
  DenseIndex blockStart_; ///< Changes apparent matrix view, see class comments.

#define ASSERT_INVARIANTS \
  assert(matrix_.cols() == variableColOffsets_.back());\
  assert(blockStart_ < (DenseIndex)variableColOffsets_.size());\
  assert(rowStart_ <= matrix_.rows());\
  assert(rowEnd_ <= matrix_.rows());\
  assert(rowStart_ <= rowEnd_);\

#define CHECK_BLOCK(block) \
  assert(matrix_.cols() == variableColOffsets_.back());\
  assert(block < (DenseIndex)variableColOffsets_.size() - 1);\
  assert(variableColOffsets_[block] < matrix_.cols() && variableColOffsets_[block+1] <= matrix_.cols());

public:

  /** Construct an empty VerticalBlockMatrix */
  VerticalBlockMatrix() :
      rowStart_(0), rowEnd_(0), blockStart_(0) {
    variableColOffsets_.push_back(0);
    ASSERT_INVARIANTS
  }

  /** Construct from a container of the sizes of each vertical block. */
  template<typename CONTAINER>
  VerticalBlockMatrix(const CONTAINER& dimensions, DenseIndex height) :
      rowStart_(0), rowEnd_(height), blockStart_(0) {
    fillOffsets(dimensions.begin(), dimensions.end());
    matrix_.resize(height, variableColOffsets_.back());
    ASSERT_INVARIANTS
  }

  /**
   * Construct from a container of the sizes of each vertical block and a
   * pre-prepared matrix.
   */
  template<typename CONTAINER>
  VerticalBlockMatrix(const CONTAINER& dimensions, const Matrix& matrix) :
      matrix_(matrix), rowStart_(0), rowEnd_(matrix.rows()), blockStart_(0) {
    fillOffsets(dimensions.begin(), dimensions.end());
    if (variableColOffsets_.back() != matrix_.cols())
      throw std::invalid_argument(
          "Requested to create a VerticalBlockMatrix with dimensions that do not sum to the total columns of the provided matrix.");
    ASSERT_INVARIANTS
  }

  /**
   * Construct from iterator over the sizes of each vertical block. */
  template<typename ITERATOR>
  VerticalBlockMatrix(ITERATOR firstBlockDim, ITERATOR lastBlockDim,
      DenseIndex height) :
      rowStart_(0), rowEnd_(height), blockStart_(0) {
    fillOffsets(firstBlockDim, lastBlockDim);
    matrix_.resize(height, variableColOffsets_.back());
    ASSERT_INVARIANTS
  }

  /**
   * Copy the block structure and resize the underlying matrix, but do not copy
   * the matrix data. If blockStart(), rowStart(), and/or rowEnd() have been
   * modified, this copies the structure of the corresponding matrix view. In the
   * destination VerticalBlockView, blockStart() and rowStart() will thus be 0,
   * rowEnd() will be cols() of the source VerticalBlockView, and the
   * underlying matrix will be the size of the view of the source matrix.
   */
  static VerticalBlockMatrix LikeActiveViewOf(const VerticalBlockMatrix& rhs);

  /** Copy the block structure, but do not copy the matrix data. If blockStart()
   * has been modified, this copies the structure of the corresponding matrix
   * view. In the destination VerticalBlockMatrix, blockStart() will be 0. */
  static VerticalBlockMatrix LikeActiveViewOf(const SymmetricBlockMatrix& rhs,
      DenseIndex height);

  /// Row size
  inline DenseIndex rows() const {
    ASSERT_INVARIANTS
    return rowEnd_ - rowStart_;
  }

  /// Column size
  inline DenseIndex cols() const {
    ASSERT_INVARIANTS
    return variableColOffsets_.back() - variableColOffsets_[blockStart_];
  }

  /// Block count
  inline DenseIndex nBlocks() const {
    ASSERT_INVARIANTS
    return variableColOffsets_.size() - 1 - blockStart_;
  }

  /** Access a single block in the underlying matrix with read/write access */
  inline Block operator()(DenseIndex block) {
    return range(block, block + 1);
  }

  /** Access a const block view */
  inline const constBlock operator()(DenseIndex block) const {
    return range(block, block + 1);
  }

  /** access ranges of blocks at a time */
  Block range(DenseIndex startBlock, DenseIndex endBlock) {
    ASSERT_INVARIANTS
    DenseIndex actualStartBlock = startBlock + blockStart_;
    DenseIndex actualEndBlock = endBlock + blockStart_;
    if (startBlock != 0 || endBlock != 0) {
      CHECK_BLOCK(actualStartBlock);
      assert(actualEndBlock < (DenseIndex)variableColOffsets_.size());
    }
    const DenseIndex startCol = variableColOffsets_[actualStartBlock];
    const DenseIndex rangeCols = variableColOffsets_[actualEndBlock] - startCol;
    return matrix_.block(rowStart_, startCol, this->rows(), rangeCols);
  }

  const constBlock range(DenseIndex startBlock, DenseIndex endBlock) const {
    ASSERT_INVARIANTS
    DenseIndex actualStartBlock = startBlock + blockStart_;
    DenseIndex actualEndBlock = endBlock + blockStart_;
    if (startBlock != 0 || endBlock != 0) {
      CHECK_BLOCK(actualStartBlock);
      assert(actualEndBlock < (DenseIndex)variableColOffsets_.size());
    }
    const DenseIndex startCol = variableColOffsets_[actualStartBlock];
    const DenseIndex rangeCols = variableColOffsets_[actualEndBlock] - startCol;
    return ((const Matrix&) matrix_).block(rowStart_, startCol, this->rows(),
        rangeCols);
  }

  /** Return the full matrix, *not* including any portions excluded by
   * rowStart(), rowEnd(), and firstBlock() */
  inline Block full() {
    return range(0, nBlocks());
  }

  /** Return the full matrix, *not* including any portions excluded by
   * rowStart(), rowEnd(), and firstBlock() */
  inline const constBlock full() const {
    return range(0, nBlocks());
  }

  inline DenseIndex offset(DenseIndex block) const {
    ASSERT_INVARIANTS
    DenseIndex actualBlock = block + blockStart_;
    CHECK_BLOCK(actualBlock);
    return variableColOffsets_[actualBlock];
  }

  /// Get/set the apparent first row of the underlying matrix for all operations
  inline DenseIndex& rowStart() {
    return rowStart_;
  }

  /** Get/set the apparent last row
   * (exclusive, i.e. rows() == rowEnd() - rowStart())
   * of the underlying matrix for all operations */
  inline DenseIndex& rowEnd() {
    return rowEnd_;
  }

  /** Get/set the apparent first block for all operations */
  inline DenseIndex& firstBlock() {
    return blockStart_;
  }

  /** Get the apparent first row of the underlying matrix for all operations */
  inline DenseIndex rowStart() const {
    return rowStart_;
  }

  /** Get the apparent last row (exclusive, i.e. rows() == rowEnd() - rowStart())
   *  of the underlying matrix for all operations */
  inline DenseIndex rowEnd() const {
    return rowEnd_;
  }

  /** Get the apparent first block for all operations */
  inline DenseIndex firstBlock() const {
    return blockStart_;
  }

  /** Access to full matrix (*including* any portions excluded by rowStart(),
   * rowEnd(), and firstBlock()) */
  inline const Matrix& matrix() const {
    return matrix_;
  }

  /** Non-const access to full matrix (*including* any portions excluded by
   * rowStart(), rowEnd(), and firstBlock()) */
  inline Matrix& matrix() {
    return matrix_;
  }

protected:

  void assertInvariants() const {
    ASSERT_INVARIANTS
  }

  void checkBlock(DenseIndex block) const {
    CHECK_BLOCK(block)
  }

  template<typename ITERATOR>
  void fillOffsets(ITERATOR firstBlockDim, ITERATOR lastBlockDim) {
    variableColOffsets_.resize((lastBlockDim - firstBlockDim) + 1);
    variableColOffsets_[0] = 0;
    DenseIndex j = 0;
    for (ITERATOR dim = firstBlockDim; dim != lastBlockDim; ++dim) {
      variableColOffsets_[j + 1] = variableColOffsets_[j] + *dim;
      ++j;
    }
  }

  friend class SymmetricBlockMatrix;

private:
  /** Serialization function */
  friend class boost::serialization::access;
  template<class ARCHIVE>
  void serialize(ARCHIVE & ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_NVP(matrix_);
    ar & BOOST_SERIALIZATION_NVP(variableColOffsets_);
    ar & BOOST_SERIALIZATION_NVP(rowStart_);
    ar & BOOST_SERIALIZATION_NVP(rowEnd_);
    ar & BOOST_SERIALIZATION_NVP(blockStart_);
  }
};

}
