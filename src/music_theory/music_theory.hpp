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

 // Do not use `#pragma once` here, because the file may be included in multiple projects
#ifndef MUSIC_THEORY_HPP
#define MUSIC_THEORY_HPP

#include <array>
#include <cassert>
#include <vector>
#include <cmath>
#include <algorithm>
#include <optional>
#include <stdexcept>
#include <string>

/*
This header-only library has no dependencies and can be shared into any other library merely
by including it.
*/

/// @namespace music_theory
/// @brief A dependency-free, header-only collection of useful functions for music theory
namespace music_theory {

constexpr int STANDARD_NUMBER_OF_STAFFLINES = 5; ///< The standard number of lines on a staff.
constexpr int STANDARD_DIATONIC_STEPS = 7; ///< currently this is the only supported number of diatonic steps.
constexpr int STANDARD_12EDO_STEPS = 12;   ///< this can be overriden when constructing a @ref Transposer instance.

constexpr std::array<int, STANDARD_DIATONIC_STEPS> MAJOR_KEYMAP = { 0, 2, 4, 5, 7, 9, 11 };     ///< keymap for 12-EDO major keys
constexpr std::array<int, STANDARD_DIATONIC_STEPS> MINOR_KEYMAP = { 0, 2, 3, 5, 7, 8, 10 };     ///< keymap for 12-EDO minor keys

/// @brief Array of chromatic intervals. Each member array contains
///     - the number of fifths to add, which is also the key signature adjustment for the interval
///     - the number of octaves to subtract
constexpr std::array<std::array<int, 2>, STANDARD_DIATONIC_STEPS> DIATONIC_INTERVAL_ADJUSTMENTS = { {
    { 0,  0 },  // unison
    { 2, -1 },  // second
    { 4, -2 },  // third
    {-1,  1 },  // fourth
    { 1,  0 },  // fifth
    { 3, -1 },  // sixth
    { 5, -2 }   // seventh
}};

/// @brief The available note names in array order.
enum class NoteName : int
{
    C = 0,
    D = 1,
    E = 2,
    F = 3,
    G = 4,
    A = 5,
    B = 6
};

/**
 * @brief A spelled pitch, expressed relative to C4.
 *
 * The alteration is relative to the natural note name, in EDO divisions.
 * This type does not encode a key signature or scale degree.
 */
struct Pitch
{
    /// @brief Creates an unspecified pitch.
    constexpr Pitch() = default;

    /// @brief Creates a spelled pitch.
    /// @param pitchName The diatonic note name.
    /// @param pitchOctave The octave number, where C4 is middle C.
    /// @param pitchAlteration The alteration relative to the natural note name, in EDO divisions.
    constexpr Pitch(NoteName pitchName, int pitchOctave, int pitchAlteration = 0)
        : noteName(pitchName), octave(pitchOctave), alteration(pitchAlteration)
    {
    }

