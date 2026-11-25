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
#include <algorithm>
#include <cctype>

#include "musx/musx.h"

namespace musx {
namespace dom {

namespace {

std::string_view trimView(std::string_view value)
{
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) {
        value.remove_prefix(1);
    }
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
        value.remove_suffix(1);
    }
    return value;
}

std::string normalizeUuid(std::string_view uuid)
{
    std::string result(trimView(uuid));
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return result;
}

} // namespace

// ********************
// ***** Soundmap *****
// ********************

Soundmap::Soundmap(SoundmapDescription description, std::vector<SoundmapSound> sounds,
    const std::vector<std::pair<std::string, std::string>>& instrumentSoundRefs)
    : m_description(std::move(description)), m_sounds(std::move(sounds))
{
    for (size_t index = 0; index < m_sounds.size(); ++index) {
        m_soundsById.try_emplace(m_sounds[index].id, index);
    }
    for (const auto& [instrumentUuid, soundRef] : instrumentSoundRefs) {
        m_soundRefsByInstrument.try_emplace(normalizeUuid(instrumentUuid), std::string(trimView(soundRef)));
    }
}

const SoundmapSound* Soundmap::findSoundById(std::string_view soundId) const
{
    const auto it = m_soundsById.find(std::string(trimView(soundId)));
    return it == m_soundsById.end() ? nullptr : &m_sounds[it->second];
}

std::string_view Soundmap::findInstrumentSoundRef(std::string_view instrumentUuid) const
{
    const auto it = m_soundRefsByInstrument.find(normalizeUuid(instrumentUuid));
    return it == m_soundRefsByInstrument.end() ? std::string_view{} : std::string_view(it->second);
}

// **************************
// ***** DeviceNoteList *****
// **************************

std::optional<PercussionNoteTypeId> DeviceNoteList::findNoteType(int midiKey) const
{
    for (const auto& entry : m_entries) {
        if (entry.midiKey == midiKey) {
            return entry.noteType;
        }
    }
    return std::nullopt;
}

std::optional<int> DeviceNoteList::findMidiKey(PercussionNoteTypeId noteType, NoteTypeMatch match) const
{
    const auto findExact = [&](PercussionNoteTypeId type) -> std::optional<int> {
        for (const auto& entry : m_entries) {
            if (entry.noteType == type) {
                return entry.midiKey;
            }
        }
        return std::nullopt;
    };
    if (const auto key = findExact(noteType)) {
        return key;
    }
    const auto baseType = PercussionNoteTypeId(noteType & percussion::BASE_NOTE_TYPE_MASK);
    if (match == NoteTypeMatch::ExactThenBase && baseType != noteType) {
        return findExact(baseType);
    }
    return std::nullopt;
}

// *****************************
// ***** PlaybackResources *****
// *****************************

std::string_view PlaybackResources::getDeviceFileName(std::string_view path)
{
    path = trimView(path);
    const auto slash = path.find_last_of("/\\");
    return slash == std::string_view::npos ? path : path.substr(slash + 1);
}

PlaybackResources::PlaybackResources(std::vector<Soundmap> soundmaps, std::vector<DeviceNoteList> noteLists)
    : m_soundmaps(std::move(soundmaps)), m_noteLists(std::move(noteLists))
{
    for (const auto& soundmap : m_soundmaps) {
        for (const auto& sound : soundmap.getSounds()) {
            if (auto uuid = normalizeUuid(sound.uuid); !uuid.empty()) {
                m_soundsByUuid.try_emplace(std::move(uuid), &sound);
            }
        }
    }
    for (size_t index = 0; index < m_noteLists.size(); ++index) {
        const auto& list = m_noteLists[index];
        m_deviceFileNames.insert(list.getFileName());
        m_noteListsByName.try_emplace({list.getFileName(), list.getListName()}, index);
    }
}

PlaybackLookupResult<SoundmapSound> PlaybackResources::findSoundByUuid(std::string_view soundUuid) const
{
    if (isEmpty()) {
        return {nullptr, PlaybackLookupFailure::ResourcesNotSupplied};
    }
    const auto it = m_soundsByUuid.find(normalizeUuid(soundUuid));
    if (it == m_soundsByUuid.end()) {
        return {nullptr, PlaybackLookupFailure::SoundNotFound};
    }
    return {it->second, std::nullopt};
}

