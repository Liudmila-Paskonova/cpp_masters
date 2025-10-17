#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>
#include <initializer_list>
#include <ostream>
#include <array>

namespace matrix
{
template <typename T>
concept IntegerOrFloatingPointType = std::integral<T> || std::floating_point<T>;

template <typename M>
concept SquareMatrixType = std::default_initializable<M> && std::copy_constructible<M> && requires {
    typename M::value_type;
    typename M::size_type;
} && requires(M &m, const M &cm) {
    { m *= cm } -> std::same_as<M &>;
    { m += cm } -> std::same_as<M &>;
    { m == m } -> std::convertible_to<bool>;
};

template <typename T>
concept NthPowerType = SquareMatrixType<T> || IntegerOrFloatingPointType<T>;

template <typename T, size_t N>
    requires IntegerOrFloatingPointType<T> && (N > 0)
class SquareMatrix
{
    std::array<std::array<T, N>, N> data_;

  public:
    using value_type = T;
    using size_type = size_t;
    constexpr SquareMatrix() { data_ = {}; }
    constexpr SquareMatrix(const T &elem)
    {
        data_ = {};
        for (size_type i = 0; i < N; ++i) {
            data_[i][i] = elem;
        }
    }

    constexpr SquareMatrix(std::initializer_list<std::initializer_list<T>> lst)
    {
        assert(lst.size() == N);
        auto row_iterator = lst.begin();
        for (size_type i = 0; i < N; ++i, ++row_iterator) {
            assert(row_iterator->size() == N);
            auto column_iterator = row_iterator->begin();
            for (size_type j = 0; j < N; ++j, ++column_iterator) {
                data_[i][j] = *column_iterator;
            }
        }
    }

    SquareMatrix(const SquareMatrix &) = default;
    SquareMatrix(SquareMatrix &&) = default;

    SquareMatrix &operator=(const SquareMatrix &) = default;
    SquareMatrix &operator=(SquareMatrix &&) = default;

    constexpr T &
    operator()(size_type row, size_type column)
    {
        return data_[row][column];
    };

    constexpr const T &
    operator()(size_type row, size_type column) const
    {
        return data_[row][column];
    };

    SquareMatrix &
    operator+=(const SquareMatrix &other)
    {
        SquareMatrix<T, N> result;
        for (size_type i = 0; i < N; ++i) {
            for (size_type j = 0; j < N; ++j) {
                result.data_[i][j] = this->data_[i][j] + other.data_[i][j];
            }
        }
        this->data_ = result.data_;
        return *this;
    }

    SquareMatrix &
    operator*=(const SquareMatrix &other)
    {
        SquareMatrix<T, N> result;
        for (size_type i = 0; i < N; ++i) {
            for (size_type j = 0; j < N; ++j) {
                for (size_type k = 0; k < N; ++k) {
                    result.data_[i][j] += this->data_[i][k] * other.data_[k][j];
                }
            }
        }
        this->data_ = result.data_;
        return *this;
    }

    bool
    operator==(const SquareMatrix &other) const
    {
        return this->data_ == other.data_;
    }

    friend std::ostream &
    operator<<(std::ostream &os, const SquareMatrix &m)
    {
        for (size_type i = 0; i < N; ++i) {
            for (size_type j = 0; j < N; ++j) {
                os << m.data_[i][j] << " ";
            }
            os << "\n";
        }
        return os;
    }
};
}; // namespace matrix
