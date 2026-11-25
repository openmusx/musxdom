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

#include <cstddef>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "musx/dom/Fundamentals.h"

namespace musx {
namespace factory {
class PlaybackResourcesFactory;
} // namespace factory

namespace dom {

class PlaybackResources;

/// @brief Shared, immutable playback resources. Null means that no resources were supplied.
using PlaybackResourcesPtr = std::shared_ptr<const PlaybackResources>;

constexpr int MAX_MIDI_KEY = 127; ///< The highest 0-based MIDI key number.

/**
 * @enum PlaybackLookupFailure
 * @brief Identifies the link that failed when resolving playback resources.
 */
enum class PlaybackLookupFailure
{
    ResourcesNotSupplied,   ///< No playback resources were supplied.
    NotPercussionNote,      ///< The note is not a mapped percussion note.
    NoPlaybackRoute,        ///< The note's layer has no play data, or its play data names no playback route.
    NoSoundUuid,            ///< The playback route has no sound UUID.
    SoundNotFound,          ///< No supplied soundmap contains the requested sound.
    SoundmapNotFound,       ///< No supplied soundmap has the requested plugin ID.
    NoPercussionMidiMap,    ///< The sound names no percussion MIDI map or no device file for it.
    DeviceFileNotSupplied,  ///< The device annotation file named by the sound was not supplied.
    NoteListNotFound,       ///< The device annotation file has no note-name list with the requested name.
    NoteTypeNotListed,      ///< The note-name list has no key for the percussion note type.
};

/**
 * @struct PlaybackLookupResult
 * @brief The result of a playback resource lookup: either a value or the reason there is none.
 * @tparam T The type of the value found.
 */
template <typename T>
struct PlaybackLookupResult
{
    const T* value{};                               ///< The value found, or null when the lookup failed.
    std::optional<PlaybackLookupFailure> failure;   ///< The reason the lookup failed. Empty when #value is non-null.

    /// @brief Returns true if the lookup found a value.
    explicit operator bool() const { return value != nullptr; }
};

/**
 * @struct SoundmapDescription
 * @brief The `<Description>` of one supplied soundmap.
 */
struct SoundmapDescription
{
    std::string name;       ///< The soundmap's display name. (xml node is `<Name>`)
    std::string pluginId;   ///< The four-character ID of the playback plugin that uses the soundmap. (xml node is `<PluginID>`)
    std::string libraryId;  ///< The soundmap's library ID. (xml node is `<LibraryID>`)
};

/**
 * @struct SoundmapSound
 * @brief One `<Sound>` in a supplied soundmap.
 *
 * All text values are trimmed.
 */
struct SoundmapSound
{
    std::string id;                     ///< The sound's id, referenced by `<MapSounds>`. (xml attribute is `id`)
    std::string uuid;                   ///< The sound's UUID, as spelled in the soundmap. (xml attribute is `uuid`)
    std::string name;                   ///< The sound's display name. (xml node is `<Name>`)
    std::string percussionMidiMap;      ///< The name of the note-name list for percussion playback, or empty. (xml node is `<PercMIDIMap>`)
    std::string percussionMidiMapFile;  ///< The device annotation file containing #percussionMidiMap, or empty. (xml node is `<PercMIDIMapFile>`)
    std::optional<int> programChange;   ///< The first program number in the sound's patch commands, if any.
                                        ///< (xml is `<PatchMIDICommands><ProgramChange Number>`)
    size_t soundmapIndex{};             ///< The index of the containing soundmap in @ref PlaybackResources::getSoundmaps.
};

/**
 * @class Soundmap
 * @brief The contents of one supplied soundmap.
 */
class Soundmap
{
public:
    /// @brief Constructs a soundmap from its parsed contents.
    /// @param description The soundmap's description.
    /// @param sounds The soundmap's sounds, in file order.
    /// @param instrumentSoundRefs Pairs of instrument UUID and sound id from `<MapSounds>`, in file order.
    Soundmap(SoundmapDescription description, std::vector<SoundmapSound> sounds,
        const std::vector<std::pair<std::string, std::string>>& instrumentSoundRefs);

    /// @brief Returns the soundmap's description.
    [[nodiscard]] const SoundmapDescription& getDescription() const { return m_description; }

