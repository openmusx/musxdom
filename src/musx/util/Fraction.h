/*
 * Copyright (C) 2025, Robert Patterson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#pragma once

#include <iostream>
#include <numeric>
#include <stdexcept>
#include <limits>

#include "musx/dom/Fundamentals.h"

namespace musx {
namespace util {

/**
 * @class Fraction
 * @brief A class to represent fractions with integer m_numerator and m_denominator, automatically reduced to simplest form.
 */
class [[nodiscard]] Fraction {
private:
    int m_numerator = 0;    ///< The m_numerator of the fraction.
    int m_denominator = 1;  ///< The m_denominator of the fraction.

    /**
     * @brief Reduces the input to its simplest form.
     * Ensures the denominator is always positive.
     */
    static constexpr std::pair<int, int> reduce(int num, int den) {
        int gcd = std::gcd(num, den);
        num /= gcd;
        den /= gcd;

        // Ensure denominator is always positive
        if (den < 0) {
            num = -num;
            den = -den;
        }

        return {num, den};
    }

    friend class std::numeric_limits<Fraction>;

public:
    constexpr Fraction() = default;

    /**
     * @brief Constructs a Fraction object.
     * @param num The m_numerator of the fraction.
     * @param den The m_denominator of the fraction. Defaults to 1.
     * @throws std::invalid_argument if the m_denominator is zero.
     */
    constexpr Fraction(int num, int den)
    {
        if (den == 0) {
            throw std::invalid_argument("Denominator cannot be zero.");
        }

        auto [n, d] = reduce(num, den);
        m_numerator = n;
        m_denominator = d;
    }

    /**
     * @brief Constructs a Fraction object from an integer.
     * @param value The integer value of the fraction.
     * @throws std::invalid_argument if the m_denominator is zero.
     */
    constexpr Fraction(int value) : m_numerator(value), m_denominator(1) {}

    /// @brief Constructs a Fraction from edu.
    /// @param edu The Edu value to convert. It is converted to a fraction of a whole note, so 1024 is
    /// constructed as Fraction(1, 4).
    static constexpr Fraction fromEdu(dom::Edu edu) { return Fraction(edu, dom::EDU_PER_WHOLE_NOTE); }

    /// @brief Constructs a Fraction from a percent (where 100 is 100%)
    /// @param percent The integral percent value to convert.
    static constexpr Fraction fromPercent(int percent) { return Fraction(percent, dom::FULL_SIZE_PERCENT); }

    /**
     * @brief Gets the m_numerator of the fraction.
     * @return The m_numerator.
     */
    [[nodiscard]]
    constexpr int numerator() const { return m_numerator; }

    /**
     * @brief Gets the m_denominator of the fraction.
     * @return The m_denominator.
     */
    [[nodiscard]]
    constexpr int denominator() const { return m_denominator; }

    /**
     * @brief Returns the integer (whole number) part of the fraction.
     * @return The integer part of the fraction.
     */
    [[nodiscard]]
    constexpr int quotient() const {
        return m_numerator / m_denominator;
    }

    /**
     * @brief Returns the fractional part of the fraction.
     * @return The remainder as a fraction, satisfying -1 < remainder < 1.
     */
    Fraction constexpr remainder() const {
        return Fraction(m_numerator % m_denominator, m_denominator);
    }

    /// @brief Returns the reciprocal fraction
    constexpr Fraction reciprocal() const {
        return Fraction(m_denominator, m_numerator);
    }

    /**
     * @brief Calculates duration as a fraction of a whole note.
     *        The result is rounded to the nearest integer Edu value,
     *        with 0.5 rounded away from zero, and saturated to the
     *        numeric range of dom::Edu.
     */
    [[nodiscard]]
    constexpr dom::Edu calcEduDuration() const
    {
        using Edu = dom::Edu;
        using Wide = std::int64_t;

        // Do the scaling in wide integer to avoid overflow.
        const auto num = Wide(m_numerator) * Wide(dom::EDU_PER_WHOLE_NOTE);
        const auto den = Wide(m_denominator); // always > 0 by class invariant

        // Round to nearest, 0.5 away from zero, without using double.
        auto rounded = Wide{};
        if (num >= 0) {
            rounded = (num + den / 2) / den;
        } else {
            rounded = (num - den / 2) / den;
        }

        // Saturate to dom::Edu range.
        constexpr auto maxEdu = static_cast<Wide>((std::numeric_limits<Edu>::max)());
        constexpr auto minEdu = static_cast<Wide>((std::numeric_limits<Edu>::min)());

        if (rounded > maxEdu) {
            rounded = maxEdu;
        } else if (rounded < minEdu) {
            rounded = minEdu;
        }

        return Edu(static_cast<int>(rounded));
    }