PlaybackLookupResult<SoundmapSound> PlaybackResources::findMappedSound(std::string_view pluginId, std::string_view instrumentUuid) const
{
    if (isEmpty()) {
        return {nullptr, PlaybackLookupFailure::ResourcesNotSupplied};
    }
    const auto plugin = trimView(pluginId);
    for (const auto& soundmap : m_soundmaps) {
        if (soundmap.getDescription().pluginId != plugin) {
            continue;
        }
        const auto soundRef = soundmap.findInstrumentSoundRef(instrumentUuid);
        if (soundRef.empty()) {
            continue;
        }
        if (const auto* sound = soundmap.findSoundById(soundRef)) {
            return {sound, std::nullopt};
        }
    }
    return {nullptr, PlaybackLookupFailure::SoundNotFound};
}

PlaybackLookupResult<DeviceNoteList> PlaybackResources::findNoteList(std::string_view fileName, std::string_view listName) const
{
    if (isEmpty()) {
        return {nullptr, PlaybackLookupFailure::ResourcesNotSupplied};
    }
    const std::string file(getDeviceFileName(fileName));
    if (m_deviceFileNames.find(file) == m_deviceFileNames.end()) {
        return {nullptr, PlaybackLookupFailure::DeviceFileNotSupplied};
    }
    const auto it = m_noteListsByName.find({file, std::string(trimView(listName))});
    if (it == m_noteListsByName.end()) {
        return {nullptr, PlaybackLookupFailure::NoteListNotFound};
    }
    return {&m_noteLists[it->second], std::nullopt};
}

PlaybackLookupResult<DeviceNoteList> PlaybackResources::resolveNoteList(const SoundmapSound& sound) const
{
    if (isEmpty()) {
        return {nullptr, PlaybackLookupFailure::ResourcesNotSupplied};
    }
    if (trimView(sound.percussionMidiMap).empty() || trimView(sound.percussionMidiMapFile).empty()) {
        return {nullptr, PlaybackLookupFailure::NoPercussionMidiMap};
    }
    return findNoteList(sound.percussionMidiMapFile, sound.percussionMidiMap);
}

PercussionPlaybackKey PlaybackResources::resolvePercussionMidiKey(std::string_view soundUuid, PercussionNoteTypeId noteType,
    NoteTypeMatch match) const
{
    PercussionPlaybackKey result;
    if (trimView(soundUuid).empty()) {
        result.failure = isEmpty() ? PlaybackLookupFailure::ResourcesNotSupplied : PlaybackLookupFailure::NoSoundUuid;
        return result;
    }
    const auto sound = findSoundByUuid(soundUuid);
    if (!sound) {
        result.failure = sound.failure;
        return result;
    }
    const auto noteList = resolveNoteList(*sound.value);
    if (!noteList) {
        result.sound = sound.value;
        result.failure = noteList.failure;
        return result;
    }
    result = resolvePercussionMidiKey(*noteList.value, noteType, match);
    result.sound = sound.value;
    return result;
}

PercussionPlaybackKey PlaybackResources::resolvePercussionMidiKey(const DeviceNoteList& noteList, PercussionNoteTypeId noteType,
    NoteTypeMatch match)
{
    PercussionPlaybackKey result;
    result.noteList = &noteList;
    if (const auto key = noteList.findMidiKey(noteType, NoteTypeMatch::Exact)) {
        result.midiKey = key;
        result.matchedNoteType = noteType;
        return result;
    }
    const auto baseType = PercussionNoteTypeId(noteType & percussion::BASE_NOTE_TYPE_MASK);
    if (match == NoteTypeMatch::ExactThenBase && baseType != noteType) {
        if (const auto key = noteList.findMidiKey(baseType, NoteTypeMatch::Exact)) {
            result.midiKey = key;
            result.matchedNoteType = baseType;
            result.matchedBaseType = true;
            return result;
        }
    }
    result.failure = PlaybackLookupFailure::NoteTypeNotListed;
    return result;
}

} // namespace dom
} // namespace musx