    /// @brief Returns the soundmap's sounds in file order.
    [[nodiscard]] const std::vector<SoundmapSound>& getSounds() const { return m_sounds; }

    /// @brief Finds a sound by its id. The id is trimmed and matched exactly.
    /// When more than one sound has the id, the first in file order is returned.
    /// @return The sound, or null if the soundmap has no sound with the id.
    [[nodiscard]] const SoundmapSound* findSoundById(std::string_view soundId) const;

    /// @brief Finds the sound id that `<MapSounds>` assigns to an instrument.
    ///
    /// The instrument UUID is trimmed and matched case-insensitively. When more than one
    /// `<MapSounds>` names the instrument, the first in file order is used. When it lists
    /// more than one `<Sound ref>`, the first is used.
    /// @param instrumentUuid The instrument UUID.
    /// @return The trimmed sound id, or empty if the soundmap does not map the instrument.
    [[nodiscard]] std::string_view findInstrumentSoundRef(std::string_view instrumentUuid) const;

    /// @brief Returns every sound id that `<MapSounds>` assigns to an instrument.
    ///
    /// Each `<MapSounds>` naming the instrument contributes its first `<Sound ref>`, in file order.
    /// The instrument UUID is matched as in #findInstrumentSoundRef.
    /// @param instrumentUuid The instrument UUID.
    /// @return The trimmed sound ids, or an empty list if the soundmap does not map the instrument.
    [[nodiscard]] const std::vector<std::string>& getInstrumentSoundRefs(std::string_view instrumentUuid) const;

private:
    SoundmapDescription m_description;
    std::vector<SoundmapSound> m_sounds;
    std::unordered_map<std::string, size_t> m_soundsById;
    std::unordered_map<std::string, std::vector<std::string>> m_soundRefsByInstrument;
};

/**
 * @struct CatalogInstrument
 * @brief One `<Instrument>` in a supplied instrument catalog.
 *
 * All text values are trimmed.
 */
struct CatalogInstrument
{
    std::string uuid;           ///< The instrument's UUID, as spelled in the catalog. (xml attribute is `uuid`)
    std::string name;           ///< The instrument's internal name. (xml attribute is `name`)
    std::string fullName;       ///< The instrument's full display name. (xml attribute is `fullName`)
    std::string clef;           ///< The instrument's clef name, such as `treble`. (xml attribute is `clef`)
    std::string notation;       ///< The instrument's notation style name, such as `standard`. (xml attribute is `notation`)
    int transposition{};        ///< Sounding pitch relative to written pitch, in semitones. (xml attribute is `transposition`)
};

/**
 * @enum NoteTypeMatch
 * @brief Specifies how a percussion note type is matched against a device note-name list.
 */
enum class NoteTypeMatch
{
    Exact,          ///< Match the full note type, including its order ID.
    ExactThenBase,  ///< Match the full note type. If it is not listed, match its base type (without its order ID).
};

/**
 * @class DeviceNoteList
 * @brief One named note-name list from a supplied MIDI Device Annotation file.
 *
 * Each entry pairs a 0-based MIDI key with a full @ref PercussionNoteTypeId, including any
 * order ID in its top four bits. Rows whose key is outside 0..@ref MAX_MIDI_KEY or whose base
 * note type is not a known percussion note type are omitted.
 */
class DeviceNoteList
{
public:
    /// @brief One row of a note-name list.
    struct Entry
    {
        int midiKey{};                  ///< The 0-based MIDI key. (xml attribute is `Number`)
        PercussionNoteTypeId noteType{};///< The full percussion note type. (xml attribute is `PercNoteType`)
    };

    /// @brief Constructs a note-name list.
    /// @param fileName The final path component of the device annotation file's name.
    /// @param listName The trimmed list name.
    /// @param entries The list's rows, in file order.
    DeviceNoteList(std::string fileName, std::string listName, std::vector<Entry> entries)
        : m_fileName(std::move(fileName)), m_listName(std::move(listName)), m_entries(std::move(entries)) {}

    /// @brief Returns the file name of the device annotation file that contains the list.
    [[nodiscard]] const std::string& getFileName() const { return m_fileName; }