    NoteName noteName{}; ///< The diatonic note name.
    int octave{};        ///< The octave number, where C4 is middle C.
    int alteration{};    ///< The alteration relative to the natural note name, in EDO divisions.
};

static constexpr std::array<music_theory::NoteName, music_theory::STANDARD_DIATONIC_STEPS> noteNames = {
    NoteName::C, NoteName::D, NoteName::E, NoteName::F, NoteName::G, NoteName::A, NoteName::B
};

/**
 * @brief Returns the uppercase letter that names a diatonic note, such as 'C' for #NoteName::C.
 * @param noteName The note name.
 * @return The letter.
 * @throws std::invalid_argument if @p noteName is not a value of the enumeration. (In constant evaluation this is a compile error.)
 */
constexpr char calcNoteNameLetter(NoteName noteName)
{
    switch (noteName) {
    case NoteName::C: return 'C';
    case NoteName::D: return 'D';
    case NoteName::E: return 'E';
    case NoteName::F: return 'F';
    case NoteName::G: return 'G';
    case NoteName::A: return 'A';
    case NoteName::B: return 'B';
    }
    assert(false);
    throw std::invalid_argument("invalid NoteName value " + std::to_string(int(noteName)));
}

/// @enum DiatonicMode
/// @brief Represents the seven standard diatonic musical modes.
///
/// Values correspond to their roots with no sharps or flats C (0) through B (6).
enum class DiatonicMode : int
{
    Ionian = 0,         ///< major
    Dorian = 1,         ///< minor with raised 6
    Phrygian = 2,       ///< minor with flat 2
    Lydian = 3,         ///< major with raised 4
    Mixolydian = 4,     ///< major with flat 7
    Aeolian = 5,        ///< natural minor
    Locrian = 6         ///< diminished with flat 2 and 5
};

/// @enum ClefType
/// @brief Represents the possible types of clef, irrespective of octave transposition.
enum class ClefType
{
    Unknown,            ///< Unknown clef type (default value with {} initializer)
    G,                  ///< Treble clef
    C,                  ///< C clef
    F,                  ///< Bass clef
    Percussion1,        ///< 2 thick vertical lines centered on middle staff line (corresponds to SMuFL glyph `unpitchedPercussionClef1`)
    Percussion2,        ///< Narrow rectangle centered on middle staff line (corresponds to SMuFL glyph `unpitchedPercussionClef2`)
    Tab,                ///< Tablature clef (TAB) with non-serif font
    TabSerif            ///< Tablature clef (TAB) with serif font
};

/// @brief Calculates the displacement value for a spelled pitch.
/// @param pitch The pitch to convert.
/// @return A displacement value that can be used to create a @ref Transposer instance.
constexpr int calcDisplacement(const Pitch& pitch)
{
    int pitchClassVal = int(pitch.noteName) % STANDARD_DIATONIC_STEPS;
    const int relativeOctave = pitch.octave - 4;

    return pitchClassVal + (STANDARD_DIATONIC_STEPS * relativeOctave);
}

/// @brief Calculates the sign of an integer.
/// @param n The integer to process.
/// @return -1 for negative; 1 for non-negative.
template <typename T>
constexpr T sign(T n)
{
    static_assert(std::is_arithmetic_v<T>, "sign requires a numeric type");
    return n < T(0) ? T(-1) : T(1);
}

/// @brief Calculates the modulus of positive and negative numbers in a predictable manner.
/// @tparam T an integer type
/// @param n The number (positive or negative) for which to calculate the modulus
/// @param d The base of the modulus. This must be a positive number.
/// @return For non-negative, the result is the same as `%`. For negative numbers, the result is `-1 * (abs(n) % d)`.
/// @note Since C++11 integer division is guaranteed to truncate toward zero, which makes `%` already
/// carry the sign of @p n. Spelling this out with `abs` instead would defeat constant evaluation on any
/// standard library that has not yet made `std::abs` constexpr, and is undefined for the most negative
/// value of @p T.
template <typename T>
constexpr T signedModulus(T n, T d)
{
    static_assert(std::is_integral_v<T>, "signedModulus requires an integer type");
    return n % d;
}

/// @brief Calculates a positive modulus in the range [0, d-1], even for negative dividends.
/// @tparam T An integer type.
/// @param n The dividend (may be negative).
/// @param d The modulus base (must be positive).
/// @param q Optional pointer to receive the quotient such that n = d * q + result.
/// @return The result of n modulo d in the range [0, d-1].
template <typename T>
constexpr T positiveModulus(T n, T d, T* q = nullptr)
{
    static_assert(std::is_integral_v<T>, "positiveModulus requires an integer type");
    if (q) *q = n / d;
    T result = signedModulus(n, d);
    if (result < 0) {
        result += d;
        if (q) --(*q);
    }
    return result;
}

/// @brief Calculates the pitch class of a spelled note name.
/// @details The pitch class is the note's chromatic position within one octave, counting from the
/// natural note name and then applying the alteration. Enharmonics share a pitch class, so C♯ and D♭
/// both return 1 in 12-EDO.
///
/// @p keyMap gives the EDO division of each natural note name in C-first order, which is the same
/// thing as the major scale rooted at C. It cannot be derived from @p numberOfEdoDivisions alone:
/// 31-EDO places the naturals at `{ 0, 5, 10, 13, 18, 23, 28 }`, so both values are needed, exactly
/// as @ref Transposer takes them. Note that a key signature's own key map is relative to its tonal
/// center, so it suits @ref Transposer rather than this function.
/// @param noteName The diatonic note name.
/// @param alteration The alteration relative to the natural note name, in EDO divisions.
/// @param numberOfEdoDivisions The number of divisions in the EDO. (E.g., 31-EDO would pass 31.)
/// @param keyMap A 7-element map giving the EDO division of each natural note name, C first.
/// @return The pitch class, in the range [0, @p numberOfEdoDivisions - 1].
constexpr int calcPitchClass(NoteName noteName, int alteration = 0,
    int numberOfEdoDivisions = STANDARD_12EDO_STEPS,
    const std::array<int, STANDARD_DIATONIC_STEPS>& keyMap = MAJOR_KEYMAP)
{
    const int natural = keyMap[positiveModulus(int(noteName), STANDARD_DIATONIC_STEPS)];
    return positiveModulus(natural + alteration, numberOfEdoDivisions);
}

/// @brief Calculates the pitch class of a spelled pitch, ignoring its octave.
/// @param pitch The pitch to convert. Its alteration is relative to the natural note name.
/// @param numberOfEdoDivisions The number of divisions in the EDO. (E.g., 31-EDO would pass 31.)
/// @param keyMap A 7-element map giving the EDO division of each natural note name, C first.
/// @return The pitch class, in the range [0, @p numberOfEdoDivisions - 1].
constexpr int calcPitchClass(const Pitch& pitch,
    int numberOfEdoDivisions = STANDARD_12EDO_STEPS,
    const std::array<int, STANDARD_DIATONIC_STEPS>& keyMap = MAJOR_KEYMAP)
{
    return calcPitchClass(pitch.noteName, pitch.alteration, numberOfEdoDivisions, keyMap);
}

/// @brief Calculates the number of 12-EDO chromatic halfsteps in the specified interval
/// @param interval              The diatonic displacement (negative for downward intervals).
/// @param chromaticAlteration   The chromatic halfstep alteration that defines the chromatic interval.
/// @return The number of 12-EDO divisions (chromatic halfsteps) in the interval (negative means down)
constexpr int calc12EdoHalfstepsInInterval(int interval, int chromaticAlteration)
{
    int octaves{};
    int diatonic = positiveModulus(interval, STANDARD_DIATONIC_STEPS, &octaves);
    return MAJOR_KEYMAP[diatonic] + (octaves * STANDARD_12EDO_STEPS) + chromaticAlteration;
}

/// @brief Calculates the alteration in chromatic halfsteps for the specified interval/halfsteps combination
/// @param interval         The diatonic displacement (negative for downward transposition).
/// @param halfsteps        The number of 12-EDO chromatic halfsteps in the interval (negative means down).
/// @return The alteration value that, with @p interval, defines the chromatic interval. This return value can be used to initialize a @ref Transposer.
constexpr int calcAlterationFrom12EdoHalfsteps(int interval, int halfsteps)
{
    int octaves{};
    int diatonic = positiveModulus(interval, STANDARD_DIATONIC_STEPS, &octaves);
    int expectedHalfsteps = MAJOR_KEYMAP[diatonic] + (octaves * STANDARD_12EDO_STEPS);
    return halfsteps - expectedHalfsteps;
}

/// @brief Determines the chromatic alteration needed for a diatonic interval to produce a desired key signature change.
/// @param interval            The diatonic interval (e.g. +3 for a perfect fourth up).
/// @param keySigChange        The desired change in key signature (positive for sharps added or flats removed, negative for flats added or sharps removed).
/// @return The chromatic alteration in halfsteps required to produce that key signature change with the given diatonic interval.
constexpr int calcAlterationFromKeySigChange(int interval, int keySigChange)
{
    int diatonic = positiveModulus(interval, STANDARD_DIATONIC_STEPS);
    int expectedKeyChange = DIATONIC_INTERVAL_ADJUSTMENTS[diatonic][0];
    if (interval < 0) {
        if (std::abs(expectedKeyChange) > 1) { // imperfect intervals
            expectedKeyChange -= STANDARD_DIATONIC_STEPS;
        }
    }
    int alteration = (keySigChange - expectedKeyChange) / STANDARD_DIATONIC_STEPS;
    return alteration;
}

/// @brief Calculates the resulting key signature change (sharps/flats) produced by a diatonic interval
///        and chromatic alteration.
/// @param interval            The diatonic displacement (e.g. +3 for a perfect fourth up).
/// @param chromaticAlteration The chromatic alteration in halfsteps that defines the chromatic interval.
/// @return The change in key signature (positive for sharps added or flats removed,
///         negative for flats added or sharps removed).
constexpr int calcKeySigChangeFromInterval(int interval, int chromaticAlteration)
{
    const int diatonic = positiveModulus(interval, STANDARD_DIATONIC_STEPS);
    int expectedKeyChange = DIATONIC_INTERVAL_ADJUSTMENTS[diatonic][0];
    if (interval < 0) {
        if (std::abs(expectedKeyChange) > 1) { // imperfect intervals
            expectedKeyChange -= STANDARD_DIATONIC_STEPS;
        }
    }
    return expectedKeyChange + (chromaticAlteration * STANDARD_DIATONIC_STEPS);
}

/// @brief Determines if the transposition values result in trasposing by one or more octaves.
/// @param displacement The diatonic displacement.
/// @param alteration The chromatic alteration.
constexpr bool calcTranspositionIsOctave(int displacement, int alteration)
{
    return (displacement % STANDARD_DIATONIC_STEPS) == 0 && alteration == 0;
}

/// @class Transposer
/// @brief Provides dependency-free transposition utilities that work with any scale that has 7 diatonic steps
/// and an equal number of divisions of the octave (EDO). The most common Western scales use 12 divisions of the octave,
/// .i.e., 12-EDO.
///
/// The way to use this class is to intialize it in the constructor, run one or more transposition functions,
/// then extract the modified displacement and alteration values.
class Transposer
{
private:
    int m_displacement;
    int m_alteration;               // alteration from key signature
    int m_numberOfEdoDivisions;     // number of divisions in the EDO (default 12)
    std::vector<int> m_keyMap;      // step map for the EDO

public:
    /// @brief Constructs a 12-EDO major-scale transposer for a spelled pitch.
    /// @param pitch The pitch to transpose. Its alteration is relative to the natural note name.
    explicit Transposer(const Pitch& pitch)
        : Transposer(calcDisplacement(pitch), pitch.alteration)
    {
    }

