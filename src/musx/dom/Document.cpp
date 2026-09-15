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
#include <algorithm>
#include <cstdlib>
#include <exception>
#include <cstring>
#include <filesystem>
#include <limits>
#include <string>
#include <vector>

#include "musx/musx.h"

#if defined(MUSX_RUNNING_ON_MACOS)
#include <CoreFoundation/CoreFoundation.h>
#include <dlfcn.h>
#endif

namespace musx {
namespace dom {

#ifndef DOXYGEN_SHOULD_IGNORE_THIS
namespace {
#if defined(MUSX_RUNNING_ON_MACOS)
std::optional<std::filesystem::path> toPathFromCfString(CFStringRef value)
{
    if (!value) {
        return std::nullopt;
    }
    const auto length = CFStringGetLength(value);
    const auto maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8);
    if (maxSize < 0) {
        return std::nullopt;
    }
    std::string buffer(static_cast<size_t>(maxSize) + 1, '\0');
    if (!CFStringGetCString(value, buffer.data(), maxSize + 1, kCFStringEncodingUTF8)) {
        return std::nullopt;
    }
    buffer.resize(std::strlen(buffer.c_str()));
    return std::filesystem::path(buffer);
}

std::optional<std::filesystem::path> resolveMacBookmarkData(const std::vector<std::uint8_t>& data)
{
    if (data.empty()) {
        return std::nullopt;
    }
    CFDataRef bookmarkData = CFDataCreate(kCFAllocatorDefault, data.data(), static_cast<CFIndex>(data.size()));
    if (!bookmarkData) {
        return std::nullopt;
    }
    CFURLRef url = CFURLCreateByResolvingBookmarkData(
        kCFAllocatorDefault,
        bookmarkData,
        kCFBookmarkResolutionWithoutUIMask | kCFBookmarkResolutionWithoutMountingMask,
        nullptr,
        nullptr,
        nullptr,
        nullptr);
    CFRelease(bookmarkData);
    if (!url) {
        return std::nullopt;
    }
    CFStringRef pathString = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
    CFRelease(url);
    auto path = toPathFromCfString(pathString);
    if (pathString) {
        CFRelease(pathString);
    }
    return path;
}

std::optional<std::filesystem::path> resolveMacAliasData(const std::vector<std::uint8_t>& data, Cmper fileDescId)
{
    if (data.empty()) {
        return std::nullopt;
    }
    CFDataRef aliasData = CFDataCreate(kCFAllocatorDefault, data.data(), static_cast<CFIndex>(data.size()));
    if (!aliasData) {
        return std::nullopt;
    }
    using BookmarkFromAliasFn = CFDataRef (*)(CFAllocatorRef, CFDataRef);
    auto fn = reinterpret_cast<BookmarkFromAliasFn>(dlsym(RTLD_DEFAULT, "CFURLCreateBookmarkDataFromAliasRecord"));
    if (!fn) {
        ::musx::util::Logger::log(::musx::util::Logger::LogLevel::Warning,
            "External graphic path resolution failed for fileDesc " + std::to_string(fileDescId)
            + ": macOS alias resolution is unavailable (CFURLCreateBookmarkDataFromAliasRecord not found).");
        CFRelease(aliasData);
        return std::nullopt;
    }
    CFDataRef bookmarkData = fn ? fn(kCFAllocatorDefault, aliasData) : nullptr;
    CFRelease(aliasData);
    if (!bookmarkData) {
        ::musx::util::Logger::log(::musx::util::Logger::LogLevel::Warning,
            "External graphic path resolution failed for fileDesc " + std::to_string(fileDescId)
            + ": unable to resolve macOS alias record to bookmark data.");
        return std::nullopt;
    }
    std::vector<std::uint8_t> bytes(static_cast<size_t>(CFDataGetLength(bookmarkData)));
    if (!bytes.empty()) {
        CFDataGetBytes(bookmarkData, CFRangeMake(0, CFDataGetLength(bookmarkData)), bytes.data());
    }
    CFRelease(bookmarkData);
    return resolveMacBookmarkData(bytes);
}
#endif
} // namespace
#endif // DOXYGEN_SHOULD_IGNORE_THIS

// ********************
// ***** Document *****
// ********************

Cmper Document::calcScrollViewCmper(Cmper partId) const noexcept
{
    if (const auto partGlobals = getOthers()->get<others::PartGlobals>(partId, MUSX_GLOBALS_CMPER)) {
        return partGlobals->calcScrollViewCmper();
    }
    return BASE_SYSTEM_ID;
}

MusxInstanceList<others::StaffUsed> Document::getScrollViewStaves(Cmper partId) const
{
    return getOthers()->getArray<others::StaffUsed>(partId, calcScrollViewCmper(partId));
}

MusxInstanceList<others::StaffUsed> Document::getStudioViewStaves(Cmper partId) const
{
    const Cmper studioViewCmper = [&]() {
        if (const auto partGlobals = getOthers()->get<others::PartGlobals>(partId, MUSX_GLOBALS_CMPER)) {
            return partGlobals->studioViewIUlist;
        }
        return STUDIO_VIEW_SYSTEM_ID;
    }();
    return getOthers()->getArray<others::StaffUsed>(partId, studioViewCmper);
}

std::optional<KnownShapeDefType> Document::getCachedShapeRecognition(Cmper shapeCmper) const
{
    auto it = m_shapeRecognitionCache.find(shapeCmper);
    if (it != m_shapeRecognitionCache.end()) {
        return it->second;
    }
    return std::nullopt;
}

void Document::setCachedShapeRecognition(Cmper shapeCmper, KnownShapeDefType type) const
{
    m_shapeRecognitionCache[shapeCmper] = type;
}

std::optional<bool> Document::getCachedFontIsSMuFL(Cmper fontId) const
{
    auto it = m_isSmuflFontCache.find(fontId);
    if (it != m_isSmuflFontCache.end()) {
        return it->second;
    }
    return std::nullopt;
}

void Document::setCachedFontIsSMuFL(Cmper fontId, bool isSmufl) const
{
    m_isSmuflFontCache[fontId] = isSmufl;
}

MusxInstance<others::Page> Document::calcPageFromMeasure(Cmper partId, MeasCmper measureId) const
{
    const auto part = getOthers()->get<others::PartDefinition>(SCORE_PARTID, partId);
    if (!part || !part->isLayoutCalculated()) {
        return nullptr;
    }
    MusxInstance<others::Page> result;
    auto pages = getOthers()->getArray<others::Page>(partId);
    for (const auto& page : pages) {
        if (page->isBlank() || !page->isLayoutCalculated()) {
            continue;
        }
        if (measureId >= page->firstMeasureId.value() && measureId <= page->lastMeasureId.value()) {
            result = page;
            break;
        }
    }
    if (!result) {
        MUSX_INTEGRITY_ERROR("Unable to find page for measure ID " + std::to_string(measureId));
    }
    return result;
}

MusxInstance<others::StaffSystem> Document::calcSystemFromMeasure(Cmper partId, MeasCmper measureId) const
{
    const auto part = getOthers()->get<others::PartDefinition>(SCORE_PARTID, partId);
    if (!part || !part->isLayoutCalculated()) {
        return nullptr;
    }
    MusxInstance<others::StaffSystem> result;
    auto systems = getOthers()->getArray<others::StaffSystem>(partId);
    for (const auto& system : systems) {
        // endMeas is 1 measure past the end of the system
        if (measureId >= system->startMeas && measureId < system->endMeas) {
            result = system;
            break;
        }
    }
    if (!result) {
        MUSX_INTEGRITY_ERROR("Unable to find system for measure ID " + std::to_string(measureId));
    }
    return result;
}

InstrumentMap Document::createInstrumentMap(Cmper forPartId) const
{
    InstrumentMap result(m_self, forPartId);

    const auto scrollView = getScrollViewStaves(forPartId);
    if (scrollView.empty()) {
        return result;
    }
    std::unordered_set<Cmper> multiStaffInstsFound;
    std::unordered_set<StaffCmper> mappedStaves;
    for (const auto& staffItem : scrollView) {
        if (auto rawStaff = getOthers()->get<others::Staff>(forPartId, staffItem->staffId)) { // do not use staffItem->getStaffInstance() because we want no throw here
            if (rawStaff->multiStaffInstId != 0) {
                if (multiStaffInstsFound.find(rawStaff->multiStaffInstId) == multiStaffInstsFound.end()) {
                    if (auto multiStaffInst = getOthers()->get<others::MultiStaffInstrumentGroup>(forPartId, rawStaff->multiStaffInstId)) {
                        multiStaffInstsFound.emplace(rawStaff->multiStaffInstId);
                        const auto [it, created] = result.emplace(rawStaff->getCmper(), InstrumentInfo(m_self, forPartId));
                        MUSX_ASSERT_IF(!created) {
                            throw std::logic_error("Attempted to insert multi-instrument id " + std::to_string(rawStaff->multiStaffInstId) + " more than once.");
                        }
                        if (auto multiStaffGroupId = getOthers()->get<others::MultiStaffGroupId>(forPartId, rawStaff->multiStaffInstId)) {
                            it->second.staffGroupId = multiStaffGroupId->staffGroupId;
                        }
                        it->second.multistaffGroupId = rawStaff->multiStaffInstId;
                        std::optional<size_t> topIndex = scrollView.getIndexForStaff(rawStaff->getCmper());
                        MUSX_ASSERT_IF(!topIndex.has_value()) {
                            throw std::logic_error("Unable to find " + std::to_string(rawStaff->getCmper()) + " in scrollView.");
                        }
                        for (StaffCmper staffId : multiStaffInst->staffNums) {
                            std::optional<size_t> staffIndex = scrollView.getIndexForStaff(staffId);
                            if (staffIndex.has_value()) {
                                it->second.staves.emplace(staffId, staffIndex.value() - topIndex.value());
                                mappedStaves.emplace(staffId);
                            } else if (forPartId == SCORE_PARTID) {
                                MUSX_INTEGRITY_ERROR("Unable to find staff " + std::to_string(staffId) + " from multistaff instrument group in scrollView.");
                            }
                        }
                    }
                }
            }
        }
    }
    auto staffGroups = details::StaffGroupInfo::getGroupsAtMeasure(1, forPartId, scrollView);
    for (const auto& staffGroup : staffGroups) {
        const auto& group = staffGroup.group;
        // for now, only identify piano braces as visual staff groups
        if (group->bracket && group->bracket->style == details::Bracket::BracketStyle::PianoBrace) {
            if (const auto topStaff = getOthers()->get<others::Staff>(forPartId, group->startInst)) {
                std::unordered_map<StaffCmper, size_t> candidateStaves;
                size_t sequenceIndex = 0;
                staffGroup.iterateStaves(1, 0, [&](const MusxInstance<others::StaffComposite>& nextStaff) {
                    if (nextStaff->multiStaffInstId == topStaff->multiStaffInstId || mappedStaves.find(nextStaff->getCmper()) == mappedStaves.end()) {
                        if (nextStaff->instUuid == topStaff->instUuid || !nextStaff->hasInstrumentAssigned()) {
                            if (nextStaff->hideNameInScore || nextStaff->getFullName().empty()) {
                                const auto [it, created] = candidateStaves.emplace(nextStaff->getCmper(), sequenceIndex++);
                                MUSX_ASSERT_IF (!created) {
                                    throw std::logic_error("Attempted to insert staff " + std::to_string(nextStaff->getCmper()) + " twice in the same multistaff instrument.");
                                }
                                return true;
                            }
                        }
                    }
                    candidateStaves.clear();
                    return false;
                });
                if (!candidateStaves.empty()) {
                    auto [instIt, created] = result.emplace(topStaff->getCmper(), InstrumentInfo(m_self, forPartId));
                    auto& [top, instInfo] = *instIt;
                    if (created || instInfo.staffGroupId == 0 || group->getCmper2() == instInfo.staffGroupId) {
                        if (instInfo.staffGroupId == 0) {
                            util::Logger::log(util::Logger::LogLevel::Verbose, "Treating piano brace " + std::to_string(group->getCmper2())
                                + " [" + group->getFullName() + "] on staff " + std::to_string(group->startInst) + " as a multistaff instrument.");
                        }
                        instInfo.staffGroupId = group->getCmper2();
                        for (const auto& [cmper, index] : candidateStaves) {
                            instInfo.staves.insert_or_assign(cmper, index);
                            mappedStaves.emplace(cmper);
                        }
                    }
                }
            }
        }
    }
    for (const auto& staffItem : scrollView) {
        if (mappedStaves.find(staffItem->staffId) == mappedStaves.end()) {
            const auto [it, created] = result.emplace(staffItem->staffId, InstrumentInfo(m_self, forPartId));
            MUSX_ASSERT_IF(!created) {
                throw std::logic_error("Attempted to insert single-instrument id " + std::to_string(staffItem->staffId) + " that was already mapped.");
            }
            it->second.staves.emplace(staffItem->staffId, 0);
        }
    }
    for (const auto& [instId, info] : result) {
        std::unordered_set<size_t> indices;
        for (const auto& [staffId, index] : info.staves) {
            indices.insert(index);
        }
        if (indices.size() != info.staves.size()) {
            MUSX_INTEGRITY_ERROR("Instrument " + std::to_string(instId) + " has duplicate or missing staff indices.");
        } else {
            for (size_t i = 0; i < info.staves.size(); ++i) {
                bool breakOut = false; // avoid compiler warning if MUSX_INTEGRITY_ERROR throws
                if (!indices.count(i)) {
                    breakOut = true;
                    MUSX_INTEGRITY_ERROR("Instrument " + std::to_string(instId) + " is missing index " + std::to_string(i));
                }
                if (breakOut) break;
            }
        }
    }
    return result;
}

const InstrumentInfo& Document::getInstrumentForStaff(StaffCmper staffId) const
{
    auto result = getInstruments().getInstrumentForStaff(staffId);
    MUSX_ASSERT_IF(!result) { // flag this as early as possible, because getting here is a program bug.
        throw std::logic_error("Staff " + std::to_string(staffId) + " was not mapped to an instrument.");
    }
    return *result;
}

std::optional<InstrumentInfo> Document::getInstrumentForStaff(Cmper partId, StaffCmper staffId) const
{
    if (partId == SCORE_PARTID) {
        if (const auto result = getInstruments().getInstrumentForStaff(staffId)) {
            return *result;
        }
        return std::nullopt;
    }
    const auto partInstrumentMap = createInstrumentMap(partId);
    if (const auto result = partInstrumentMap.getInstrumentForStaff(staffId)) {
        return *result;
    }
    return std::nullopt;
}

void Document::createRehearsalMarkMap()
{
    m_rehearsalMarks.clear();
    const auto assignments = getOthers()->getArray<others::MeasureExprAssign>(SCORE_PARTID);
    const auto defs = getOthers()->getArray<others::TextExpressionDef>(SCORE_PARTID);
    for (const auto& def : defs) {
        using RMS = others::RehearsalMarkStyle;
        if (def->rehearsalMarkStyle == RMS::None || def->rehearsalMarkStyle == RMS::MeasureNumber) {
            continue;
        }
        // This code depends on the assignments being in measure number order, but that is
        // a built-in guarantee from how the pools are structured. If it changed, the pain
        // would extend far beyond this little corner of the library.
        int currentSequence = 1;
        for (const auto& assign : assignments) {
            if (assign->textExprId != def->getCmper()) {
                continue;
            }
            auto newSequencenNum = currentSequence;
            if (assign->rehearsalMarkOffset > 0) {
                newSequencenNum = assign->rehearsalMarkOffset;
            }
            const auto key = RehearsalMarkKey{ MeasCmper(assign->getCmper()), assign->textExprId };
            if (m_rehearsalMarks.emplace(key, RehearsalMarkInfo{ newSequencenNum }).second) {
                currentSequence = newSequencenNum + 1;
            }
        }
    }
}

std::optional<RehearsalMarkInfo> Document::getRehearsalMarkInfo(MeasCmper measureId, Cmper textExpressionId) const
{
    const auto it = m_rehearsalMarks.find(RehearsalMarkKey{ measureId, textExpressionId });
    if (it == m_rehearsalMarks.end()) {
        return std::nullopt;
    }
    return it->second;
}

MusicRange Document::calcEntireDocument() const
{
    auto measures = getOthers()->getArray<others::Measure>(SCORE_PARTID);
    return MusicRange(m_self, 1, 0, static_cast<MeasCmper>(measures.size()), (std::numeric_limits<util::Fraction>::max)());
}

bool Document::calcHasVaryingSystemStaves(Cmper forPartId) const
{
    auto staffSystems = getOthers()->getArray<others::StaffSystem>(forPartId);
    auto scrollView = getScrollViewStaves(forPartId);
    for (const auto& staffSystem : staffSystems) {
        auto nextSystem = getOthers()->getArray<others::StaffUsed>(forPartId, staffSystem->getCmper());
        if (nextSystem.size() != scrollView.size()) {
            return true;
        }
        for (size_t staffIndex = 0; staffIndex < nextSystem.size(); staffIndex++) {
            if (nextSystem[staffIndex]->staffId != scrollView[staffIndex]->staffId) {
                return true;
            }
        }
    }
    return false;
}

std::vector<MeasCmper> Document::calcJumpFromMeasures(Cmper partId, MeasCmper currentMeasure) const
{
    std::vector<MeasCmper> forwardJumpOrigins;
    std::vector<MeasCmper> backwardJumpOrigins;

    const auto addOrigin = [&](MeasCmper origin) {
        if (origin > currentMeasure) {
            forwardJumpOrigins.push_back(origin);
        } else if (origin < currentMeasure) {
            backwardJumpOrigins.push_back(origin);
        }
    };

    const auto endings = getOthers()->getArray<others::RepeatEndingStart>(partId);
    for (const auto& ending : endings) {
        if (!ending) {
            continue;
        }
        if (const auto passList = getOthers()->get<others::RepeatPassList>(partId, ending->getCmper())) {
            // currently we only search first endings. if necessary we can get more complicated with other endings.
            if (std::find(passList->values.begin(), passList->values.end(), 1) == passList->values.end()) {
                continue;
            }
        }
        const auto target = ending->calcTargetMeasure();
        if (target && *target == currentMeasure) {
            if (ending->getCmper() > 1) {
                addOrigin(ending->getCmper() - 1);
            }
        }
    }

    const auto repeatBacks = getOthers()->getArray<others::RepeatBack>(partId);
    for (const auto& repeatBack : repeatBacks) {
        if (!repeatBack) {
            continue;
        }
        const auto target = repeatBack->calcTargetMeasure();
        if (target && *target == currentMeasure) {
            addOrigin(repeatBack->getCmper());
        }
    }

    const auto textRepeats = getOthers()->getArray<others::TextRepeatAssign>(partId);
    for (const auto& textRepeat : textRepeats) {
        if (!textRepeat) {
            continue;
        }
        const auto target = textRepeat->calcTargetMeasure();
        if (target && *target == currentMeasure) {
            addOrigin(textRepeat->getCmper());
        }
    }

    std::sort(forwardJumpOrigins.begin(), forwardJumpOrigins.end());
    forwardJumpOrigins.erase(std::unique(forwardJumpOrigins.begin(), forwardJumpOrigins.end()),
        forwardJumpOrigins.end());

    std::sort(backwardJumpOrigins.begin(), backwardJumpOrigins.end());
    backwardJumpOrigins.erase(std::unique(backwardJumpOrigins.begin(), backwardJumpOrigins.end()),
        backwardJumpOrigins.end());
    std::reverse(backwardJumpOrigins.begin(), backwardJumpOrigins.end());

    std::vector<MeasCmper> result;
    result.reserve(forwardJumpOrigins.size() + backwardJumpOrigins.size());
    result.insert(result.end(), forwardJumpOrigins.begin(), forwardJumpOrigins.end());
    result.insert(result.end(), backwardJumpOrigins.begin(), backwardJumpOrigins.end());
    return result;
}

bool Document::iterateEntries(Cmper partId, std::function<bool(const EntryInfoPtr&)> iterator) const
{
    auto scrollView = getScrollViewStaves(partId);
    if (scrollView.empty()) {
        return true;
    }
    return scrollView.iterateEntries(0, scrollView.size() - 1, calcEntireDocument(), iterator);
}

std::optional<std::filesystem::path> Document::resolveExternalGraphicPath(Cmper fileDescId) const
{
    const auto others = getOthers();
    if (!others) {
        return std::nullopt;
    }

    const auto fileDesc = others->get<others::FileDescription>(SCORE_PARTID, fileDescId);
    const Cmper pathId = fileDesc ? fileDesc->pathId : fileDescId;
    const auto filePath = others->get<others::FilePath>(SCORE_PARTID, pathId);
    if (!filePath || filePath->path.empty()) {
        return std::nullopt;
    }

    const auto trySourceDirectory = [&](const std::filesystem::path& storedPath)
        -> std::optional<std::filesystem::path> {
        if (!m_sourcePath) {
            return std::nullopt;
        }
        std::filesystem::path filename = storedPath.filename();
        if (filename.empty()) {
            return std::nullopt;
        }
        std::filesystem::path candidate = m_sourcePath->parent_path() / filename;
        if (std::filesystem::is_regular_file(candidate)) {
            return candidate;
        }
        return std::nullopt;
    };

    std::filesystem::path storedPath = std::filesystem::path(filePath->path);

    if (auto candidate = trySourceDirectory(storedPath)) {
        return candidate;
    }

    if (fileDesc) {
        switch (fileDesc->pathType) {
        case others::FileDescription::PathType::MacAlias: {
#if defined(MUSX_RUNNING_ON_MACOS)
            const auto alias = others->get<others::FileAlias>(SCORE_PARTID, fileDescId);
            if (alias) {
                if (auto path = resolveMacAliasData(alias->aliasHandle, fileDescId); path && std::filesystem::is_regular_file(*path)) {
                    return path;
                }
            }
#endif
            break;
        }
        case others::FileDescription::PathType::MacUrlBookmark: {
#if defined(MUSX_RUNNING_ON_MACOS)
            const auto bookmark = others->get<others::FileUrlBookmark>(SCORE_PARTID, fileDescId);
            if (bookmark) {
                if (auto path = resolveMacBookmarkData(bookmark->urlBookmarkData); path && std::filesystem::is_regular_file(*path)) {
                    return path;
                }
            }
#endif
            break;
        }
        case others::FileDescription::PathType::MacFsSpec:
            break;
        case others::FileDescription::PathType::DosPath:
        case others::FileDescription::PathType::MacPosixPath:
            break;
        }
    }

    if (storedPath.is_absolute() && std::filesystem::is_regular_file(storedPath)) {
        return storedPath;
    }

    return std::nullopt;
}

} // namespace dom
} // namespace mus
