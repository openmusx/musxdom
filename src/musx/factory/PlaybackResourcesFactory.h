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
#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "musx/dom/PlaybackResources.h"
#include "musx/xml/XmlInterface.h"

namespace musx {
namespace factory {

/**
 * @struct PlaybackResourceFile
 * @brief A caller-supplied playback resource file: its name and a non-owning view of its bytes.
 *
 * The bytes must remain valid only until @ref PlaybackResourcesFactory::create returns.
 */
struct PlaybackResourceFile
{
    std::string fileName;       ///< The file's name or path. Device annotation files are matched by its final path component.
    const char* data{};         ///< The file's bytes.
    size_t size{};              ///< The number of bytes in #data.

    PlaybackResourceFile() = default;

    /// @brief Constructs from a name and a byte pointer and size.
    PlaybackResourceFile(std::string name, const void* bytes, size_t byteCount)
        : fileName(std::move(name)), data(static_cast<const char*>(bytes)), size(byteCount) {}

    /// @brief Constructs from a name and a contiguous container of bytes, such as `std::vector<char>` or `std::string`.
    template <typename Container, typename = std::enable_if_t<
        sizeof(std::remove_cv_t<typename Container::value_type>) == 1
        && !std::is_same_v<std::remove_cv_t<typename Container::value_type>, bool>>>
    PlaybackResourceFile(std::string name, const Container& bytes)
        : PlaybackResourceFile(std::move(name), bytes.data(), bytes.size()) {}
};

/**
 * @struct PlaybackResourceInputs
 * @brief The files from which @ref PlaybackResourcesFactory::create builds @ref dom::PlaybackResources.
 *
 * The factory does not search for files. These may be found in the standard locations in a Finale installation,
 * where `[version]` is the Finale version, such as `27`:
 *
 * | File type | macOS | Windows |
 * |---|---|---|
 * | Soundmaps (`*.soundmap`, such as `GIFF.soundmap` and `SoftSynth.soundmap`) | `/Library/Application Support/MakeMusic/Finale [version]/Data/` | `C:\Program Files\MakeMusic\Finale\[version]\Data\` |
 * | MIDI Device Annotation files (`*.xml`, such as `Garritan Instruments for Finale.xml`) | `/Library/Application Support/MakeMusic/Finale [version]/MIDI Device Annotation/` | `C:\ProgramData\MakeMusic\Finale [version]\MIDI Device Annotation\` |
 * | Per-user MIDI Device Annotation files | `~/Library/Application Support/MakeMusic/Finale [version]/MIDI Device Annotation/` | `C:\Users\[user name]\AppData\Roaming\MakeMusic\Finale [version]\MIDI Device Annotation\` |
 *
 * The per-user folder holds copies of the shared files and may also hold files that the user or
 * third-party playback software installed. Third-party playback software may install its soundmap
 * elsewhere, such as `/Applications/Garritan Instruments for Finale/GIFF.soundmap` on macOS. Some
 * older versions keep `General MIDI.soundmap` and `SoftSynth.soundmap` inside the application
 * bundle, in `Finale.app/Contents/Resources/` on macOS.
 */
struct PlaybackResourceInputs
{
    /// @brief Soundmap XML files (root `<Soundmap>`), in priority order. Their names are used only in error messages.
    std::vector<PlaybackResourceFile> soundmaps;
    /// @brief MIDI Device Annotation XML files (root `<FinaleNameDocument>`), in priority order.
    /// A file with an empty name cannot be matched and is skipped. A file is matched by the
    /// final component of its name, so a path such as
    /// `/Library/Application Support/MakeMusic/Finale 27/MIDI Device Annotation/Garritan Instruments for Finale.xml`
    /// matches the name `Garritan Instruments for Finale.xml`.
    std::vector<PlaybackResourceFile> deviceAnnotations;
};

/** @brief Creates @ref dom::PlaybackResources from caller-supplied file contents. */
class PlaybackResourcesFactory
{
public:
    /**
     * @brief Parses playback resource files with the selected XML backend.
     *
     * Files with no bytes are skipped. Inputs with no files produce an empty resource object.
     * The returned object does not refer to the input bytes.
     *
     * @tparam XmlDocumentType The @ref xml::IXmlDocument implementation used to parse the files.
     * @param inputs The files to parse.
     * @return The immutable resources.
     * @throws std::invalid_argument If a file is not well-formed XML or has the wrong root element.
     * The message names the file.
     */
    template <typename XmlDocumentType>
    [[nodiscard]] static dom::PlaybackResourcesPtr create(const PlaybackResourceInputs& inputs)
    {
        static_assert(std::is_base_of_v<xml::IXmlDocument, XmlDocumentType>,
            "XmlDocumentType must derive from IXmlDocument.");
        const auto parse = [](const PlaybackResourceFile& file, const char* rootName) -> std::unique_ptr<xml::IXmlDocument> {
            if (!file.data || file.size == 0) {
                return nullptr;
            }
            auto xmlDocument = std::make_unique<XmlDocumentType>();
            try {
                xmlDocument->loadFromBuffer(file.data, file.size);
            } catch (const std::exception& ex) {
                throw std::invalid_argument("Unable to parse playback resource file \"" + file.fileName + "\": " + ex.what());
            }
            validateRoot(*xmlDocument, file, rootName);
            return xmlDocument;
        };
        ParsedFiles soundmaps;
        for (const auto& file : inputs.soundmaps) {
            if (auto xmlDocument = parse(file, SOUNDMAP_ROOT)) {
                soundmaps.emplace_back(file.fileName, std::move(xmlDocument));
            }
        }
        ParsedFiles deviceAnnotations;
        for (const auto& file : inputs.deviceAnnotations) {
            if (auto xmlDocument = parse(file, DEVICE_ANNOTATION_ROOT)) {
                deviceAnnotations.emplace_back(file.fileName, std::move(xmlDocument));
            }
        }
        return createFromXml(soundmaps, deviceAnnotations);
    }

private:
    static constexpr const char* SOUNDMAP_ROOT = "Soundmap";
    static constexpr const char* DEVICE_ANNOTATION_ROOT = "FinaleNameDocument";

    using ParsedFiles = std::vector<std::pair<std::string, std::unique_ptr<xml::IXmlDocument>>>;

    static void validateRoot(const xml::IXmlDocument& xmlDocument, const PlaybackResourceFile& file, const char* rootName);
    static dom::PlaybackResourcesPtr createFromXml(const ParsedFiles& soundmaps, const ParsedFiles& deviceAnnotations);
};

} // namespace factory
} // namespace musx