    /// @brief Constructor function
    /// @param displacement     the scale step displacement value. 0 signifies the tonic in the C4 (middle-C) octave.
    /// With the default values @p isMinor = `false` and @p keyMap = `std::nullopt`, 0 is C4.
    /// @param alteration       the number of EDO divisions by which the pitch is altered. This corresponds to the accidental on a note.
    /// 0 signifies that the note has no accidental in the key signature.
    /// @param isMinor          true if you wish to use the default keymap for minor mode. (Ignored if @p keyMap is provided.)
    /// @param numberOfEdoDivisions the number of divisions in the EDO. (E.g., 31-EDO would pass 31.)
    /// @param keyMap           a 7-element map specifying the starting EDO division of each diatonic step. 12-EDO Major (the default) is { 0, 2, 4, 5, 7, 9, 11 }.
    Transposer(int displacement, int alteration,
        bool isMinor = false, int numberOfEdoDivisions = STANDARD_12EDO_STEPS,
        const std::optional <std::vector<int>>& keyMap = std::nullopt)
        : m_displacement(displacement), m_alteration(alteration), m_numberOfEdoDivisions(numberOfEdoDivisions)
    {
        if (keyMap) {
            if (keyMap.value().size() != STANDARD_DIATONIC_STEPS) {
                throw std::invalid_argument("The Transposer class only supports key map arrays of " + std::to_string(STANDARD_DIATONIC_STEPS) + " elements");
            }
            m_keyMap = keyMap.value();
        } else if (isMinor) {
            m_keyMap.assign(MINOR_KEYMAP.begin(), MINOR_KEYMAP.end());
        } else {
            m_keyMap.assign(MAJOR_KEYMAP.begin(), MAJOR_KEYMAP.end());
        }
    }