    /**
     * @brief Calculates the absolute value of a Fraction.
     */
    constexpr Fraction abs() const
    {
        if (numerator() < 0)
            return Fraction(-numerator(), denominator());
        return *this;
    }

    /**
     * @brief Converts the fraction to floating point double.
     */
    [[nodiscard]]
    constexpr double toDouble() const
    { return double(m_numerator) / double(m_denominator); }

    /**
     * @brief Adds two fractions.
     * @param other The other fraction to add.
     * @return The resulting fraction after addition.
     */
    Fraction constexpr operator+(const Fraction& other) const {
        return Fraction(
            m_numerator * other.m_denominator + other.m_numerator * m_denominator,
            m_denominator * other.m_denominator
        );
    }

    /**
     * @brief Subtracts one fraction from another.
     * @param other The other fraction to subtract.
     * @return The resulting fraction after subtraction.
     */
    Fraction constexpr operator-(const Fraction& other) const {
        return Fraction(
            m_numerator * other.m_denominator - other.m_numerator * m_denominator,
            m_denominator * other.m_denominator
        );
    }

    /**
     * @brief Multiplies two fractions.
     * @param other The other fraction to multiply.
     * @return The resulting fraction after multiplication.
     */
    Fraction constexpr operator*(const Fraction& other) const {
        return Fraction(
            m_numerator * other.m_numerator,
            m_denominator * other.m_denominator
        );
    }

    /**
     * @brief Divides one fraction by another.
     * @param other The other fraction to divide by.
     * @return The resulting fraction after division.
     * @throws std::invalid_argument if attempting to divide by a fraction with a zero m_numerator.
     */
    constexpr Fraction operator/(const Fraction& other) const {
        return Fraction(
            m_numerator * other.m_denominator,
            m_denominator * other.m_numerator
        );
    }

    /**
     * @brief Compound addition assignment operator.
     * @param other The other fraction to add.
     * @return A reference to the updated fraction.
     */
    constexpr Fraction& operator+=(const Fraction& other) {
        *this = *this + other;
        return *this;
    }

    /**
     * @brief Compound subtraction assignment operator.
     * @param other The other fraction to subtract.
     * @return A reference to the updated fraction.
     */
    constexpr Fraction& operator-=(const Fraction& other) {
        *this = *this - other;
        return *this;
    }

    /**
     * @brief Compound multiplication assignment operator.
     * @param other The other fraction to multiply.
     * @return A reference to the updated fraction.
     */
    constexpr Fraction& operator*=(const Fraction& other) {
        *this = *this * other;
        return *this;
    }

    /**
     * @brief Compound division assignment operator.
     * @param other The other fraction to divide by.
     * @return A reference to the updated fraction.
     */
    constexpr Fraction& operator/=(const Fraction& other) {
        *this = *this / other;
        return *this;
    }

    /**
     * @brief Equality comparison operator. (This depends on the fact that instances of Fraction are always reduced in the constructor.)
     * @param other The other fraction to compare.
     * @return True if the fractions are equal, false otherwise.
     */
    [[nodiscard]]
    constexpr bool operator==(const Fraction& other) const {
        return m_numerator == other.m_numerator && m_denominator == other.m_denominator;
    }

    /**
     * @brief Inequality comparison operator.
     * @param other The other fraction to compare.
     * @return True if the fractions are not equal, false otherwise.
     */
    [[nodiscard]]
    constexpr bool operator!=(const Fraction& other) const {
        return !(*this == other);
    }

    /**
     * @brief Less-than comparison operator.
     * @param other The other fraction to compare.
     * @return True if this fraction is less than the other, false otherwise.
     */
    [[nodiscard]]
    constexpr bool operator<(const Fraction& other) const {
        double lhs = static_cast<double>(m_numerator) / m_denominator;
        double rhs = static_cast<double>(other.m_numerator) / other.m_denominator;
        return lhs < rhs;
    }

    /**
     * @brief Less-than-or-equal-to comparison operator.
     * @param other The other fraction to compare.
     * @return True if this fraction is less than or equal to the other, false otherwise.
     */
    [[nodiscard]]
    constexpr bool operator<=(const Fraction& other) const {
        return *this < other || *this == other;
    }

