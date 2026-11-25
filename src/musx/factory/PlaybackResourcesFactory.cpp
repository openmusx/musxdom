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
#include "musx/factory/PlaybackResourcesFactory.h"

#include <limits>
#include <optional>

#include "musx/dom/PercussionNoteType.h"

namespace musx {
namespace factory {

namespace {

std::string childText(const xml::XmlElementPtr& parent, const char* tagName)
{
    const auto child = parent ? parent->getFirstChildElement(tagName) : nullptr;
    return child ? child->getTextTrimmed() : std::string{};
}

std::string attributeText(const xml::XmlElementPtr& element, const char* name)
{
    const auto attribute = element ? element->findAttribute(name) : nullptr;
    return attribute ? attribute->getValueTrimmed() : std::string{};
}

std::optional<unsigned long> parseUnsigned(const std::string& text)
{
    if (text.empty() || text.find_first_not_of("0123456789") != std::string::npos) {
        return std::nullopt;
    }
    try {
        return std::stoul(text);
    } catch (const std::out_of_range&) {
        return std::nullopt;
    }
}

dom::Soundmap parseSoundmap(const xml::XmlElementPtr& root, size_t soundmapIndex)
{
    const auto descriptionElement = root->getFirstChildElement("Description");
    dom::SoundmapDescription description;
    description.name = childText(descriptionElement, "Name");
    description.pluginId = childText(descriptionElement, "PluginID");
    description.libraryId = childText(descriptionElement, "LibraryID");

    std::vector<dom::SoundmapSound> sounds;
    for (auto element = root->getFirstChildElement("Sound"); element; element = element->getNextSibling("Sound")) {
        dom::SoundmapSound sound;
        sound.id = attributeText(element, "id");
        sound.uuid = attributeText(element, "uuid");
        sound.name = childText(element, "Name");
        sound.percussionMidiMap = childText(element, "PercMIDIMap");
        sound.percussionMidiMapFile = childText(element, "PercMIDIMapFile");
        const auto commands = element->getFirstChildElement("PatchMIDICommands");
        const auto programChange = commands ? commands->getFirstChildElement("ProgramChange") : nullptr;
        if (const auto program = parseUnsigned(attributeText(programChange, "Number"));
            program && *program <= static_cast<unsigned long>((std::numeric_limits<int>::max)())) {
            sound.programChange = static_cast<int>(*program);
        }
        sound.soundmapIndex = soundmapIndex;
        sounds.push_back(std::move(sound));
    }

    std::vector<std::pair<std::string, std::string>> instrumentSoundRefs;
    for (auto mapping = root->getFirstChildElement("MapSounds"); mapping; mapping = mapping->getNextSibling("MapSounds")) {
        auto instrumentUuid = attributeText(mapping->getFirstChildElement("FinInstrument"), "uuid");
        auto soundRef = attributeText(mapping->getFirstChildElement("Sound"), "ref");
        if (!instrumentUuid.empty() && !soundRef.empty()) {
            instrumentSoundRefs.emplace_back(std::move(instrumentUuid), std::move(soundRef));
        }
    }
    return dom::Soundmap(std::move(description), std::move(sounds), instrumentSoundRefs);
}

bool isKnownNoteType(dom::PercussionNoteTypeId noteType)
{
    const auto baseType = dom::PercussionNoteTypeId(noteType & dom::percussion::BASE_NOTE_TYPE_MASK);
    return dom::percussion::getPercussionNoteTypeFromId(baseType).instrumentId == int(baseType);
}

std::vector<dom::DeviceNoteList::Entry> parseNoteListEntries(const xml::XmlElementPtr& list)
{
    std::vector<dom::DeviceNoteList::Entry> result;
    for (auto note = list->getFirstChildElement("Note"); note; note = note->getNextSibling("Note")) {
        const auto key = parseUnsigned(attributeText(note, "Number"));
        const auto noteType = parseUnsigned(attributeText(note, "PercNoteType"));
        if (!key || !noteType || *key > static_cast<unsigned long>(dom::MAX_MIDI_KEY)
            || *noteType > (std::numeric_limits<dom::PercussionNoteTypeId>::max)()) {
            continue;
        }
        const auto typeId = dom::PercussionNoteTypeId(*noteType);
        if (!isKnownNoteType(typeId)) {
            continue;
        }
        result.push_back({static_cast<int>(*key), typeId});
    }
    return result;
}

void collectNoteLists(const xml::XmlElementPtr& element, const std::string& fileName, std::vector<dom::DeviceNoteList>& result)
{
    if (element->getTagName() == "NoteNameList") {
        if (auto name = attributeText(element, "Name"); !name.empty()) {
            result.emplace_back(fileName, std::move(name), parseNoteListEntries(element));
        }
        return;
    }
    for (auto child = element->getFirstChildElement(); child; child = child->getNextSibling()) {
        collectNoteLists(child, fileName, result);
    }
}

std::optional<int> parseInt(const std::string& text)
{
    const bool signedText = !text.empty() && (text.front() == '-' || text.front() == '+');
    const auto digits = signedText ? text.substr(1) : text;
    if (digits.empty() || digits.find_first_not_of("0123456789") != std::string::npos) {
        return std::nullopt;
    }
    try {
        return std::stoi(text);
    } catch (const std::out_of_range&) {
        return std::nullopt;
    }
}

void collectInstruments(const xml::XmlElementPtr& root, const std::string& fileName, std::vector<dom::CatalogInstrument>& result)
{
    for (auto element = root->getFirstChildElement("Instrument"); element; element = element->getNextSibling("Instrument")) {
        const auto required = [&](const char* name) {
            auto value = attributeText(element, name);
            if (value.empty()) {
                throw std::invalid_argument("Instrument catalog file \"" + fileName + "\" has an <Instrument> without " + name + ".");
            }
            return value;
        };
        dom::CatalogInstrument instrument;
        instrument.uuid = required("uuid");
        instrument.name = required("name");
        instrument.fullName = required("fullName");
        instrument.clef = required("clef");
        instrument.notation = required("notation");
        const auto transposition = parseInt(required("transposition"));
        if (!transposition) {
            throw std::invalid_argument("Instrument catalog file \"" + fileName
                + "\" has an <Instrument> whose transposition is not an integer.");
        }
        instrument.transposition = *transposition;
        result.push_back(std::move(instrument));
    }
}

} // namespace

void PlaybackResourcesFactory::validateRoot(const xml::IXmlDocument& xmlDocument, const PlaybackResourceFile& file, const char* rootName)
{
    const auto root = xmlDocument.getRootElement();
    if (!root || root->getTagName() != rootName) {
        throw std::invalid_argument("Playback resource file \"" + file.fileName + "\" must have a <" + rootName + "> root element.");
    }
}

dom::PlaybackResourcesPtr PlaybackResourcesFactory::createFromXml(const ParsedFiles& soundmaps, const ParsedFiles& deviceAnnotations,
    const ParsedFiles& instrumentCatalogs)
{
    std::vector<dom::Soundmap> parsedSoundmaps;
    parsedSoundmaps.reserve(soundmaps.size());
    for (const auto& soundmap : soundmaps) {
        parsedSoundmaps.push_back(parseSoundmap(soundmap.second->getRootElement(), parsedSoundmaps.size()));
    }
    std::vector<std::string> deviceFileNames;
    std::vector<dom::DeviceNoteList> noteLists;
    for (const auto& [fileName, xmlDocument] : deviceAnnotations) {
        const std::string deviceFileName(dom::PlaybackResources::getDeviceFileName(fileName));
        if (deviceFileName.empty()) {
            continue;
        }
        deviceFileNames.push_back(deviceFileName);
        collectNoteLists(xmlDocument->getRootElement(), deviceFileName, noteLists);
    }
    std::vector<dom::CatalogInstrument> instruments;
    for (const auto& [fileName, xmlDocument] : instrumentCatalogs) {
        collectInstruments(xmlDocument->getRootElement(), fileName, instruments);
    }
    return dom::PlaybackResourcesPtr(
        new dom::PlaybackResources(std::move(parsedSoundmaps), std::move(deviceFileNames), std::move(noteLists), std::move(instruments)));
}

} // namespace factory
} // namespace musx