    /// @brief Returns the list's name. (xml attribute is `Name`)
    [[nodiscard]] const std::string& getListName() const { return m_listName; }

    /// @brief Returns the list's rows in file order.
    [[nodiscard]] const std::vector<Entry>& getEntries() const { return m_entries; }

    /// @brief Finds the note type for a MIDI key. When the key appears more than once, the first row wins.
    /// @param midiKey The 0-based MIDI key.
    /// @return The full note type, or no value if the key is not listed.
    [[nodiscard]] std::optional<PercussionNoteTypeId> findNoteType(int midiKey) const;

    /// @brief Finds the MIDI key for a note type.
    ///
    /// When more than one key has the note type, the first in file order is returned.
    /// @param noteType The full percussion note type.
    /// @param match How to match @p noteType.
    /// @return The 0-based MIDI key, or no value if the note type is not listed.
    [[nodiscard]] std::optional<int> findMidiKey(PercussionNoteTypeId noteType, NoteTypeMatch match = NoteTypeMatch::Exact) const;

private:
    std::string m_fileName;
    std::string m_listName;
    std::vector<Entry> m_entries;
};

/**
 * @struct PercussionPlaybackKey
 * @brief The MIDI key that plays a percussion note type, or the reason there is none.
 */
struct PercussionPlaybackKey
{
    std::optional<int> midiKey;                     ///< The 0-based MIDI key, or no value if resolution failed.
    std::optional<PlaybackLookupFailure> failure;   ///< The link that failed. Empty when #midiKey has a value.
    PercussionNoteTypeId matchedNoteType{};         ///< The note type that matched the list: the full or the base note type.
    bool matchedBaseType{};                         ///< True if only the base note type (without its order ID) was listed.
    const SoundmapSound* sound{};                   ///< The resolved sound, or null if resolution failed before it was found.
    const DeviceNoteList* noteList{};               ///< The resolved note-name list, or null if resolution failed before it was found.
};

/**
 * @class PlaybackResources
 * @brief Soundmaps, MIDI Device Annotation note-name lists, and instrument catalog entries supplied by the caller.
 *
 * Create instances with @ref musx::factory::PlaybackResourcesFactory. An instance is immutable and
 * may be shared and used concurrently.
 *
 * Name matching rules:
 * - UUIDs are trimmed and matched case-insensitively.
 * - Sound ids, sound refs, plugin IDs, and list names are trimmed and matched exactly.
 * - Device file names are reduced to their final path component (after the last `/` or `\`) and matched exactly.
 *
 * Duplicates resolve to the first supplied: the first soundmap (in caller order) with a sound UUID,
 * the first sound in a soundmap with an id, and the first device file and list with a given
 * file name and list name.
 *
 * When no resources at all were supplied, every lookup fails with
 * @ref PlaybackLookupFailure::ResourcesNotSupplied.
 */
class PlaybackResources
{
public:
    PlaybackResources(const PlaybackResources&) = delete;               ///< Not copyable, because lookups refer to owned data.
    PlaybackResources& operator=(const PlaybackResources&) = delete;    ///< Not copyable, because lookups refer to owned data.

    /// @brief Returns the supplied soundmaps in caller order.
    [[nodiscard]] const std::vector<Soundmap>& getSoundmaps() const { return m_soundmaps; }

    /// @brief Returns every supplied note-name list, in caller order and then file order.
    [[nodiscard]] const std::vector<DeviceNoteList>& getDeviceNoteLists() const { return m_noteLists; }

    /// @brief Returns every supplied instrument catalog entry, in caller order and then file order.
    [[nodiscard]] const std::vector<CatalogInstrument>& getInstruments() const { return m_instruments; }

    /// @brief Returns true if no soundmaps, no device annotation files, and no instrument catalog entries were supplied.
    [[nodiscard]] bool isEmpty() const
    { return m_soundmaps.empty() && m_deviceFileNames.empty() && m_noteLists.empty() && m_instruments.empty(); }

    /// @brief Finds the first soundmap, in caller order, whose plugin ID matches.
    /// @param pluginId The plugin ID to match against @ref SoundmapDescription::pluginId. An empty
    /// plugin ID matches no soundmap.
    /// @return The soundmap, or @ref PlaybackLookupFailure::SoundmapNotFound.
    [[nodiscard]] PlaybackLookupResult<Soundmap> findSoundmap(std::string_view pluginId) const;