    /**
     * @brief Greater-than comparison operator.
     * @param other The other fraction to compare.
     * @return True if this fraction is greater than the other, false otherwise.
     */
    [[nodiscard]]
    constexpr bool operator>(const Fraction& other) const {
        return !(*this <= other);
    }

    /**
     * @brief Greater-than-or-equal-to comparison operator.
     * @param other The other fraction to compare.
     * @return True if this fraction is greater than or equal to the other, false otherwise.
     */
    [[nodiscard]]
    constexpr bool operator>=(const Fraction& other) const {
        return !(*this < other);
    }

    /**
     * @brief Checks if the fraction is nonzero.
     * @return True if the fraction is nonzero, false otherwise.
     */
    constexpr explicit operator bool() const noexcept {
        return m_numerator != 0;
    }

    /**
     * @brief Stream output operator.
     * @param os The output stream.
     * @param frac The fraction to output.
     * @return A reference to the output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const Fraction& frac) {
        os << frac.m_numerator;
        if (frac.m_denominator != 1) {
            os << "/" << frac.m_denominator;
        }
        return os;
    }

    /**
     * @brief Stream input operator.
     * @param is The input stream.
     * @param frac The fraction to populate.
     * @return A reference to the input stream.
     * @throws std::invalid_argument if the input format is invalid or the m_denominator is zero.
     */
    friend std::istream& operator>>(std::istream& is, Fraction& frac) {
        int num, den;
        char sep;
        is >> num >> sep >> den;
        if (sep != '/' || den == 0) {
            throw std::invalid_argument("Invalid fraction format or zero m_denominator.");
        }
        frac = Fraction(num, den);
        return is;
    }
};

/// @brief Free-standing abs function for Fraction
constexpr Fraction abs(const Fraction& v) noexcept
{
    return v.abs();
}

} // namespace util
} // namespace musx

#ifndef DOXYGEN_SHOULD_IGNORE_THIS
#include <limits>

namespace std {
template <>
class numeric_limits<musx::util::Fraction> {
public:
    static constexpr bool is_specialized = true;

    // Smallest positive normalized value (not necessarily lowest)
    static constexpr musx::util::Fraction min() noexcept {
        return musx::util::Fraction(1, (std::numeric_limits<int>::max)());
    }

    // Largest representable positive fraction
    static constexpr musx::util::Fraction max() noexcept {
        return musx::util::Fraction((std::numeric_limits<int>::max)());
    }

    // Most negative representable fraction
    static constexpr musx::util::Fraction lowest() noexcept {
        return musx::util::Fraction(std::numeric_limits<int>::lowest());
    }

    static constexpr int digits    = std::numeric_limits<int>::digits;
    static constexpr int digits10  = std::numeric_limits<int>::digits10;

    static constexpr bool is_signed      = true;
    static constexpr bool is_integer     = false;
    static constexpr bool is_exact       = true;
    static constexpr bool has_infinity   = false;
    static constexpr bool has_quiet_NaN  = false;
    static constexpr bool has_signaling_NaN = false;

    static constexpr musx::util::Fraction epsilon() noexcept {
        return musx::util::Fraction(1, (std::numeric_limits<int>::max)());
    }

    static constexpr musx::util::Fraction round_error() noexcept {
        return musx::util::Fraction(0);
    }

    static constexpr int radix = 2;

    static constexpr musx::util::Fraction infinity() noexcept { return musx::util::Fraction(0); }
    static musx::util::Fraction quiet_NaN() noexcept { return musx::util::Fraction(0); }
    static musx::util::Fraction signaling_NaN() noexcept { return musx::util::Fraction(0); }

    static constexpr bool is_iec559       = false;
    static constexpr bool is_bounded      = true;
    static constexpr bool is_modulo       = false;
    static constexpr bool traps           = true;  // Because invalid construction throws
    static constexpr bool tinyness_before = false;
    static constexpr float_round_style round_style = round_indeterminate;
};

template <>
struct hash<musx::util::Fraction>
{
    size_t operator()(const musx::util::Fraction& frac) const noexcept
    {
        // boost algorithm tailored to Fraction
        size_t seed = std::hash<int>{}(frac.numerator());
        seed ^= std::hash<int>{}(frac.denominator()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};
} // namespace std

#endif // DOXYGEN_SHOULD_IGNORE_THIS
