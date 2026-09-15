/*
 * Copyright (C) 2024, Robert Patterson
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
#include <cstdint>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "Header.h"
#include "Instrument.h"
#include "ObjectPool.h"
#include "MusxInstance.h"

/**
 * @namespace musx
 * @brief object model for musx file (enigmaxml)
 */
namespace musx {

namespace factory {
class DocumentFactory;
} // namespace factory

/**
 * @namespace musx::dom
 * @brief classes representing the musx document
 */
namespace dom {

using namespace header;

namespace others {
class MeasureExprAssign;
}

namespace texts {
class FileInfoText;
class LyricsTextBase;
}

enum class KnownShapeDefType;
using EmbeddedGraphicBlob = std::vector<uint8_t>; ///< Raw bytes for one embedded graphic payload from a musx archive.

/// @brief Embedded graphic payload from a musx archive entry.
struct EmbeddedGraphicData
{
    std::string extension;          ///< File extension without the leading dot (e.g. "png", "jpg", "svg").
    EmbeddedGraphicBlob bytes;      ///< Raw file bytes.
};

using EmbeddedGraphicsMap = std::unordered_map<Cmper, EmbeddedGraphicData>; ///< Embedded graphics keyed by cmper (filename stem in musx archive).

/// @enum PartVoicingPolicy
/// @brief Controls whether Finale-style part voicing is applied when iterating entries via
///        @ref musx::dom::details::GFrameHoldContext.
enum class PartVoicingPolicy
{
    Ignore,     ///< Default. Part voicing is ignored entirely. All layers in a staff are visible to all parts,
                ///< and entries behave as if no part-voicing definitions exist.

    Apply       ///< Apply Finale’s voiced-part interpretation.
                ///< When enabled, entry iteration filters layers according to the active part voicing,
                ///< and individual notes may be included or excluded based on voicing rules.
                ///< Callers that operate at the note level should use
                ///< #NoteInfoPtr::calcIsIncludedInVoicing to test note visibility.
                ///< If all notes in an entry are excluded by voicing, then
                ///< #EntryInfoPtr::calcDisplaysAsRest returns true for that entry.
};

/// @brief Key identifying a rehearsal mark by measure number and @ref others::TextExpressionDef @ref Cmper.
using RehearsalMarkKey = std::pair<MeasCmper, Cmper>;

/// @brief Information associated with a rehearsal mark.
struct RehearsalMarkInfo
{
    int rehearsalSequence{};  ///< 1-based rehearsal mark sequence number for this measure.
};

/// @brief Maps each occurrence of a rehearsal mark to its sequence number for that measure.
using RehearsalMarkMap = std::map<RehearsalMarkKey, RehearsalMarkInfo>;

/**
 * @brief Represents a document object that encapsulates the entire EnigmaXML structure.
 */
class Document
{
public:
    /**  @brief Retrieves the header */
    [[nodiscard]]
    HeaderPtr& getHeader() { return m_header; }
    /**  @brief Retrieves the const header */
    [[nodiscard]]
    const HeaderPtr& getHeader() const { return m_header; }

    /** @brief Retrieves the options pool */
    [[nodiscard]]
    OptionsPoolPtr& getOptions() { return m_options; }
    /** @brief Retrieves the const options pool */
    [[nodiscard]]
    const OptionsPoolPtr& getOptions() const { return m_options; }

    /** @brief Retrieves the others pool */
    [[nodiscard]]
    OthersPoolPtr& getOthers() { return m_others; }
    /** @brief Retrieves the const others pool */
    [[nodiscard]]
    const OthersPoolPtr& getOthers() const { return m_others; }

    /** @brief Retrieves the details pool */
    [[nodiscard]]
    DetailsPoolPtr& getDetails() { return m_details; }
    /** @brief Retrieves the const others pool */
    [[nodiscard]]
    const DetailsPoolPtr& getDetails() const { return m_details; }

    /** @brief Retrieves the entry pool */
    [[nodiscard]]
    EntryPoolPtr& getEntries() { return m_entries; }
    /** @brief Retrieves the entry others pool */
    [[nodiscard]]
    const EntryPoolPtr& getEntries() const { return m_entries; }

    /** @brief Retrieves the texts pool */
    [[nodiscard]]
    TextsPoolPtr& getTexts() { return m_texts; }
    /** @brief Retrieves the const texts pool */
    [[nodiscard]]
    const TextsPoolPtr& getTexts() const { return m_texts; }

    /** @brief Retrieves the document's part voicing policy */
    [[nodiscard]]
    PartVoicingPolicy getPartVoicingPolicy() const { return m_partVoicingPolicy; }

    /// @brief Returns the score playback duration in seconds, if provided by NotationMetadata.xml.
    [[nodiscard]]
    std::optional<double> getScoreDurationSeconds() const { return m_scoreDurationSeconds; }

    /// @brief Returns all embedded graphics keyed by graphic cmper.
    [[nodiscard]]
    const EmbeddedGraphicsMap& getEmbeddedGraphics() const { return m_embeddedGraphics; }