    /// @brief Return the current displacement value
    int displacement() const { return m_displacement; }

    /// @brief Return the current chromatic alteration value
    int alteration() const { return m_alteration; }

    /// @brief Transposes the displacement by the specified interval.
    /// @param interval 0 = unison, 1 = second, 2 = third, etc. The interval may be any size, and it can be negative for downward transposition.
    void diatonicTranspose(int interval)
    {
        m_displacement += interval;
    }

    /// @brief Transposes enharmonically relative to the current values.
    /// @param diatonicSteps number of diatonic steps to move; may be positive or negative.
    void enharmonicTranspose(int diatonicSteps)
    {
        const int stepSign = sign(diatonicSteps);
        for (int i = 0; i < std::abs(diatonicSteps); ++i) {
            const int keyStepEnharmonic = calcStepsBetweenScaleDegrees(m_displacement, m_displacement + stepSign);
            diatonicTranspose(stepSign);
            m_alteration -= stepSign * keyStepEnharmonic;
        }
    }

    /**
     * @brief Chromatically transposes by a specified chromatic interval.
     *
     * This function transposes the note chromatically in any EDO. The chromatic interval is defined by a diatonic
     * displacement (e.g., 0 = unison, 1 = second, 2 = third, etc.) plus a chromatic alteration.
     *
     * @note Chromatic alterations have the same values in any EDO. That is, the inputs to this functions are
     * the same regardless of EDO number. This function calculates the correct number of EDO divisions in a
     * chromatic halfstep using the key map provided when the Transposer instance was created.
     *
     * A chromatic alteration of 0 corresponds to major or perfect intervals. For example:
     * - `{1,  0}` is a major second up
     * - `{2, -1}` is a minor third up
     * - `{3,  0}` is a perfect fourth up
     * - `{5,  1}` is an augmented sixth up
     *
     * Reversing the signs of both `interval` and `alteration` allows for downward transposition.
     *
     * @param interval              The diatonic displacement (negative for downward transposition).
     * @param chromaticAlteration   The chromatic halfstep alteration that defines the chromatic interval.
     */
    void chromaticTranspose(int interval, int chromaticAlteration)
    {
        const int intervalNormalized = signedModulus(interval, STANDARD_DIATONIC_STEPS);
        const int stepsInAlteration = calcStepsInAlteration(interval, chromaticAlteration);
        const int stepsInInterval = calcStepsInNormalizedInterval(intervalNormalized);
        const int stepsInDiatonicInterval = calcStepsBetweenScaleDegrees(m_displacement, m_displacement + intervalNormalized);

        const int effectiveAlteration = stepsInAlteration + stepsInInterval - sign(interval) * stepsInDiatonicInterval;

        diatonicTranspose(interval);
        m_alteration += effectiveAlteration;
    }

