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
#include "musx/musx.h"
#include <cmath>

using namespace musx::dom;

namespace musx {
namespace util {

namespace {

NoteInfoPtr calcTiedTo(const NoteInfoPtr& noteInfo, bool requireTie)
{
    if (!noteInfo) {
        return {};
    }
    if (requireTie && !noteInfo.calcHasTieStart()) {
        return {};
    }
    auto tiedTo = noteInfo.calcTieTo();
    if (requireTie && tiedTo && !tiedTo.calcHasTieEnd()) {
        return {};
    }
    return tiedTo;
}

constexpr bool isInnerConnectStyle(TieConnectStyleType type) noexcept
{
    switch (type) {
        case TieConnectStyleType::OverStartPosInner:
        case TieConnectStyleType::OverEndPosInner:
        case TieConnectStyleType::UnderStartPosInner:
        case TieConnectStyleType::UnderEndPosInner:
            return true;
        default:
            return false;
    }
}

constexpr bool isOverInnerConnectStyle(TieConnectStyleType type) noexcept
{
    switch (type) {
        case TieConnectStyleType::OverStartPosInner:
        case TieConnectStyleType::OverEndPosInner:
            return true;
        default:
            return false;
    }
}

constexpr bool isOuterNoteConnectStyle(TieConnectStyleType type) noexcept
{
    switch (type) {
        case TieConnectStyleType::OverHighestNoteStartPosOver:
        case TieConnectStyleType::OverHighestNoteEndPosOver:
        case TieConnectStyleType::UnderLowestNoteStartPosUnder:
        case TieConnectStyleType::UnderLowestNoteEndPosUnder:
            return true;
        default:
            return false;
    }
}

constexpr bool isOuterStemConnectStyle(TieConnectStyleType type) noexcept
{
    switch (type) {
        case TieConnectStyleType::OverHighestNoteStemStartPosOver:
        case TieConnectStyleType::OverHighestNoteStemEndPosOver:
        case TieConnectStyleType::UnderLowestNoteStemStartPosUnder:
        case TieConnectStyleType::UnderLowestNoteStemEndPosUnder:
            return true;
        default:
            return false;
    }
}

constexpr bool isOuterConnectStyle(TieConnectStyleType type) noexcept
{
    switch (type) {
        case TieConnectStyleType::OverStartPosInner:
        case TieConnectStyleType::OverEndPosInner:
        case TieConnectStyleType::UnderStartPosInner:
        case TieConnectStyleType::UnderEndPosInner:
            return false;
        default:
            return true;
    }
}

bool calcEntryHasNonAlignedSecond(const EntryInfoPtr& entryInfo)
{
    const auto entry = entryInfo->getEntry();
    for (const auto& note : entry->notes) {
        if (note->upStemSecond || note->downStemSecond) {
            return true;
        }
    }
    return false;
}

bool calcNoteIsNonAlignedSecond(const NoteInfoPtr& noteInfo, bool upStem)
{
    const auto note = noteInfo.operator->();
    return upStem ? note->upStemSecond : note->downStemSecond;
}

unsigned calcEntryDots(const EntryInfoPtr& entryInfo)
{
    return entryInfo->getEntry()->calcDurationInfo().second;
}

bool calcNoteHasAccidental(const NoteInfoPtr& noteInfo)
{
    const auto note = noteInfo.operator->();
    return note->showAcci;
}

bool calcUseAfterSingleDot(const NoteInfoPtr& noteInfo, bool forTieEnd, const NoteInfoPtr& contextNote)
{
    const auto entryFrame = noteInfo.getEntryInfo().getFrame();
    if (auto tieAlter = details::TieAlterBase::fromNoteInfo(contextNote ? contextNote : noteInfo, forTieEnd)) {
        if (tieAlter->afterSingleDotLocal) {
            return tieAlter->afterSingleDotOn;
        }
    }
    if (const auto tieOptions = entryFrame->getDocument()->getOptions()->get<options::TieOptions>()) {
        return tieOptions->afterSingleDot;
    }
    return false;
}

bool calcUseAfterMultipleDots(const NoteInfoPtr& noteInfo, bool forTieEnd, const NoteInfoPtr& contextNote)
{
    const auto entryFrame = noteInfo.getEntryInfo().getFrame();
    if (auto tieAlter = details::TieAlterBase::fromNoteInfo(contextNote ? contextNote : noteInfo, forTieEnd)) {
        if (tieAlter->afterMultiDotsLocal) {
            return tieAlter->afterMultiDotsOn;
        }
    }
    if (const auto tieOptions = entryFrame->getDocument()->getOptions()->get<options::TieOptions>()) {
        return tieOptions->afterMultipleDots;
    }
    return false;
}

bool calcUseBeforeSingleAcci(const NoteInfoPtr& noteInfo, bool forTieEnd, const NoteInfoPtr& contextNote)
{
    const auto entryFrame = noteInfo.getEntryInfo().getFrame();
    if (auto tieAlter = details::TieAlterBase::fromNoteInfo(contextNote ? contextNote : noteInfo, forTieEnd)) {
        if (tieAlter->beforeSingleAcciLocal) {
            return tieAlter->beforeSingleAcciOn;
        }
    }
    if (const auto tieOptions = entryFrame->getDocument()->getOptions()->get<options::TieOptions>()) {
        return tieOptions->beforeAcciSingleNote;
    }
    return false;
}

bool calcShiftForSeconds(const NoteInfoPtr& noteInfo, bool forTieEnd, const NoteInfoPtr& contextNote)
{
    const auto entryFrame = noteInfo.getEntryInfo().getFrame();
    if (auto tieAlter = details::TieAlterBase::fromNoteInfo(contextNote ? contextNote : noteInfo, forTieEnd)) {
        if (tieAlter->shiftForSecondsLocal) {
            return tieAlter->shiftForSecondsOn;
        }
    }
    if (const auto tieOptions = entryFrame->getDocument()->getOptions()->get<options::TieOptions>()) {
        return tieOptions->secondsPlacement == options::TieOptions::SecondsPlacement::ShiftForSeconds;
    }
    return false;
}

bool calcUseOuterPlacement(const NoteInfoPtr& noteInfo, bool forTieEnd, const NoteInfoPtr& contextNote)
{
    const auto entryFrame = noteInfo.getEntryInfo().getFrame();
    if (auto tieAlter = details::TieAlterBase::fromNoteInfo(contextNote ? contextNote : noteInfo, forTieEnd)) {
        if (tieAlter->outerLocal) {
            return tieAlter->outerOn;
        }
    }
    if (const auto tieOptions = entryFrame->getDocument()->getOptions()->get<options::TieOptions>()) {
        return tieOptions->useOuterPlacement;
    }
    return true;
}

bool calcIsEndOfSystem(const NoteInfoPtr& noteInfo, bool forPageView)
{
    const auto entryInfo = noteInfo.getEntryInfo();
    const auto entryFrame = entryInfo.getFrame();
    const auto document = entryFrame->getDocument();

    if (!entryInfo.getNextInLayer()) {
        if (entryInfo.getMeasure() == document->calcEntireDocument().end.measureId) {
            return true;
        }
    }

    if (forPageView) {
        if (auto endNote = calcTiedTo(noteInfo, true)) {
            const auto startSystem = document->calcSystemFromMeasure(entryFrame->getRequestedPartId(), entryInfo.getMeasure());
            const auto endSystem = document->calcSystemFromMeasure(entryFrame->getRequestedPartId(), endNote.getEntryInfo().getMeasure());
            if (startSystem && endSystem && startSystem->getCmper() != endSystem->getCmper()) {
                return true;
            }
        }
    }

    return false;
}

options::TieOptions::ConnectStyle calcConnectStyleOffset(const MusxInstance<options::TieOptions>& tieOptions, TieConnectStyleType connectStyle)
{
    options::TieOptions::ConnectStyle result{};
    if (!tieOptions) {
        return result;
    }
    const auto it = tieOptions->tieConnectStyles.find(connectStyle);
    if (it == tieOptions->tieConnectStyles.end() || !it->second) {
        return result;
    }
    return *it->second;
}

bool directionContextMatches(details::TieAlterBase::DirectionContext context, CurveContourDirection direction)
{
    switch (context) {
        case details::TieAlterBase::DirectionContext::None:
            return true;
        case details::TieAlterBase::DirectionContext::Under:
            return direction == CurveContourDirection::Down;
        case details::TieAlterBase::DirectionContext::Over:
            return direction == CurveContourDirection::Up;
    }
    return false;
}

options::TieOptions::ConnectStyle applyTieAlterEndpointOffsets(const NoteInfoPtr& noteInfo, bool forTieEnd, bool forEndPoint,
    details::TieAlterBase::ConnectionType connectionType, CurveContourDirection direction, const options::TieOptions::ConnectStyle& defaultOffset)
{
    if (!noteInfo) {
        return defaultOffset;
    }
    const auto tieAlter = details::TieAlterBase::fromNoteInfo(noteInfo, forTieEnd);
    if (!tieAlter) {
        return defaultOffset;
    }
    if (forEndPoint) {
        if (!tieAlter->enPtAdjOn || tieAlter->enPtEntCnct != connectionType
            || !directionContextMatches(tieAlter->enPtContext, direction)) {
            return defaultOffset;
        }
        return options::TieOptions::ConnectStyle{tieAlter->xEnd, tieAlter->yEnd};
    }
    if (!tieAlter->stPtAdjOn || tieAlter->stPtEntCnct != connectionType
        || !directionContextMatches(tieAlter->stPtContext, direction)) {
        return defaultOffset;
    }
    return options::TieOptions::ConnectStyle{tieAlter->xStart, tieAlter->yStart};
}

Evpu calcEndpointOffsetX(const NoteInfoPtr& noteInfo, bool forTieEnd, bool forEndPoint,
    Tie::EndPointKind endpointKind, CurveContourDirection direction)
{
    if (endpointKind == Tie::EndPointKind::Notehead) {
        return Tie::calcEndpointOffsets(noteInfo, forTieEnd, forEndPoint, direction, true).offsetX;
    }

    options::TieOptions::ConnectStyle defaultOffset{};
    if (!noteInfo) {
        return defaultOffset.offsetX;
    }
    const auto tieOptions = noteInfo.getEntryInfo().getFrame()->getDocument()->getOptions()->get<options::TieOptions>();
    if (!tieOptions) {
        return defaultOffset.offsetX;
    }
    if (endpointKind == Tie::EndPointKind::SystemBoundary) {
        defaultOffset.offsetX = forEndPoint ? tieOptions->sysBreakRightHAdj : tieOptions->sysBreakLeftHAdj;
    }

    std::optional<details::TieAlterBase::ConnectionType> connectionType;
    if (forEndPoint && endpointKind == Tie::EndPointKind::SystemBoundary && !forTieEnd) {
        connectionType = details::TieAlterBase::ConnectionType::SystemEnd;
    } else if (!forEndPoint && endpointKind == Tie::EndPointKind::SystemBoundary && forTieEnd) {
        connectionType = details::TieAlterBase::ConnectionType::SystemStart;
    } else {
        connectionType = Tie::calcConnectionType(noteInfo, forTieEnd, forEndPoint, true);
    }
    if (!connectionType) {
        return defaultOffset.offsetX;
    }
    return applyTieAlterEndpointOffsets(noteInfo, forTieEnd, forEndPoint, *connectionType, direction, defaultOffset).offsetX;
}

} // namespace

CurveContourDirection Tie::calcDefaultDirection(const dom::NoteInfoPtr& noteInfo, bool forTieEnd)
{
    const auto entryInfo = noteInfo.getEntryInfo();
    const auto entryFrame = entryInfo.getFrame();
    const auto thisNote = noteInfo.operator->();

    if (forTieEnd) {
        if (!noteInfo.calcHasTieEnd()) {
            return CurveContourDirection::Unspecified;
        }
    } else {
        if (!noteInfo.calcHasTieStart()) {
            return CurveContourDirection::Unspecified;
        }
    }

    const auto tieOptions = entryFrame->getDocument()->getOptions()->get<options::TieOptions>();
    size_t noteCount = entryInfo->getEntry()->notes.size();
    const bool upStem = entryInfo.calcUpStem();

    auto tryMixedStemDirection = [&]() -> std::optional<CurveContourDirection> {
        std::optional<bool> adjacentUpStem;
        if (forTieEnd) {
            // There seems to be a "bug" in how Finale (as of Finale 2000) determines mixed-stem values for Tie-Ends.
            // It looks at the stem direction of the immediately preceding entry, even if that entry
            // is not the entry that started the tie. Therefore, do not use TiedFromNote() to
            // get the stem direction.
            if (EntryInfoPtr prevEntry = entryInfo.getPreviousInLayer()) {
                adjacentUpStem = prevEntry.calcUpStem();
            }
        } else {
            if (NoteInfoPtr endNote = noteInfo.calcTieTo()) {
                adjacentUpStem = endNote.getEntryInfo().calcUpStem();
            } else {
                // Finale (as of Finale 2000) has the following observed behavior. When no Tie-To note exists,
                // it determines the mixed stem value based on
                //      1. If the next entry is a rest, the adjStemDir is indeterminate so use stemDir
                //      2. If the next entry is a note with its stem frozen, use it
                //      3. If the next entry floats, but it has a V2Launch, then if EITHER the V1 or
                //              the V2 has a stem in the opposite direction, use it.
                auto nextEntry = entryInfo.getNextInLayer();
                if (nextEntry && !nextEntry.calcDisplaysAsRest()) {
                    auto nextStem = nextEntry.calcUpStem();
                    auto [freezeStem, freezeDir] = nextEntry.calcEntryStemSettings();
                    if (!freezeStem && nextEntry->getEntry()->v2Launch && nextStem == upStem) {
                        nextEntry = nextEntry.getNextInLayer();
                        if (nextEntry) {
                            nextStem = nextEntry.calcUpStem();
                        }
                    }
                    adjacentUpStem = nextStem;
                }
            }
        }
        if (!adjacentUpStem.has_value()) {
            return std::nullopt;
        }
        if (tieOptions->mixedStemDirection == options::TieOptions::MixedStemDirection::OppositeFirst) {
            return std::nullopt;
        }
        if (*adjacentUpStem == upStem) {
            return std::nullopt;
        }
        const bool tieUp = tieOptions->mixedStemDirection == options::TieOptions::MixedStemDirection::Over;
        return tieUp ? CurveContourDirection::Up : CurveContourDirection::Down;
    };

    if (noteCount > 1) {
        const bool opposingSeconds = tieOptions->chordTieDirOpposingSeconds;
        auto applyOpposingSeconds = [&](CurveContourDirection direction) {
            if (!opposingSeconds || direction == CurveContourDirection::Unspecified) {
                return direction;
            }
            if (direction == CurveContourDirection::Up && !thisNote->upStemSecond && thisNote->downStemSecond) {
                return CurveContourDirection::Down;
            }
            if (direction == CurveContourDirection::Down && thisNote->upStemSecond && !thisNote->downStemSecond) {
                return CurveContourDirection::Up;
            }
            return direction;
        };

        // This code depends on observed Finale behavior that the notes are always sorted
        // from lowest-to-highest inside the entry. If Finale's behavior ever changes, this
        // code is screwed.
        const size_t noteIndex = noteInfo.getNoteIndex();

        if (noteIndex == 0) {
            if (auto mixed = tryMixedStemDirection()) {
                return *mixed;
            }
            return CurveContourDirection::Down;
        }
        if (noteIndex + 1 == noteCount) {
            if (auto mixed = tryMixedStemDirection()) {
                return *mixed;
            }
            return CurveContourDirection::Up;
        }

        if (tieOptions->chordTieDirType != options::TieOptions::ChordTieDirType::StemReversal) {
            if (noteIndex < noteCount / 2) {
                return applyOpposingSeconds(CurveContourDirection::Down);
            }
            if (noteIndex >= (noteCount + 1) / 2) {
                return applyOpposingSeconds(CurveContourDirection::Up);
            }
            if (tieOptions->chordTieDirType == options::TieOptions::ChordTieDirType::OutsideInside) {
                return applyOpposingSeconds(upStem ? CurveContourDirection::Down : CurveContourDirection::Up);
            }
        }

        NotePropertiesOptions propertiesOptions;
        propertiesOptions.alwaysUseEntryStaff = true;
        const int staffPos = noteInfo.calcNoteProperties(propertiesOptions).staffPosition;
        const auto staff = entryInfo.createCurrentStaff();
        int stemReversalPos = staff->stemReversal;
        return applyOpposingSeconds((staffPos < stemReversalPos) ? CurveContourDirection::Down : CurveContourDirection::Up);
    }

    if (auto mixed = tryMixedStemDirection()) {
        return *mixed;
    }

    return upStem ? CurveContourDirection::Down : CurveContourDirection::Up;
}

CurveContourDirection Tie::calcEffectiveDirection(const dom::NoteInfoPtr& noteInfo, bool forTieEnd)
{
    if (const auto frozenDir = noteInfo.calcFreezeTieDirection(forTieEnd); frozenDir != CurveContourDirection::Unspecified) {
        return frozenDir;
    }

    const auto entryInfo = noteInfo.getEntryInfo();
    const auto entryFrame = entryInfo.getFrame();
    const auto entry = entryInfo->getEntry();

    if (entry->splitStem) {
        return noteInfo->upSplitStem ? CurveContourDirection::Up : CurveContourDirection::Down;
    }

    if (const auto layerInfo = entryFrame->getLayerAttributes()) {
        if (layerInfo->freezeLayer && entryInfo.calcIfLayerSettingsApply()) {
            const bool tieUp = layerInfo->freezeStemsUp == layerInfo->freezTiesToStems;
            return tieUp ? CurveContourDirection::Up : CurveContourDirection::Down;
        }
    }

    if (entry->v2Launch || entry->voice2) {
        return entryInfo.calcUpStem() ? CurveContourDirection::Up : CurveContourDirection::Down;
    }

    if (entry->flipTie) {
        return entryInfo.calcUpStem() ? CurveContourDirection::Up : CurveContourDirection::Down;
    }

    const auto scrollViewStaves = entryFrame->getDocument()->getScrollViewStaves(entryFrame->getRequestedPartId());
    const int crossStaffDir = noteInfo.calcCrossStaffDirection(scrollViewStaves);
    if (crossStaffDir != 0) {
        return (crossStaffDir > 0) ? CurveContourDirection::Up : CurveContourDirection::Down;
    }

    return calcDefaultDirection(noteInfo, forTieEnd);
}

std::optional<TieConnectStyleType> Tie::calcConnectStyleTypeAtEndPoint(
    const dom::NoteInfoPtr& noteInfo, bool forTieEnd, CurveContourDirection direction, bool stemUp,
    const dom::NoteInfoPtr& tieAlterContext, bool forEndPoint,
    std::optional<size_t> noteIndexOverride, std::optional<size_t> noteCountOverride,
    std::optional<bool> upStemSecondOverride, std::optional<bool> downStemSecondOverride)
{
    if (!noteInfo) {
        return std::nullopt;
    }
    const auto entryInfo = noteInfo.getEntryInfo();
    const auto entry = entryInfo->getEntry();
    const auto note = noteInfo.operator->();

    const size_t noteIndex = noteIndexOverride.value_or(noteInfo.getNoteIndex());
    const size_t noteCount = noteCountOverride.value_or(entry->notes.size());
    const bool upStemSecond = upStemSecondOverride.value_or(note->upStemSecond);
    const bool downStemSecond = downStemSecondOverride.value_or(note->downStemSecond);
    const bool isStartPos = !forEndPoint;

    if ((noteIndex == 0 && direction == CurveContourDirection::Down)
        || (noteIndex + 1 == noteCount && direction == CurveContourDirection::Up)) {
        if (calcUseOuterPlacement(noteInfo, forTieEnd, tieAlterContext)) {
            if (entry->hasStem()) {
                if (forEndPoint) {
                    // A downstem 2nd is always treated as OuterNote
                    // An upstem 2nd is always treated as OuterStem
                    if (!stemUp && direction == CurveContourDirection::Down && !downStemSecond) {
                        return isStartPos ? TieConnectStyleType::UnderLowestNoteStemStartPosUnder
                                          : TieConnectStyleType::UnderLowestNoteStemEndPosUnder;
                    }
                    if (stemUp && direction == CurveContourDirection::Up && upStemSecond) {
                        return isStartPos ? TieConnectStyleType::OverHighestNoteStemStartPosOver
                                          : TieConnectStyleType::OverHighestNoteStemEndPosOver;
                    }
                } else {
                    // see comments above and take their opposites
                    if (stemUp && direction == CurveContourDirection::Up && !upStemSecond) {
                        return isStartPos ? TieConnectStyleType::OverHighestNoteStemStartPosOver
                                          : TieConnectStyleType::OverHighestNoteStemEndPosOver;
                    }
                    if (!stemUp && direction == CurveContourDirection::Down && downStemSecond) {
                        return isStartPos ? TieConnectStyleType::UnderLowestNoteStemStartPosUnder
                                          : TieConnectStyleType::UnderLowestNoteStemEndPosUnder;
                    }
                }
            }
            return (direction == CurveContourDirection::Down)
                ? (isStartPos ? TieConnectStyleType::UnderLowestNoteStartPosUnder
                              : TieConnectStyleType::UnderLowestNoteEndPosUnder)
                : (isStartPos ? TieConnectStyleType::OverHighestNoteStartPosOver
                              : TieConnectStyleType::OverHighestNoteEndPosOver);
        }
    }

    return (direction == CurveContourDirection::Down)
        ? (isStartPos ? TieConnectStyleType::UnderStartPosInner : TieConnectStyleType::UnderEndPosInner)
        : (isStartPos ? TieConnectStyleType::OverStartPosInner : TieConnectStyleType::OverEndPosInner);
}

std::optional<std::pair<TieConnectStyleType, TieConnectStyleType>> Tie::calcConnectStyleTypes(
    const dom::NoteInfoPtr& noteInfo, bool forTieEnd)
{
    if (!noteInfo) {
        return std::nullopt;
    }
    if ((forTieEnd && !noteInfo.calcHasTieEnd()) || (!forTieEnd && !noteInfo.calcHasTieStart())) {
        return std::nullopt;
    }
    const auto entryInfo = noteInfo.getEntryInfo();
    const auto entry = entryInfo->getEntry();

    auto direction = noteInfo.calcEffectiveTieDirection(forTieEnd);
    if (direction == CurveContourDirection::Unspecified) {
        return std::nullopt;
    }

    const bool stemUp = entryInfo.calcUpStem();
    auto startStyle = calcConnectStyleTypeAtEndPoint(noteInfo, forTieEnd, direction, stemUp, noteInfo, false)
        .value_or(TieConnectStyleType::OverStartPosInner);
    auto endStyle = startStyle; // initialize it with something

    if (forTieEnd) {
        endStyle = calcConnectStyleTypeAtEndPoint(noteInfo, forTieEnd, direction, stemUp, noteInfo, true)
            .value_or(TieConnectStyleType::OverEndPosInner);
    } else {
        if (auto endNote = calcTiedTo(noteInfo, true)) {
            const bool endStemUp = endNote.getEntryInfo().calcUpStem();
            endStyle = calcConnectStyleTypeAtEndPoint(endNote, forTieEnd, direction, endStemUp, noteInfo, true)
                .value_or(TieConnectStyleType::OverEndPosInner);
        } else {
            // Here is observed Finale behavior as of Finale 2000, when this code was reverse-engineered.
            // (It never changed through the rest of Finale's lifetime up through Finale 27.)
            // 1. Outer ties to rests and nothing have StemOuter placement at their endpoint.
            // 2. Outer ties to an adjacent empty bar have inner placement on both ends. (weird but true)
            // 3. Ties to notes are Inner if the down-tied-to entry has a note that is lower or
            //          an up-tied-to entry has a note that is higher.
            // 4. The flakiest behavior is with with under-ties to downstem chords containing 2nds.
            //          In this case, we must pass in the UPSTEM 2nd bit or'ed from all notes in the chord.
            auto nextEntry = entryInfo.getNextInLayer();
            if (nextEntry) {
                if (!nextEntry.calcDisplaysAsRest() && !nextEntry->getEntry()->notes.empty()) {
                    if (direction == CurveContourDirection::Down) {
                        NoteInfoPtr nextNote(nextEntry, 0);
                        NotePropertiesOptions propertiesOptions;
                        propertiesOptions.alwaysUseEntryStaff = true;
                        const int nextStaffPos = nextNote.calcNoteProperties(propertiesOptions).staffPosition;
                        const int currStaffPos = noteInfo.calcNoteProperties(propertiesOptions).staffPosition;
                        if (nextStaffPos < currStaffPos) {
                            // Ties to an adjacent empty bar have inner placement on both ends. (weird but true)
                            endStyle = TieConnectStyleType::UnderEndPosInner;
                        } else {
                            // Ties to notes are Inner if the down-tied-to entry has a note that is lower or
                            //    an up-tied-to entry has a note that is higher.
                            const bool nextStemUp = nextEntry.calcUpStem();
                            endStyle = calcConnectStyleTypeAtEndPoint(nextNote, forTieEnd, direction, nextStemUp, noteInfo, true)
                                .value_or(TieConnectStyleType::UnderEndPosInner);
                        }
                    } else {
                        const auto nextEntryNoteCount = nextEntry->getEntry()->notes.size();
                        NoteInfoPtr nextNote(nextEntry, nextEntryNoteCount - 1);
                        NotePropertiesOptions propertiesOptions;
                        propertiesOptions.alwaysUseEntryStaff = true;
                        const int nextStaffPos = nextNote.calcNoteProperties(propertiesOptions).staffPosition;
                        const int currStaffPos = noteInfo.calcNoteProperties(propertiesOptions).staffPosition;
                        if (nextStaffPos > currStaffPos) {
                            // Ties to an adjacent empty bar have inner placement on both ends. (weird but true)
                            endStyle = TieConnectStyleType::OverEndPosInner;
                        } else {
                            // Ties to notes are Inner if the down-tied-to entry has a note that is lower or
                            //    an up-tied-to entry has a note that is higher.
                            bool upStemSecond = nextNote->upStemSecond;
                            bool downStemSecond = nextNote->downStemSecond;
                            bool nextStemUp = nextEntry.calcUpStem();
                            if (nextStemUp) {
                                // Finale 2000 behavior:
                                // If the entry is downstem, OR together all the Upstem 2nd bits.
                                // The behavior is stranger in that it does not do this for Scroll View at less than 130%.
                                // However, it seems to do it consistently in Page View.
                                for (size_t noteIndex = 0; noteIndex < nextEntryNoteCount; noteIndex++) {
                                    NoteInfoPtr checkNote(nextEntry, noteIndex);
                                    if (checkNote->upStemSecond) {
                                        upStemSecond = true;
                                    }
                                }
                                nextStemUp = (direction == CurveContourDirection::Up);
                            }
                            endStyle = calcConnectStyleTypeAtEndPoint(
                                nextNote, forTieEnd, direction, nextStemUp, noteInfo, true,
                                nextNote.getNoteIndex(), nextEntryNoteCount, upStemSecond, downStemSecond)
                                .value_or(TieConnectStyleType::OverEndPosInner);
                        }
                    }
                } else {
                    // 1. Ties to rests and nothing have StemOuter placement at their endpoint when the tie is outer.
                    const bool nextStemUp = (direction == CurveContourDirection::Up);
                    endStyle = calcConnectStyleTypeAtEndPoint(noteInfo, forTieEnd, direction, nextStemUp, noteInfo, true,
                        noteInfo.getNoteIndex(), entry->notes.size(), false, false)
                        .value_or(TieConnectStyleType::OverEndPosInner);
                    if (isOuterConnectStyle(endStyle)) {
                        if (endStyle == TieConnectStyleType::OverHighestNoteEndPosOver) {
                            endStyle = TieConnectStyleType::OverHighestNoteStemEndPosOver;
                        } else if (endStyle == TieConnectStyleType::UnderLowestNoteEndPosUnder) {
                            endStyle = TieConnectStyleType::UnderLowestNoteStemEndPosUnder;
                        }
                    }
                }
            } else {
                if (calcIsEndOfSystem(noteInfo, false)) {
                    endStyle = (direction == CurveContourDirection::Down)
                        ? TieConnectStyleType::UnderLowestNoteStemEndPosUnder
                        : TieConnectStyleType::OverHighestNoteStemEndPosOver;
                } else {
                    endStyle = (direction == CurveContourDirection::Down)
                        ? TieConnectStyleType::UnderEndPosInner
                        : TieConnectStyleType::OverEndPosInner;
                }
            }
        }
    }

    // if either of the endpoints is inner, make both of them inner.
    if (isInnerConnectStyle(startStyle)) {
        endStyle = isOverInnerConnectStyle(startStyle)
            ? TieConnectStyleType::OverEndPosInner
            : TieConnectStyleType::UnderEndPosInner;
    } else if (isInnerConnectStyle(endStyle)) {
        startStyle = isOverInnerConnectStyle(endStyle)
            ? TieConnectStyleType::OverStartPosInner
            : TieConnectStyleType::UnderStartPosInner;
    }

    return std::make_pair(startStyle, endStyle);
}

bool Tie::calcIsOuterConnectStyle(TieConnectStyleType type)
{
    return isOuterConnectStyle(type);
}

std::optional<details::TieAlterBase::ConnectionType> Tie::calcConnectionType(
    const dom::NoteInfoPtr& noteInfo, bool forTieEnd, bool forEndPoint, bool forPageView)
{
    const auto direction = noteInfo.calcEffectiveTieDirection(forTieEnd);
    if (direction == CurveContourDirection::Unspecified) {
        return std::nullopt;
    }

    if (!forEndPoint && forTieEnd) {
        return details::TieAlterBase::ConnectionType::SystemStart;
    }
    if (forEndPoint && !forTieEnd && calcIsEndOfSystem(noteInfo, forPageView)) {
        return details::TieAlterBase::ConnectionType::SystemEnd;
    }

    const auto styles = calcConnectStyleTypes(noteInfo, forTieEnd);
    if (!styles) {
        return std::nullopt;
    }
    const auto& [startStyle, endStyle] = *styles;
    const auto connectStyle = forEndPoint ? endStyle : startStyle;

    const auto entryInfo = noteInfo.getEntryInfo();
    const auto entry = entryInfo->getEntry();
    const bool upStem = entryInfo.calcUpStem();
    const bool noteIsNonAligned = calcNoteIsNonAlignedSecond(noteInfo, upStem);
    const bool hasNonAligned = calcEntryHasNonAlignedSecond(entryInfo);
    NoteInfoPtr tieAlterContext = noteInfo;
    if (!forTieEnd && !noteInfo.calcHasTieStart() && noteInfo.calcHasTieEnd()) {
        if (auto fromNote = noteInfo.calcTieFrom(true)) {
            tieAlterContext = fromNote;
        }
    }
    const bool shiftForSeconds = calcShiftForSeconds(noteInfo, forTieEnd, tieAlterContext);

    const bool tieOver = direction == CurveContourDirection::Up;
    const bool tieUnder = direction == CurveContourDirection::Down;

    if (isInnerConnectStyle(connectStyle)) {
        if (forEndPoint) {
            if (calcUseBeforeSingleAcci(noteInfo, forTieEnd, tieAlterContext) && entry->notes.size() == 1
                && calcNoteHasAccidental(noteInfo)) {
                return details::TieAlterBase::ConnectionType::AccidentalLeftNoteCenter;
            }
            if (shiftForSeconds && hasNonAligned) {
                if ((upStem && !tieUnder && noteIsNonAligned) || (!upStem && !noteIsNonAligned)) {
                    return details::TieAlterBase::ConnectionType::NoteLeftNoteCenter;
                }
            }
            return details::TieAlterBase::ConnectionType::EntryLeftNoteCenter;
        }

        {
            const unsigned numDots = calcEntryDots(entryInfo);
            if ((calcUseAfterSingleDot(noteInfo, forTieEnd, tieAlterContext) && numDots == 1)
                || (calcUseAfterMultipleDots(noteInfo, forTieEnd, tieAlterContext) && numDots > 1)) {
                return details::TieAlterBase::ConnectionType::DotRightNoteCenter;
            }
        }

        if (shiftForSeconds && hasNonAligned) {
            if ((upStem && !noteIsNonAligned) || (!upStem && !tieOver && noteIsNonAligned)) {
                return details::TieAlterBase::ConnectionType::NoteRightNoteCenter;
            }
        }
        return details::TieAlterBase::ConnectionType::EntryRightNoteCenter;
    }

    if (isOuterNoteConnectStyle(connectStyle)) {
        if (connectStyle == TieConnectStyleType::OverHighestNoteStartPosOver
            || connectStyle == TieConnectStyleType::OverHighestNoteEndPosOver) {
            return details::TieAlterBase::ConnectionType::NoteCenterNoteTop;
        }
        return details::TieAlterBase::ConnectionType::NoteCenterNoteBottom;
    }

    if (isOuterStemConnectStyle(connectStyle)) {
        const bool overStem = connectStyle == TieConnectStyleType::OverHighestNoteStemStartPosOver
            || connectStyle == TieConnectStyleType::OverHighestNoteStemEndPosOver;
        if (overStem) {
            return forEndPoint
                ? details::TieAlterBase::ConnectionType::NoteLeftNoteTop
                : details::TieAlterBase::ConnectionType::NoteRightNoteTop;
        }
        return forEndPoint
            ? details::TieAlterBase::ConnectionType::NoteLeftNoteBottom
            : details::TieAlterBase::ConnectionType::NoteRightNoteBottom;
    }
    return std::nullopt;
}

options::TieOptions::ConnectStyle Tie::calcEndpointOffsets(const dom::NoteInfoPtr& noteInfo, bool forTieEnd, bool forEndPoint,
    std::optional<CurveContourDirection> direction, bool forPageView)
{
    options::TieOptions::ConnectStyle defaultOffset{};
    if (!noteInfo) {
        return defaultOffset;
    }
    const auto entryFrame = noteInfo.getEntryInfo().getFrame();
    const auto tieOptions = entryFrame->getDocument()->getOptions()->get<options::TieOptions>();
    if (!tieOptions) {
        return defaultOffset;
    }

    if (!direction.has_value() || *direction == CurveContourDirection::Unspecified) {
        direction = noteInfo.calcEffectiveTieDirection(forTieEnd);
    }
    if (!direction.has_value() || *direction == CurveContourDirection::Unspecified) {
        return defaultOffset;
    }

    const auto connectionType = calcConnectionType(noteInfo, forTieEnd, forEndPoint, forPageView);
    if (!connectionType) {
        return defaultOffset;
    }

    if (*connectionType == details::TieAlterBase::ConnectionType::SystemEnd) {
        defaultOffset.offsetX = tieOptions->sysBreakRightHAdj;
    } else if (*connectionType == details::TieAlterBase::ConnectionType::SystemStart) {
        defaultOffset.offsetX = tieOptions->sysBreakLeftHAdj;
    } else if (const auto styles = calcConnectStyleTypes(noteInfo, forTieEnd)) {
        defaultOffset = calcConnectStyleOffset(tieOptions, forEndPoint ? styles->second : styles->first);
    }

    return applyTieAlterEndpointOffsets(noteInfo, forTieEnd, forEndPoint, *connectionType, *direction, defaultOffset);
}

std::optional<Tie::ContourResult> Tie::calcContourStyleType(
    const dom::NoteInfoPtr& noteInfo, const ContourGeometry& geometry, bool forTieEnd)
{
    if (!noteInfo) {
        return std::nullopt;
    }

    const auto entryFrame = noteInfo.getEntryInfo().getFrame();
    const auto tieOptions = entryFrame->getDocument()->getOptions()->get<options::TieOptions>();
    if (!tieOptions) {
        return std::nullopt;
    }

    const auto direction = noteInfo.calcEffectiveTieDirection(forTieEnd);
    if (direction == CurveContourDirection::Unspecified) {
        return std::nullopt;
    }

    const auto connectStyles = calcConnectStyleTypes(noteInfo, forTieEnd);
    if (!connectStyles) {
        return std::nullopt;
    }
    const auto& [startStyle, endStyle] = *connectStyles;

    constexpr double kOuterNoteOffsetPct = 7.0 / 16.0;
    constexpr Evpu kInnerIncrement = 6;
    dom::EvpuFloat startPos = 0.0;
    dom::EvpuFloat endPos = geometry.startToEndLeft;
    Evpu startIncrement = 0;
    Evpu endIncrement = 0;

    if (geometry.startPointKind == EndPointKind::Notehead) {
        if (startStyle == TieConnectStyleType::OverStartPosInner
            || startStyle == TieConnectStyleType::OverHighestNoteStemStartPosOver
            || startStyle == TieConnectStyleType::UnderStartPosInner) {
            startPos += geometry.startNoteheadWidth;
            startIncrement = kInnerIncrement;
        } else {
            startPos += geometry.startNoteheadWidth * kOuterNoteOffsetPct;
        }
    }

    if (geometry.endPointKind == EndPointKind::Notehead) {
        if (endStyle == TieConnectStyleType::OverEndPosInner
            || endStyle == TieConnectStyleType::UnderEndPosInner
            || endStyle == TieConnectStyleType::UnderLowestNoteStemEndPosUnder) {
            endIncrement = -kInnerIncrement;
        } else {
            const dom::EvpuFloat endNoteheadWidth = geometry.endNoteheadWidth.value_or(geometry.startNoteheadWidth);
            endPos += endNoteheadWidth * (1.0 - kOuterNoteOffsetPct);
        }
    }

    const dom::EvpuFloat startOffset = static_cast<dom::EvpuFloat>(calcEndpointOffsetX(
        noteInfo, forTieEnd, false, geometry.startPointKind, direction)) + geometry.startAdjustment;
    const dom::EvpuFloat endOffset = static_cast<dom::EvpuFloat>(calcEndpointOffsetX(
        noteInfo, forTieEnd, true, geometry.endPointKind, direction)) + geometry.endAdjustment;

    const Evpu tieLength = static_cast<Evpu>(std::lround((endPos - startPos)
        + ((endOffset + endIncrement) - (startOffset + startIncrement))));

    if (tieOptions->useTieEndCtlStyle) {
        return ContourResult{options::TieOptions::ControlStyleType::TieEnds, tieLength};
    }

    const auto shortStyleIt = tieOptions->tieControlStyles.find(options::TieOptions::ControlStyleType::ShortSpan);
    const auto mediumStyleIt = tieOptions->tieControlStyles.find(options::TieOptions::ControlStyleType::MediumSpan);
    const auto longStyleIt = tieOptions->tieControlStyles.find(options::TieOptions::ControlStyleType::LongSpan);
    if (shortStyleIt == tieOptions->tieControlStyles.end() || !shortStyleIt->second
        || mediumStyleIt == tieOptions->tieControlStyles.end() || !mediumStyleIt->second
        || longStyleIt == tieOptions->tieControlStyles.end() || !longStyleIt->second) {
        return std::nullopt;
    }

    if (tieLength >= longStyleIt->second->span) {
        return ContourResult{options::TieOptions::ControlStyleType::LongSpan, tieLength};
    }
    if (tieLength <= shortStyleIt->second->span) {
        return ContourResult{options::TieOptions::ControlStyleType::ShortSpan, tieLength};
    }
    return ContourResult{options::TieOptions::ControlStyleType::MediumSpan, tieLength};
}

std::optional<Tie::ContourControlPoints> Tie::calcDefaultContourControlPoints(
    const dom::NoteInfoPtr& noteInfo, options::TieOptions::ControlStyleType styleType, Evpu length)
{
    if (!noteInfo) {
        return std::nullopt;
    }

    const auto tieOptions = noteInfo.getEntryInfo().getFrame()->getDocument()->getOptions()->get<options::TieOptions>();
    if (!tieOptions) {
        return std::nullopt;
    }

    auto getStyle = [&](options::TieOptions::ControlStyleType type)
        -> const std::shared_ptr<options::TieOptions::ControlStyle>& {
        static const std::shared_ptr<options::TieOptions::ControlStyle> kEmpty;
        const auto it = tieOptions->tieControlStyles.find(type);
        return (it == tieOptions->tieControlStyles.end()) ? kEmpty : it->second;
    };

    const auto style = getStyle(styleType);
    if (!style || !style->cp1 || !style->cp2) {
        return std::nullopt;
    }

    ContourControlPoints result{
        ContourControlPoint{style->cp1->insetRatio, style->cp1->height, style->cp1->insetFixed},
        ContourControlPoint{style->cp2->insetRatio, style->cp2->height, style->cp2->insetFixed},
        tieOptions->insetStyle
    };

    if (!tieOptions->useInterpolation || styleType != options::TieOptions::ControlStyleType::MediumSpan) {
        return result;
    }

    const auto shortStyle = getStyle(options::TieOptions::ControlStyleType::ShortSpan);
    const auto mediumStyle = getStyle(options::TieOptions::ControlStyleType::MediumSpan);
    const auto longStyle = getStyle(options::TieOptions::ControlStyleType::LongSpan);
    if (!shortStyle || !shortStyle->cp1 || !shortStyle->cp2
        || !mediumStyle || !mediumStyle->cp1 || !mediumStyle->cp2
        || !longStyle || !longStyle->cp1 || !longStyle->cp2) {
        return result;
    }

    const Evpu mediumSpan = mediumStyle->span;
    const bool belowMedium = length < mediumSpan;
    const auto& lowerStyle = belowMedium ? shortStyle : mediumStyle;
    const auto& upperStyle = belowMedium ? mediumStyle : longStyle;
    const Evpu lowerSpan = lowerStyle->span;
    const Evpu upperSpan = upperStyle->span;
    if (upperSpan == lowerSpan) {
        return result;
    }

    const double interpolationPercent =
        (static_cast<double>(length) - static_cast<double>(lowerSpan))
        / (static_cast<double>(upperSpan) - static_cast<double>(lowerSpan));

    auto lerpRounded = [interpolationPercent](auto lower, auto upper) {
        const double value = static_cast<double>(lower)
            + (static_cast<double>(upper) - static_cast<double>(lower)) * interpolationPercent;
        return static_cast<decltype(lower)>(std::lround(value));
    };

    result.left.height = lerpRounded(lowerStyle->cp1->height, upperStyle->cp1->height);
    result.right.height = lerpRounded(lowerStyle->cp2->height, upperStyle->cp2->height);

    if (tieOptions->insetStyle == options::TieOptions::InsetStyle::Percent) {
        result.left.insetRatio = lerpRounded(lowerStyle->cp1->insetRatio, upperStyle->cp1->insetRatio);
        result.right.insetRatio = lerpRounded(lowerStyle->cp2->insetRatio, upperStyle->cp2->insetRatio);
    }

    return result;
}

std::optional<Tie::ContourControlPoints> Tie::calcEffectiveContourControlPoints(
    const dom::NoteInfoPtr& noteInfo, bool forTieEnd, options::TieOptions::ControlStyleType styleType, Evpu length)
{
    if (!noteInfo) {
        return std::nullopt;
    }

    if (const auto tieAlter = details::TieAlterBase::fromNoteInfo(noteInfo, forTieEnd)) {
        if (tieAlter->ctlPtAdjOn) {
            const bool fixed = tieAlter->ctlPtFixed;
            return ContourControlPoints{
                ContourControlPoint{
                    static_cast<Efix>(tieAlter->insetRatio1),
                    tieAlter->height1,
                    fixed ? tieAlter->insetRatio1 : 0
                },
                ContourControlPoint{
                    static_cast<Efix>(tieAlter->insetRatio2),
                    tieAlter->height2,
                    fixed ? tieAlter->insetRatio2 : 0
                },
                fixed ? options::TieOptions::InsetStyle::Fixed
                      : options::TieOptions::InsetStyle::Percent
            };
        }
    }

    return calcDefaultContourControlPoints(noteInfo, styleType, length);
}

std::optional<Tie::ContourMetrics> Tie::calcEffectiveContourMetrics(
    const dom::NoteInfoPtr& noteInfo, const ContourGeometry& geometry, bool forTieEnd)
{
    const auto contour = calcContourStyleType(noteInfo, geometry, forTieEnd);
    if (!contour) {
        return std::nullopt;
    }

    const auto controlPoints = calcEffectiveContourControlPoints(
        noteInfo, forTieEnd, contour->styleType, contour->length);
    if (!controlPoints) {
        return std::nullopt;
    }

    return ContourMetrics{*contour, *controlPoints};
}

} // namespace util
} // namespace musx
