/*
 * Copyright (C) 2026, Robert Patterson
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

#include <memory>
#include <vector>

#include "musx/dom/Entries.h"

namespace musx::util {

/// @brief Stateless interpretation utilities for cue material and its visibility.
class Cue
{
public:
    /// @brief Describes why an entry is or is not visible in a score or linked-part context.
    enum class EntryVisibility
    {
        Visible,                    ///< The entry is visible in the target context.
        HiddenEntry,                ///< The stored #dom::Entry::isHidden flag is set.
        HiddenStaff,                ///< The staff is absent from the context or is force-hidden.
        HiddenByAlternateNotation,  ///< Alternate notation hides entries in this layer.
        ExcludedByVoicing           ///< The target part's voicing excludes this layer.
    };

    /// @brief Cue classification and visibility information for an entry.
    struct EntryAnalysis
    {
        /// @brief Source-level cue classification for a reduced entry.
        enum class Kind
        {
            None,           ///< The entry is not cue material.
            PartOnly,       ///< The reduced entry is hidden in the score and visible in at least one linked part.
            ScoreVisible    ///< The reduced entry is visible in the score.
        };

        Kind kind{Kind::None};              ///< Source-level cue classification.
        int entrySizePercent{dom::FULL_SIZE_PERCENT}; ///< Effective entry size in the requested context.
        bool visibleInRequestedContext{};   ///< Whether the entry is visible in its EntryFrame's requested context.
        bool visibleInScore{};              ///< Whether the entry is visible in the score.
        bool visibleInAnyLinkedPart{};      ///< Whether the entry is visible in at least one linked part.

        /// @brief Returns whether the entry is classified as cue material.
        [[nodiscard]] bool isCue() const noexcept
        { return kind != Kind::None; }
    };

    /// @brief Cue classification and requested-context visibility for an entry frame.
    struct FrameAnalysis
    {
        bool isCueFrame{};                    ///< True if every applicable source entry is cue material.
        bool hasVisibleCueEntries{};           ///< True if any cue entry is visible in the requested context.
        std::vector<size_t> cueEntryIndices;   ///< Indices of entries classified as cue material.
    };

    /// @brief Cue-layer analysis for one staff in one measure and its requested context.
    struct StaffMeasureAnalysis
    {
        std::vector<dom::LayerIndex> cueLayers;        ///< Layers whose frames are cue frames.
        std::vector<dom::LayerIndex> visibleCueLayers; ///< Cue-frame layers with visible cue entries in this context.
        bool isCueOnly{};                              ///< True if all significant layers are cue frames and at least one exists.

        /// @brief Allows clean Boolean checks of cue-only status.
        explicit operator bool() const
        { return isCueOnly; }
    };

    /// @brief Calculates why @p entry is visible or hidden in @p targetPartId.
    /// @note This checks the stored #dom::Entry::isHidden flag. It does not apply iterator-specific
    /// effective-hidden workarounds from #dom::EntryInfoPtr::InterpretedIterator::getEffectiveHidden.
    [[nodiscard]]
    static EntryVisibility calcVisibility(const dom::EntryInfoPtr& entry, dom::Cmper targetPartId);

    /// @brief Calculates visibility in the entry frame's requested score or linked-part context.
    [[nodiscard]]
    static EntryVisibility calcVisibilityInRequestedContext(const dom::EntryInfoPtr& entry);

    /// @brief Returns whether the entry is visible in its requested context.
    [[nodiscard]]
    static bool calcIsVisibleInRequestedContext(const dom::EntryInfoPtr& entry)
    { return calcVisibilityInRequestedContext(entry) == EntryVisibility::Visible; }

    /// @brief Calculates source-level cue classification separately from visibility in relevant contexts.
    [[nodiscard]]
    static EntryAnalysis calcEntryAnalysis(const dom::EntryInfoPtr& entry);

    /// @brief Calculates cue classification and requested-context visibility for @p frame.
    /// Explicitly hidden source entries do not disqualify the frame.
    [[nodiscard]]
    static FrameAnalysis calcFrameAnalysis(const std::shared_ptr<const dom::EntryFrame>& frame);

    /// @brief Calculates semantic and requested-context-visible cue layers for one staff and measure.
    /// Layers containing only a possible full-measure rest and layers excluded by part-voicing policy are ignored.
    [[nodiscard]]
    static StaffMeasureAnalysis calcStaffMeasureAnalysis(const dom::details::GFrameHoldContext& context);
};

} // namespace musx::util