    /**
     * @brief Simplifies the spelling by reducing its alteration while preserving pitch.
     *
     * This is typically used after a chromatic or enharmonic transposition to improve a note’s readability.
     * It performs enharmonic transpositions in the appropriate direction until the absolute alteration is minimized.
     */
    void simplifySpelling()
    {
        while (std::abs(m_alteration) > 0) {
            const int currSign = sign(m_alteration);
            const int currAbsDisp = std::abs(m_alteration);
            enharmonicTranspose(currSign);
            if (std::abs(m_alteration) >= currAbsDisp) {
                enharmonicTranspose(-currSign);
                return;
            }
            if (currSign != sign(m_alteration)) {
                break;
            }
        }
    }

    /**
     * @brief Transposes by the given number of EDO divisions and simplifies the spelling.
     *
     * In standard key signatures, each division corresponds to a halfstep.
     *
     * After transposition, the note's spelling is simplified to reduce the alteration while preserving pitch.
     *
     * @param numberOfEdoDivisions The number of EDO divisions to transpose (positive for up, negative for down).
     */
    void stepwiseTranspose(int numberOfEdoDivisions)
    {
        m_alteration += numberOfEdoDivisions;
        simplifySpelling();
    }

    /**
     * @brief Determines if the given displacement and alteration refer to the same pitch as the current state.
     *
     * This compares the total EDO division position derived from the current displacement and alteration
     * with that of the input values. It accounts for microtonal tuning via the custom key map and EDO size.
     *
     * @param displacement The scale step displacement to compare.
     * @param alteration   The alteration (accidental specified in EDO divisions) to compare.
     * @return true if the given values are enharmonically equivalent to the current values.
     */
    bool isEnharmonicEquivalent(int displacement, int alteration) const {
        return calcAbsoluteDivision(displacement, alteration) == calcAbsoluteDivision(m_displacement, m_alteration);
    }

private:
    int calcFifthSteps() const
    {
        // std::log(3.0 / 2.0) / std::log(2.0) is 0.5849625007211562.
        static constexpr double kFifthsMultiplier = 0.5849625007211562;
        return static_cast<int>(std::floor(m_numberOfEdoDivisions * kFifthsMultiplier) + 0.5);
    }