    /// @brief Returns the Scroll View Cmper for the given @p partId.
    /// @param partId The linked part to check.
    [[nodiscard]]
    Cmper calcScrollViewCmper(Cmper partId) const noexcept;

    /// @brief Returns the Scroll View staves for the given @p partId.
    /// @param partId The linked part to check.
    [[nodiscard]]
    MusxInstanceList<others::StaffUsed> getScrollViewStaves(Cmper partId) const;

    /// @brief Returns the Studio View staves for the given @p partId. Normally this is
    /// only one staff and is invariant between parts.
    /// @param partId The linked part to check.
    [[nodiscard]]
    MusxInstanceList<others::StaffUsed> getStudioViewStaves(Cmper partId) const;

    /// @brief Retrieves a cached shape recognition result, if available.
    [[nodiscard]]
    std::optional<KnownShapeDefType> getCachedShapeRecognition(Cmper shapeCmper) const;

    /// @brief Stores a shape recognition result in the cache.
    void setCachedShapeRecognition(Cmper shapeCmper, KnownShapeDefType type) const;

    /// @brief Retrieves a cached SMuFL font recognition result, if available.
    [[nodiscard]]
    std::optional<bool> getCachedFontIsSMuFL(Cmper fontId) const;

    /// @brief Stores a SMuFL font recognition result in the cache.
    void setCachedFontIsSMuFL(Cmper fontId, bool isSmufl) const;

    /// @brief Searches pages to find the page that contains the measure.
    /// @return The page, or nullptr if the part's page layout is unavailable or the measure is not found.
    /// @param partId the linked part to search
    /// @param measureId the measure to find
    [[nodiscard]]
    MusxInstance<others::Page> calcPageFromMeasure(Cmper partId, MeasCmper measureId) const;

    /// @brief Searches systems to find the system that contains the measure.
    /// @return The system, or nullptr if the part's page layout is unavailable or the measure is not found.
    /// @param partId the linked part to search
    /// @param measureId the measure to find
    [[nodiscard]]
    MusxInstance<others::StaffSystem> calcSystemFromMeasure(Cmper partId, MeasCmper measureId) const;

    /// @brief Returns the maximum number of blank pages in any part. This is calculated by #factory::DocumentFactory::create.
    [[nodiscard]]
    int getMaxBlankPages() const { return m_maxBlankPages; }

    /// @brief Returns the instrument map for this document. It is computed by the factory.
    [[nodiscard]]
    const InstrumentMap& getInstruments() const
    {
        MUSX_ASSERT_IF(!m_instruments.has_value()) {
            throw std::logic_error("Attempted to retrieve instrument map before it was constructed.");
        }
        return m_instruments.value();
    }

    /// @brief Returns the optional instrument map, which is empty until the map has been constructed.
    /// @details This supports interpretation helpers that can provide a useful fallback while the
    /// document factory is still resolving relationships. Call #getInstruments when the map is required.
    [[nodiscard]]
    const std::optional<InstrumentMap>& getInstrumentsIfAvailable() const noexcept
    {
        return m_instruments;
    }

    /// @brief Returns the path to the musx (or EnigmaXML) file used to create this document, if provided.
    [[nodiscard]]
    const std::optional<std::filesystem::path>& getSourcePath() const { return m_sourcePath; }

    /// @brief Resolve an external graphic path using SCORE_PARTID.
    /// @param fileDescId The FileDescription cmper associated with the graphic.
    /// @return A resolved path if the file can be located, otherwise std::nullopt.
    [[nodiscard]]
    std::optional<std::filesystem::path> resolveExternalGraphicPath(Cmper fileDescId) const;

    /// @brief Get the instrument info for the given staffId
    /// @param staffId The staffId to find.
    [[nodiscard]]
    const InstrumentInfo& getInstrumentForStaff(StaffCmper staffId) const;

    /// @brief Get the instrument info for the given staffId in the specified part.
    /// @param partId The linked part to search.
    /// @param staffId The staffId to find.
    /// @return The instrument info for the given staff, or std::nullopt if the staff is not mapped to an instrument.
    [[nodiscard]]
    std::optional<InstrumentInfo> getInstrumentForStaff(Cmper partId, StaffCmper staffId) const;

    /// @brief Returns the rehearsal mark info for a rehearsal mark text expression assignment.
    /// @return The rehearsal mark info or std::nullopt if none.
    [[nodiscard]]
    std::optional<RehearsalMarkInfo> getRehearsalMarkInfo(MeasCmper measureId, Cmper textExpressionId) const;

    /**
     * @brief Builds an instrument map for the specified linked part ID.
     *
     * This routine detects instrument groupings in three stages:
     * 1. Defined multi-staff instruments (via multiStaffInstId).
     * 2. Visually bracketed staves with matching instrument UUIDs.
     * 3. Remaining single staves as individual instruments.
     *
     * This is especially important for supporting legacy .musx files
     * created before multi-staff instruments were defined explicitly.
     *
     * @param forPartId The linked part for which to create the map.
     */
    [[nodiscard]]
    InstrumentMap createInstrumentMap(Cmper forPartId) const;