    /// @brief Finds a sound by UUID across all soundmaps.
    /// @return The sound, or @ref PlaybackLookupFailure::SoundNotFound.
    [[nodiscard]] PlaybackLookupResult<SoundmapSound> findSoundByUuid(std::string_view soundUuid) const;

    /// @brief Finds the sound that `<MapSounds>` assigns to an instrument for a playback plugin.
    ///
    /// Soundmaps whose plugin ID matches @p pluginId are searched in caller order. Within each, the
    /// instrument's sound refs are tried in file order (see @ref Soundmap::getInstrumentSoundRefs), and
    /// the first ref that names one of the soundmap's sounds selects that sound. If the selected sound
    /// has a non-empty UUID it is the result; otherwise the search continues with the next soundmap.
    /// @param pluginId The plugin ID to match against @ref SoundmapDescription::pluginId. An empty
    /// plugin ID matches no soundmap.
    /// @param instrumentUuid The instrument UUID.
    /// @return The sound, or @ref PlaybackLookupFailure::SoundNotFound.
    [[nodiscard]] PlaybackLookupResult<SoundmapSound> findMappedSound(std::string_view pluginId, std::string_view instrumentUuid) const;

    /// @brief Finds a note-name list by device file name and list name.
    /// @return The list, @ref PlaybackLookupFailure::DeviceFileNotSupplied, or @ref PlaybackLookupFailure::NoteListNotFound.
    [[nodiscard]] PlaybackLookupResult<DeviceNoteList> findNoteList(std::string_view fileName, std::string_view listName) const;

    /// @brief Finds the note-name list that a sound uses for percussion playback.
    /// @return The list, @ref PlaybackLookupFailure::NoPercussionMidiMap, or a failure from #findNoteList.
    [[nodiscard]] PlaybackLookupResult<DeviceNoteList> resolveNoteList(const SoundmapSound& sound) const;

    /// @brief Resolves the MIDI key that plays a percussion note type with a sound.
    ///
    /// Resolution finds the sound by UUID, then its note-name list, then the note type's key.
    /// @param soundUuid The sound UUID, usually from @ref others::PlaybackRoute::soundUuid.
    /// @param noteType The full percussion note type.
    /// @param match How to match @p noteType.
    /// @return The key, or the failure of the first link that did not resolve.
    [[nodiscard]] PercussionPlaybackKey resolvePercussionMidiKey(std::string_view soundUuid, PercussionNoteTypeId noteType,
        NoteTypeMatch match = NoteTypeMatch::ExactThenBase) const;

    /// @brief Resolves the MIDI key that plays a percussion note type in a note-name list.
    /// @param noteList The note-name list.
    /// @param noteType The full percussion note type.
    /// @param match How to match @p noteType.
    /// @return The key, or @ref PlaybackLookupFailure::NoteTypeNotListed. #PercussionPlaybackKey::sound is null.
    [[nodiscard]] static PercussionPlaybackKey resolvePercussionMidiKey(const DeviceNoteList& noteList, PercussionNoteTypeId noteType,
        NoteTypeMatch match = NoteTypeMatch::ExactThenBase);

    /// @brief Returns the name by which a device annotation file is matched: the trimmed final path component of @p path.
    /// @param path A file name or path, using `/` or `\` separators.
    [[nodiscard]] static std::string_view getDeviceFileName(std::string_view path);

private:
    PlaybackResources(std::vector<Soundmap> soundmaps, std::vector<std::string> deviceFileNames, std::vector<DeviceNoteList> noteLists,
        std::vector<CatalogInstrument> instruments);

    std::vector<Soundmap> m_soundmaps;
    std::vector<DeviceNoteList> m_noteLists;
    std::vector<CatalogInstrument> m_instruments;
    std::unordered_map<std::string, const SoundmapSound*> m_soundsByUuid;
    std::unordered_set<std::string> m_deviceFileNames;
    std::map<std::pair<std::string, std::string>, size_t> m_noteListsByName;

    friend class musx::factory::PlaybackResourcesFactory;
};

} // namespace dom
} // namespace musx
