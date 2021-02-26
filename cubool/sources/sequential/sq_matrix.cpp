/**********************************************************************************/
/* MIT License                                                                    */
/*                                                                                */
/* Copyright (c) 2020, 2021 JetBrains-Research                                    */
/*                                                                                */
/* Permission is hereby granted, free of charge, to any person obtaining a copy   */
/* of this software and associated documentation files (the "Software"), to deal  */
/* in the Software without restriction, including without limitation the rights   */
/* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell      */
/* copies of the Software, and to permit persons to whom the Software is          */
/* furnished to do so, subject to the following conditions:                       */
/*                                                                                */
/* The above copyright notice and this permission notice shall be included in all */
/* copies or substantial portions of the Software.                                */
/*                                                                                */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     */
/* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,       */
/* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE    */
/* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER         */
/* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  */
/* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  */
/* SOFTWARE.                                                                      */
/**********************************************************************************/

#include <sequential/sq_matrix.hpp>
#include <sequential/sq_transpose.hpp>
#include <sequential/sq_submatrix.hpp>
#include <sequential/sq_kronecker.hpp>
#include <core/error.hpp>
#include <algorithm>
#include <cassert>

namespace cubool {

    SqMatrix::SqMatrix(size_t nrows, size_t ncols) {
        assert(nrows > 0);
        assert(ncols > 0);

        mData.mNrows = nrows;
        mData.mNcols = ncols;
    }

    void SqMatrix::build(const index *rows, const index *cols, size_t nvals, bool isSorted) {
        if (nvals == 0)
            return;

        assert(rows);
        assert(cols);

        auto nrows = mData.mNrows;
        auto ncols = mData.mNcols;

        mData.mRowIndices.clear();
        mData.mColIndices.clear();
        mData.mRowIndices.reserve(nvals);
        mData.mColIndices.reserve(nvals);

        if (isSorted) {
            for (size_t k = 0; k < nvals; k++) {
                auto i = rows[k];
                auto j = cols[k];

                CHECK_RAISE_ERROR(i < nrows, InvalidArgument, "Index out of matrix bound");
                CHECK_RAISE_ERROR(j < ncols, InvalidArgument, "Index out of matrix bound");

                mData.mRowIndices.push_back(i);
                mData.mColIndices.push_back(j);
            }
        }
        else {
            std::vector<Pair> values;
            values.reserve(nvals);

            for (size_t k = 0; k < nvals; k++) {
                auto i = rows[k];
                auto j = cols[k];

                CHECK_RAISE_ERROR(i < nrows, InvalidArgument, "Index out of matrix bound");
                CHECK_RAISE_ERROR(j < ncols, InvalidArgument, "Index out of matrix bound");

                values.push_back({i, j});
            }

            std::sort(values.begin(), values.end(), [](const Pair& a, const Pair& b) {
                return a.i < b.i || (a.i == b.i && a.j < b.j);
            });

            for (const auto& p: values) {
                mData.mRowIndices.push_back(p.i);
                mData.mColIndices.push_back(p.j);
            }
        }

        mData.mNvals = nvals;
    }

    void SqMatrix::extract(index *rows, index *cols, size_t &nvals) {
        assert(nvals >= getNvals());

        nvals = getNvals();

        if (nvals > 0) {
            assert(rows);
            assert(cols);

            for (size_t k = 0; k < getNvals(); k++) {
                rows[k] = mData.mRowIndices[k];
                cols[k] = mData.mColIndices[k];
            }
        }
    }

    void SqMatrix::extractSubMatrix(const MatrixBase &otherBase, index i, index j, index nrows, index ncols) {
        auto other = dynamic_cast<const SqMatrix*>(&otherBase);

        CHECK_RAISE_ERROR(other != nullptr, InvalidArgument, "Provided matrix does not belongs to sequential matrix class");

        auto M = this->getNrows();
        auto N = this->getNcols();

        assert(M == nrows);
        assert(N == ncols);

        sq_submatrix(other->mData, this->mData, i, j, nrows, ncols);
    }

    void SqMatrix::clone(const MatrixBase &otherBase) {
        auto other = dynamic_cast<const SqMatrix*>(&otherBase);

        CHECK_RAISE_ERROR(other != nullptr, InvalidArgument, "Provided matrix does not belongs to sequential matrix class");

        auto M = other->getNrows();
        auto N = other->getNcols();

        assert(M == this->getNrows());
        assert(N == this->getNcols());

        this->mData = other->mData;
    }

    void SqMatrix::transpose(const MatrixBase &otherBase) {
        auto other = dynamic_cast<const SqMatrix*>(&otherBase);

        CHECK_RAISE_ERROR(other != nullptr, InvalidArgument, "Provided matrix does not belongs to sequential matrix class");

        auto M = other->getNrows();
        auto N = other->getNcols();

        assert(N == this->getNrows());
        assert(M == this->getNcols());

        sq_transpose(other->mData, this->mData);
    }

    void SqMatrix::reduce(const MatrixBase &otherBase) {
        auto other = dynamic_cast<const SqMatrix*>(&otherBase);

        CHECK_RAISE_ERROR(other != nullptr, InvalidArgument, "Provided matrix does not belongs to sequential matrix class");

        auto M = other->getNrows();
        auto N = other->getNcols();
    }

    void SqMatrix::multiply(const MatrixBase &aBase, const MatrixBase &bBase, bool accumulate) {

    }

    void SqMatrix::kronecker(const MatrixBase &aBase, const MatrixBase &bBase) {
        auto a = dynamic_cast<const SqMatrix*>(&aBase);
        auto b = dynamic_cast<const SqMatrix*>(&bBase);

        CHECK_RAISE_ERROR(a != nullptr, InvalidArgument, "Provided matrix does not belongs to sequential matrix class");
        CHECK_RAISE_ERROR(b != nullptr, InvalidArgument, "Provided matrix does not belongs to sequential matrix class");

        auto M = a->getNrows();
        auto N = a->getNcols();
        auto K = b->getNrows();
        auto T = b->getNcols();

        assert(M * K == this->getNrows());
        assert(N * T == this->getNcols());

        sq_kronecker(a->mData, b->mData, this->mData);
    }

    void SqMatrix::eWiseAdd(const MatrixBase &aBase, const MatrixBase &bBase) {

    }

    index SqMatrix::getNrows() const {
        return mData.mNrows;
    }

    index SqMatrix::getNcols() const {
        return mData.mNcols;
    }

    index SqMatrix::getNvals() const {
        return mData.mNvals;
    }
}