    /// @brief Calculate if the current score/part has staves that differ from system to system.
    /// @param forPartId The linked score or part ID to check.
    [[nodiscard]]
    bool calcHasVaryingSystemStaves(Cmper forPartId) const;

    /// @brief Calcuate a @ref MusicRange instance for the entire document.
    [[nodiscard]]
    MusicRange calcEntireDocument() const;

    /// @brief Calculates the measures from which playback jumped into @p currentMeasure.
    /// @details This function inspects repeat endings and text repeats to detect jumps that
    /// land on the specified measure. The result is the measure immediately preceding the
    /// jump origin, when determinable. It is intended for jump-aware tie continuation logic.
    /// @param partId The linked part to inspect (use #SCORE_PARTID for score).
    /// @param currentMeasure The measure that playback has jumped into.
    /// @return A list of measures immediately preceding jump origins, ordered by preference
    /// (forward jump origins first in ascending order, then backward origins in descending order).
    [[nodiscard]]
    std::vector<MeasCmper> calcJumpFromMeasures(Cmper partId, MeasCmper currentMeasure) const;

    /// @brief Iterate all entries in the document by staff and then measure. This function wraps MusxInstanceList<others::StaffUsed>::iterateEntries.
    /// @param partId The linked part id to iterate. (Use #SCORE_PARTID to iterate the score.)
    /// @param iterator The callback function.
    /// @return True if iteration completed. False if the @p iterator returned false and exited early.
    bool iterateEntries(Cmper partId, std::function<bool(const EntryInfoPtr&)> iterator) const;

private:
    /// @brief Constructs a `Document`
    explicit Document() = default;

    DocumentWeakPtr m_self;     ///< A weak pointer to self, to allow constructing common classes inside this.

    HeaderPtr m_header;         ///< The <header>
    OptionsPoolPtr m_options;   ///< The <options> pool
    OthersPoolPtr m_others;     ///< The <others> pool
    DetailsPoolPtr m_details;   ///< The <details> pool
    EntryPoolPtr m_entries;     ///< The <entries> pool
    TextsPoolPtr m_texts;       ///< The <texts> pool

    int m_maxBlankPages{};      ///< The maximum number of leading blank pages in any part.

    std::optional<InstrumentMap> m_instruments = std::nullopt; ///< List of instruments in the document,
                                ///< indexed by the top staff in each instrument in Scroll View of the score.
                                ///< This computed by the factory.

    RehearsalMarkMap m_rehearsalMarks; ///< Map of rehearsal marks in the document.
    void createRehearsalMarkMap();

    PartVoicingPolicy m_partVoicingPolicy{};    ///< The part voicing policy in effect for this document.
    std::optional<double> m_scoreDurationSeconds; ///< Optional score duration in seconds from NotationMetadata.xml.
    EmbeddedGraphicsMap m_embeddedGraphics;     ///< Embedded graphics passed in by the caller (from musx container files).
    std::optional<std::filesystem::path> m_sourcePath; ///< Path to the musx (or EnigmaXML) file used to create this document.

    mutable std::unordered_map<Cmper, KnownShapeDefType> m_shapeRecognitionCache; ///< Cache of ShapeDef recognitions.
    mutable std::unordered_map<Cmper, bool> m_isSmuflFontCache; ///< Cache of SMuFL font recognitions.

    // Grant the factory class access to the private constructor
    friend class musx::factory::DocumentFactory;
};

namespace texts {

template <typename T>
[[nodiscard]]
std::optional<Cmper> importTextInto(const DocumentPtr& target,
    const MusxInstance<T>& source, const ImportObjectCallback& onImported)
{
    static_assert(is_pool_type_v<TextsPool, T>, "Type T is not registered in TextsPool");
    MUSX_ASSERT_IF(!target) {
        throw std::invalid_argument("importTextInto received a null target document");
    }
    MUSX_ASSERT_IF(!source) {
        throw std::invalid_argument("importTextInto received null text");
    }

    std::optional<Cmper> importedId;
    if constexpr (std::is_same_v<T, FileInfoText>) {
        importedId = source->getTextNumber();
        if (target->getTexts()->get<FileInfoText>(*importedId)) {
            return std::nullopt;
        }
    } else {
        importedId = target->getTexts()->nextFreeCmper<T>();
    }
    if (!importedId) {
        return std::nullopt;
    }
    auto imported = std::make_shared<T>(
        target, SCORE_PARTID, EnigmaBase::ShareMode::All, *importedId);
    *imported = *source;
    target->getTexts()->add(T::XmlNodeName, imported);
    if (onImported) {
        onImported(*imported);
    }
    if constexpr (std::is_base_of_v<LyricsTextBase, T>) {
        imported->createSyllableInfo(imported);
    }
    return importedId;
}

template <typename T>
[[nodiscard]]
std::optional<Cmper> importTextInto(const DocumentPtr& target,
    const MusxInstance<T>& source)
{
    return importTextInto(target, source, {});
}

} // namespace texts

} // namespace dom
} // namespace musx