    int calcScaleDegree(int interval) const
    { return positiveModulus(interval, int(m_keyMap.size())); }

    int calcStepsBetweenScaleDegrees(int firstDisplacement, int secondDisplacement) const
    {
        const int firstScaleDegree = calcScaleDegree(firstDisplacement);
        const int secondScaleDegree = calcScaleDegree(secondDisplacement);
        int result = sign(secondDisplacement - firstDisplacement) * (m_keyMap[secondScaleDegree] - m_keyMap[firstScaleDegree]);
        if (result < 0) {
            result += m_numberOfEdoDivisions;
        }
        return result;
    }

    int calcStepsInAlteration(int interval, int alteration) const
    {
        const int fifthSteps = calcFifthSteps();
        const int plusFifths = sign(interval) * alteration * 7;     // number of fifths to add for a chromatic halfstep alteration (in any EDO)
        const int minusOctaves = sign(interval) * alteration * -4;  // number of octaves to subtract for a chromatic halfstep alteration (in any EDO)
        const int result = sign(interval) * ((plusFifths * fifthSteps) + (minusOctaves * m_numberOfEdoDivisions));
        return result;
    }

    int calcStepsInNormalizedInterval(int intervalNormalized) const
    {
        const int fifthSteps = calcFifthSteps();
        const int index = std::abs(intervalNormalized);
        const int plusFifths = DIATONIC_INTERVAL_ADJUSTMENTS[index][0];    // number of fifths
        const int minusOctaves = DIATONIC_INTERVAL_ADJUSTMENTS[index][1];  // number of octaves

        return sign(intervalNormalized) * ((plusFifths * fifthSteps) + (minusOctaves * m_numberOfEdoDivisions));
    }

    int calcAbsoluteDivision(int displacement, int alteration) const {
        const int scaleDegree = calcScaleDegree(displacement); // 0..6
        const int baseStep = m_keyMap[scaleDegree];

        const int octaveCount = (displacement < 0 && displacement % STANDARD_DIATONIC_STEPS != 0)
                              ? (displacement / STANDARD_DIATONIC_STEPS) - 1
                              : (displacement / STANDARD_DIATONIC_STEPS);
        const int octaveSteps = octaveCount * m_numberOfEdoDivisions;
        const int chromaticSteps = calcStepsInAlteration(/*interval=*/+1, alteration);

        return baseStep + chromaticSteps + octaveSteps;
    }
};

} // namespace music_theory

#endif // MUSIC_THEORY_HPP
