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

#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "musx/musx.h"
#include "test_utils.h"
#include "local_playback_resources.h"

using namespace musx::dom;
using musx::factory::PlaybackResourceFile;
using musx::factory::PlaybackResourceInputs;
using musx::factory::PlaybackResourcesFactory;

namespace {

constexpr std::string_view kitUuid = "aaaaaaaa-0000-4000-8000-00000000000a";
constexpr std::string_view kitUuidUpper = "AAAAAAAA-0000-4000-8000-00000000000A";
constexpr std::string_view altKitUuid = "bbbbbbbb-0000-4000-8000-00000000000b";
constexpr std::string_view pianoUuid = "cccccccc-0000-4000-8000-00000000000c";
constexpr std::string_view missingFileUuid = "eeeeeeee-0000-4000-8000-00000000000e";
constexpr std::string_view missingListUuid = "ffffffff-0000-4000-8000-00000000000f";
constexpr std::string_view drumsInstrumentUuid = "11111111-0000-4000-8000-000000000001";
constexpr std::string_view fallbackInstrumentUuid = "22222222-0000-4000-8000-000000000002";

constexpr musxtest::string_view firstSoundmap = R"xml(<?xml version="1.0" encoding="UTF-8"?>
<Soundmap>
  <Description>
    <Name> Test Samples </Name>
    <PluginID>TEST</PluginID>
    <LibraryID>42</LibraryID>
  </Description>
  <Sound id="Kit" uuid=" AAAAAAAA-0000-4000-8000-00000000000A ">
    <Name>Test Kit</Name>
    <PercMIDIMap> Test Kit </PercMIDIMap>
    <PercMIDIMapFile>Test Device.xml</PercMIDIMapFile>
  </Sound>
  <Sound id="Alt Kit" uuid="bbbbbbbb-0000-4000-8000-00000000000b">
    <Name>Alternate Kit</Name>
    <PercMIDIMap>Alt Kit</PercMIDIMap>
    <PercMIDIMapFile>Test Device.xml</PercMIDIMapFile>
  </Sound>
  <Sound id="Piano" uuid="cccccccc-0000-4000-8000-00000000000c">
    <Name>Piano</Name>
    <PatchMIDICommands>
      <ControlChange Control="0" Value="121" />
      <ProgramChange Number="3" />
    </PatchMIDICommands>
  </Sound>
  <Sound id="Kit" uuid="dddddddd-0000-4000-8000-00000000000d">
    <Name>Duplicate Kit Id</Name>
  </Sound>
  <Sound id="Missing File" uuid="eeeeeeee-0000-4000-8000-00000000000e">
    <Name>Missing File</Name>
    <PercMIDIMap>Test Kit</PercMIDIMap>
    <PercMIDIMapFile>Absent Device.xml</PercMIDIMapFile>
  </Sound>
  <Sound id="Missing List" uuid="ffffffff-0000-4000-8000-00000000000f">
    <Name>Missing List</Name>
    <PercMIDIMap>No Such List</PercMIDIMap>
    <PercMIDIMapFile>Test Device.xml</PercMIDIMapFile>
  </Sound>
  <MapSounds>
    <FinInstrument uuid="11111111-0000-4000-8000-000000000001"/>
    <Sound ref=" Kit "/>
    <Sound ref="Piano"/>
  </MapSounds>
  <MapSounds>
    <FinInstrument uuid="11111111-0000-4000-8000-000000000001"/>
    <Sound ref="Piano"/>
  </MapSounds>
  <MapSounds>
    <FinInstrument uuid="22222222-0000-4000-8000-000000000002"/>
    <Sound ref="Nonexistent"/>
  </MapSounds>
</Soundmap>
)xml";

constexpr musxtest::string_view secondSoundmap = R"xml(<?xml version="1.0" encoding="UTF-8"?>
<Soundmap>
  <Description>
    <Name>Second Samples</Name>
    <PluginID>TEST</PluginID>
    <LibraryID>43</LibraryID>
  </Description>
  <Sound id="Duplicate Uuid" uuid="aaaaaaaa-0000-4000-8000-00000000000a">
    <Name>Duplicate Uuid</Name>
  </Sound>
  <Sound id="Fallback" uuid="99999999-0000-4000-8000-000000000009">
    <Name>Fallback Sound</Name>
  </Sound>
  <MapSounds>
    <FinInstrument uuid="22222222-0000-4000-8000-000000000002"/>
    <Sound ref="Fallback"/>
  </MapSounds>
</Soundmap>
)xml";

constexpr musxtest::string_view firstDevice = R"xml(<?xml version="1.0" encoding="UTF-8"?>
<FinaleNameDocument>
  <MasterDeviceNames>
    <NoteNameList Name="Test Kit" DisplayName="Test Kit">
      <Note Number="36" PercNoteType="13" />
      <Note Number="38" PercNoteType="2" />
      <Note Number="40" PercNoteType="2" />
      <Note Number="42" PercNoteType="4260" />
      <Note Number="60" PercNoteType="1" />
      <Note Number="38" PercNoteType="3" />
      <Note Number="200" PercNoteType="5" />
      <Note Number="61" PercNoteType="3000" />
      <Note Number="62" />
      <Note Number="x" PercNoteType="5" />
    </NoteNameList>
    <NoteNameList Name="Test Kit">
      <Note Number="70" PercNoteType="1" />
    </NoteNameList>
    <NoteNameList Name=" Alt Kit ">
      <Note Number="50" PercNoteType="1" />
      <Note Number="51" PercNoteType="2" />
    </NoteNameList>
  </MasterDeviceNames>
</FinaleNameDocument>
)xml";

constexpr musxtest::string_view secondDevice = R"xml(<?xml version="1.0" encoding="UTF-8"?>
<FinaleNameDocument>
  <NoteNameList Name="Test Kit">
    <Note Number="71" PercNoteType="1" />
  </NoteNameList>
  <NoteNameList Name="Extra">
    <Note Number="72" PercNoteType="1" />
  </NoteNameList>
</FinaleNameDocument>
)xml";

PlaybackResourceInputs createTestInputs()
{
    PlaybackResourceInputs inputs;
    inputs.soundmaps.emplace_back("first.soundmap", firstSoundmap);
    inputs.soundmaps.emplace_back("empty.soundmap", std::string{});
    inputs.soundmaps.emplace_back("second.soundmap", secondSoundmap);
    inputs.deviceAnnotations.emplace_back("Devices/Test Device.xml", firstDevice);
    inputs.deviceAnnotations.emplace_back("C:\\Other\\Test Device.xml", secondDevice);
    inputs.deviceAnnotations.emplace_back("", secondDevice);
    return inputs;
}

template <typename XmlDocumentType>
PlaybackResourcesPtr createTestResources()
{
    return PlaybackResourcesFactory::create<XmlDocumentType>(createTestInputs());
}

std::string createErrorMessage(const PlaybackResourceInputs& inputs)
{
    try {
        (void)PlaybackResourcesFactory::create<musx::xml::tinyxml2::Document>(inputs);
    } catch (const std::invalid_argument& ex) {
        return ex.what();
    }
    return {};
}

} // namespace

TEST(PlaybackResourcesTest, EmptyResources)
{
    const auto resources = PlaybackResourcesFactory::create<musx::xml::tinyxml2::Document>({});
    ASSERT_TRUE(resources);
    EXPECT_TRUE(resources->isEmpty());
    EXPECT_TRUE(resources->getSoundmaps().empty());
    EXPECT_TRUE(resources->getDeviceNoteLists().empty());

    const auto sound = resources->findSoundByUuid(kitUuid);
    EXPECT_FALSE(sound);
    EXPECT_EQ(sound.failure, PlaybackLookupFailure::ResourcesNotSupplied);
    EXPECT_EQ(resources->findMappedSound("TEST", drumsInstrumentUuid).failure, PlaybackLookupFailure::ResourcesNotSupplied);
    EXPECT_EQ(resources->findNoteList("Test Device.xml", "Test Kit").failure, PlaybackLookupFailure::ResourcesNotSupplied);
    SoundmapSound percussionSound;
    percussionSound.percussionMidiMap = "Test Kit";
    percussionSound.percussionMidiMapFile = "Test Device.xml";
    EXPECT_EQ(resources->resolveNoteList(percussionSound).failure, PlaybackLookupFailure::ResourcesNotSupplied);
    const auto key = resources->resolvePercussionMidiKey(kitUuid, 1);
    EXPECT_FALSE(key.midiKey);
    EXPECT_EQ(key.failure, PlaybackLookupFailure::ResourcesNotSupplied);
}

TEST(PlaybackResourcesTest, InvalidInputsThrowWithFileName)
{
    {
        PlaybackResourceInputs inputs;
        const std::string broken = "<Soundmap><Sound></Soundmap>";
        inputs.soundmaps.emplace_back("broken.soundmap", broken);
        const auto message = createErrorMessage(inputs);
        EXPECT_NE(message.find("broken.soundmap"), std::string::npos) << message;
    }
    {
        PlaybackResourceInputs inputs;
        inputs.soundmaps.emplace_back("wrong-root.soundmap", firstDevice);
        const auto message = createErrorMessage(inputs);
        EXPECT_NE(message.find("wrong-root.soundmap"), std::string::npos) << message;
        EXPECT_NE(message.find("<Soundmap>"), std::string::npos) << message;
    }
    {
        PlaybackResourceInputs inputs;
        inputs.deviceAnnotations.emplace_back("Wrong Root.xml", firstSoundmap);
        const auto message = createErrorMessage(inputs);
        EXPECT_NE(message.find("Wrong Root.xml"), std::string::npos) << message;
        EXPECT_NE(message.find("<FinaleNameDocument>"), std::string::npos) << message;
    }
}

TEST(PlaybackResourcesTest, SoundmapContents)
{
    const auto resources = createTestResources<musx::xml::pugi::Document>();
    ASSERT_TRUE(resources);
    EXPECT_FALSE(resources->isEmpty());

    const auto& soundmaps = resources->getSoundmaps();
    ASSERT_EQ(soundmaps.size(), 2u);
    EXPECT_EQ(soundmaps[0].getDescription().name, "Test Samples");
    EXPECT_EQ(soundmaps[0].getDescription().pluginId, "TEST");
    EXPECT_EQ(soundmaps[0].getDescription().libraryId, "42");
    EXPECT_EQ(soundmaps[1].getDescription().name, "Second Samples");
    ASSERT_EQ(soundmaps[0].getSounds().size(), 6u);
    EXPECT_EQ(soundmaps[0].getSounds()[0].uuid, kitUuidUpper);

    const auto* duplicateId = soundmaps[0].findSoundById(" Kit ");
    ASSERT_TRUE(duplicateId);
    EXPECT_EQ(duplicateId->name, "Test Kit");

    const auto* piano = soundmaps[0].findSoundById("Piano");
    ASSERT_TRUE(piano);
    EXPECT_EQ(piano->programChange, 3);
    EXPECT_FALSE(duplicateId->programChange);
    EXPECT_EQ(soundmaps[0].findInstrumentSoundRef("11111111-0000-4000-8000-000000000001"), "Kit");
    EXPECT_TRUE(soundmaps[0].findInstrumentSoundRef("33333333-0000-4000-8000-000000000003").empty());
}

TEST(PlaybackResourcesTest, SoundByUuid)
{
    const auto resources = createTestResources<musx::xml::rapidxml::Document>();
    ASSERT_TRUE(resources);

    for (const auto uuid : {kitUuid, kitUuidUpper}) {
        const auto sound = resources->findSoundByUuid(uuid);
        ASSERT_TRUE(sound) << uuid;
        EXPECT_FALSE(sound.failure);
        EXPECT_EQ(sound.value->id, "Kit");
        EXPECT_EQ(sound.value->name, "Test Kit");
        EXPECT_EQ(sound.value->percussionMidiMap, "Test Kit");
        EXPECT_EQ(sound.value->percussionMidiMapFile, "Test Device.xml");
        EXPECT_EQ(sound.value->soundmapIndex, 0u);
    }
    const auto fallback = resources->findSoundByUuid("99999999-0000-4000-8000-000000000009");
    ASSERT_TRUE(fallback);
    EXPECT_EQ(fallback.value->soundmapIndex, 1u);

    const auto missing = resources->findSoundByUuid("12345678-0000-4000-8000-000000000000");
    EXPECT_FALSE(missing);
    EXPECT_EQ(missing.failure, PlaybackLookupFailure::SoundNotFound);
}

TEST(PlaybackResourcesTest, InstrumentToSound)
{
    const auto resources = createTestResources<musx::xml::tinyxml2::Document>();
    ASSERT_TRUE(resources);

    const auto drums = resources->findMappedSound(" TEST ", "11111111-0000-4000-8000-000000000001");
    ASSERT_TRUE(drums);
    EXPECT_EQ(drums.value->id, "Kit");
    EXPECT_EQ(drums.value->uuid, kitUuidUpper);

    const auto fallback = resources->findMappedSound("TEST", fallbackInstrumentUuid);
    ASSERT_TRUE(fallback);
    EXPECT_EQ(fallback.value->id, "Fallback");
    EXPECT_EQ(fallback.value->soundmapIndex, 1u);

    EXPECT_EQ(resources->findMappedSound("OTHR", drumsInstrumentUuid).failure, PlaybackLookupFailure::SoundNotFound);
    EXPECT_EQ(resources->findMappedSound("TEST", "33333333-0000-4000-8000-000000000003").failure, PlaybackLookupFailure::SoundNotFound);
}

TEST(PlaybackResourcesTest, MappedSoundUsesFirstResolvedRef)
{
    constexpr musxtest::string_view firstMap = R"xml(<?xml version="1.0" encoding="UTF-8"?>
<Soundmap>
  <Description><Name>First</Name><PluginID>SKIP</PluginID></Description>
  <Sound id="No Uuid"><Name>No Uuid</Name></Sound>
  <Sound id="Usable" uuid="12121212-0000-4000-8000-000000000012"><Name>Usable</Name></Sound>
  <MapSounds><FinInstrument uuid="11111111-0000-4000-8000-000000000001"/><Sound ref="Missing"/></MapSounds>
  <MapSounds><FinInstrument uuid="11111111-0000-4000-8000-000000000001"/><Sound ref="Usable"/></MapSounds>
  <MapSounds><FinInstrument uuid="22222222-0000-4000-8000-000000000002"/><Sound ref="No Uuid"/></MapSounds>
  <MapSounds><FinInstrument uuid="22222222-0000-4000-8000-000000000002"/><Sound ref="Usable"/></MapSounds>
</Soundmap>
)xml";
    constexpr musxtest::string_view secondMap = R"xml(<?xml version="1.0" encoding="UTF-8"?>
<Soundmap>
  <Description><Name>Second</Name><PluginID>SKIP</PluginID></Description>
  <Sound id="Later" uuid="13131313-0000-4000-8000-000000000013"><Name>Later</Name></Sound>
  <MapSounds><FinInstrument uuid="22222222-0000-4000-8000-000000000002"/><Sound ref="Later"/></MapSounds>
</Soundmap>
)xml";
    PlaybackResourceInputs inputs;
    inputs.soundmaps.emplace_back("first.soundmap", firstMap);
    inputs.soundmaps.emplace_back("second.soundmap", secondMap);
    const auto resources = PlaybackResourcesFactory::create<musx::xml::tinyxml2::Document>(inputs);
    ASSERT_TRUE(resources);

    const auto& firstSoundmapRead = resources->getSoundmaps()[0];
    EXPECT_EQ(firstSoundmapRead.getInstrumentSoundRefs(drumsInstrumentUuid), (std::vector<std::string>{"Missing", "Usable"}));
    EXPECT_EQ(firstSoundmapRead.findInstrumentSoundRef(drumsInstrumentUuid), "Missing");
    EXPECT_TRUE(firstSoundmapRead.getInstrumentSoundRefs("33333333-0000-4000-8000-000000000003").empty());

    // A ref naming no sound is passed over in favor of a later <MapSounds> for the instrument.
    const auto usable = resources->findMappedSound("SKIP", drumsInstrumentUuid);
    ASSERT_TRUE(usable);
    EXPECT_EQ(usable.value->id, "Usable");

    // The first resolved sound has no UUID, so the next soundmap for the plugin supplies the result.
    const auto later = resources->findMappedSound("SKIP", fallbackInstrumentUuid);
    ASSERT_TRUE(later);
    EXPECT_EQ(later.value->id, "Later");
    EXPECT_EQ(later.value->soundmapIndex, 1u);

    EXPECT_EQ(resources->findMappedSound("", drumsInstrumentUuid).failure, PlaybackLookupFailure::SoundNotFound);
}

TEST(PlaybackResourcesTest, SoundmapByPlugin)
{
    const auto resources = createTestResources<musx::xml::tinyxml2::Document>();
    ASSERT_TRUE(resources);

    const auto first = resources->findSoundmap(" TEST ");
    ASSERT_TRUE(first);
    EXPECT_EQ(first.value, &resources->getSoundmaps()[0]);
    EXPECT_EQ(first.value->getDescription().name, "Test Samples");

    const auto missing = resources->findSoundmap("OTHR");
    EXPECT_FALSE(missing);
    EXPECT_EQ(missing.failure, PlaybackLookupFailure::SoundmapNotFound);

    constexpr musxtest::string_view noPluginMap = R"xml(<Soundmap><Description><Name>No Plugin</Name></Description></Soundmap>)xml";
    PlaybackResourceInputs noPluginInputs;
    noPluginInputs.soundmaps.emplace_back("no-plugin.soundmap", noPluginMap);
    const auto noPlugin = PlaybackResourcesFactory::create<musx::xml::tinyxml2::Document>(noPluginInputs);
    EXPECT_EQ(noPlugin->findSoundmap("").failure, PlaybackLookupFailure::SoundmapNotFound);
    EXPECT_EQ(noPlugin->findSoundmap("  ").failure, PlaybackLookupFailure::SoundmapNotFound);

    const auto empty = PlaybackResourcesFactory::create<musx::xml::tinyxml2::Document>({});
    EXPECT_EQ(empty->findSoundmap("TEST").failure, PlaybackLookupFailure::ResourcesNotSupplied);
}

TEST(PlaybackResourcesTest, InstrumentCatalog)
{
    constexpr musxtest::string_view firstCatalog = R"xml(<?xml version="1.0" encoding="UTF-8"?>
<InstrumentCatalog>
  <Instrument uuid=" 33333333-0000-4000-8000-000000000003 " name="Horn" fullName="Horn in F" clef="treble" notation="standard" transposition="-7"/>
  <Instrument uuid="44444444-0000-4000-8000-000000000004" name="Flute" fullName="Flute" clef="treble" notation="standard" transposition="+0"/>
</InstrumentCatalog>
)xml";
    constexpr musxtest::string_view secondCatalog = R"xml(<?xml version="1.0" encoding="UTF-8"?>
<InstrumentCatalog>
  <Instrument uuid="55555555-0000-4000-8000-000000000005" name="Drums" fullName="Drum Set" clef="percussion" notation="percussion" transposition="0"/>
</InstrumentCatalog>
)xml";
    PlaybackResourceInputs inputs;
    inputs.instrumentCatalogs.emplace_back("first.xml", firstCatalog);
    inputs.instrumentCatalogs.emplace_back("empty.xml", std::string{});
    inputs.instrumentCatalogs.emplace_back("second.xml", secondCatalog);
    const auto resources = PlaybackResourcesFactory::create<musx::xml::pugi::Document>(inputs);
    ASSERT_TRUE(resources);
    EXPECT_FALSE(resources->isEmpty());
    EXPECT_TRUE(resources->getSoundmaps().empty());

    const auto& instruments = resources->getInstruments();
    ASSERT_EQ(instruments.size(), 3u);
    EXPECT_EQ(instruments[0].uuid, "33333333-0000-4000-8000-000000000003");
    EXPECT_EQ(instruments[0].name, "Horn");
    EXPECT_EQ(instruments[0].fullName, "Horn in F");
    EXPECT_EQ(instruments[0].clef, "treble");
    EXPECT_EQ(instruments[0].notation, "standard");
    EXPECT_EQ(instruments[0].transposition, -7);
    EXPECT_EQ(instruments[1].transposition, 0);
    EXPECT_EQ(instruments[2].fullName, "Drum Set");
    EXPECT_EQ(resources->findSoundByUuid(kitUuid).failure, PlaybackLookupFailure::SoundNotFound);

    {
        PlaybackResourceInputs invalid;
        invalid.instrumentCatalogs.emplace_back("wrong-root.xml", firstDevice);
        const auto message = createErrorMessage(invalid);
        EXPECT_NE(message.find("wrong-root.xml"), std::string::npos) << message;
        EXPECT_NE(message.find("<InstrumentCatalog>"), std::string::npos) << message;
    }
    {
        PlaybackResourceInputs invalid;
        const std::string catalog = R"(<InstrumentCatalog><Instrument uuid="x" name="n" clef="treble" notation="standard" transposition="0"/></InstrumentCatalog>)";
        invalid.instrumentCatalogs.emplace_back("missing.xml", catalog);
        const auto message = createErrorMessage(invalid);
        EXPECT_NE(message.find("missing.xml"), std::string::npos) << message;
        EXPECT_NE(message.find("fullName"), std::string::npos) << message;
    }
    {
        PlaybackResourceInputs invalid;
        const std::string catalog = R"(<InstrumentCatalog><Instrument uuid="x" name="n" fullName="f" clef="treble" notation="standard" transposition="up"/></InstrumentCatalog>)";
        invalid.instrumentCatalogs.emplace_back("transposition.xml", catalog);
        const auto message = createErrorMessage(invalid);
        EXPECT_NE(message.find("transposition.xml"), std::string::npos) << message;
        EXPECT_NE(message.find("transposition"), std::string::npos) << message;
    }
}

TEST(PlaybackResourcesTest, DeviceNoteLists)
{
    const auto resources = createTestResources<musx::xml::tinyxml2::Document>();
    ASSERT_TRUE(resources);

    const auto& lists = resources->getDeviceNoteLists();
    ASSERT_EQ(lists.size(), 5u);
    EXPECT_EQ(lists[0].getFileName(), "Test Device.xml");
    EXPECT_EQ(lists[0].getListName(), "Test Kit");
    EXPECT_EQ(lists[2].getListName(), "Alt Kit");
    EXPECT_EQ(lists[4].getListName(), "Extra");

    const auto list = resources->findNoteList("Test Device.xml", " Test Kit ");
    ASSERT_TRUE(list);
    EXPECT_EQ(list.value, &lists[0]);
    const std::vector<std::pair<int, PercussionNoteTypeId>> expectedEntries = {{36, 13}, {38, 2}, {40, 2}, {42, 4260}, {60, 1}, {38, 3}};
    ASSERT_EQ(list.value->getEntries().size(), expectedEntries.size());
    for (size_t index = 0; index < expectedEntries.size(); ++index) {
        EXPECT_EQ(list.value->getEntries()[index].midiKey, expectedEntries[index].first);
        EXPECT_EQ(list.value->getEntries()[index].noteType, expectedEntries[index].second);
    }

    EXPECT_EQ(list.value->findNoteType(38), PercussionNoteTypeId(2));
    EXPECT_EQ(list.value->findNoteType(42), PercussionNoteTypeId(4260));
    EXPECT_FALSE(list.value->findNoteType(61));

    EXPECT_EQ(list.value->findMidiKey(2), 38);
    EXPECT_EQ(list.value->findMidiKey(4260), 42);
    EXPECT_FALSE(list.value->findMidiKey(164));
    EXPECT_FALSE(list.value->findMidiKey(4097));
    EXPECT_EQ(list.value->findMidiKey(4097, NoteTypeMatch::ExactThenBase), 60);
    EXPECT_FALSE(list.value->findMidiKey(5, NoteTypeMatch::ExactThenBase));

    EXPECT_TRUE(resources->findNoteList("/Some/Path/Test Device.xml", "Alt Kit"));
    EXPECT_TRUE(resources->findNoteList("Test Device.xml", "Extra"));
    EXPECT_EQ(resources->findNoteList("test device.xml", "Test Kit").failure, PlaybackLookupFailure::DeviceFileNotSupplied);
    EXPECT_EQ(resources->findNoteList("Absent Device.xml", "Test Kit").failure, PlaybackLookupFailure::DeviceFileNotSupplied);
    EXPECT_EQ(resources->findNoteList("Test Device.xml", "No Such List").failure, PlaybackLookupFailure::NoteListNotFound);

    constexpr musxtest::string_view noListsDevice = R"xml(<FinaleNameDocument><Author>Test</Author></FinaleNameDocument>)xml";
    PlaybackResourceInputs noListsInputs;
    noListsInputs.deviceAnnotations.emplace_back("Lists/No Lists.xml", noListsDevice);
    const auto noLists = PlaybackResourcesFactory::create<musx::xml::tinyxml2::Document>(noListsInputs);
    EXPECT_FALSE(noLists->isEmpty());
    EXPECT_TRUE(noLists->getDeviceNoteLists().empty());
    EXPECT_EQ(noLists->findNoteList("No Lists.xml", "Test Kit").failure, PlaybackLookupFailure::NoteListNotFound);
    EXPECT_EQ(noLists->findNoteList("Other.xml", "Test Kit").failure, PlaybackLookupFailure::DeviceFileNotSupplied);
}

TEST(PlaybackResourcesTest, PercussionMidiKey)
{
    const auto resources = createTestResources<musx::xml::tinyxml2::Document>();
    ASSERT_TRUE(resources);

    const auto snare = resources->resolvePercussionMidiKey(kitUuidUpper, 1);
    EXPECT_EQ(snare.midiKey, 60);
    EXPECT_FALSE(snare.failure);
    EXPECT_EQ(snare.matchedNoteType, PercussionNoteTypeId(1));
    EXPECT_FALSE(snare.matchedBaseType);
    ASSERT_TRUE(snare.sound);
    EXPECT_EQ(snare.sound->id, "Kit");
    ASSERT_TRUE(snare.noteList);
    EXPECT_EQ(snare.noteList->getListName(), "Test Kit");

    const auto snareCopy = resources->resolvePercussionMidiKey(kitUuid, 4097);
    EXPECT_EQ(snareCopy.midiKey, 60);
    EXPECT_EQ(snareCopy.matchedNoteType, PercussionNoteTypeId(1));
    EXPECT_TRUE(snareCopy.matchedBaseType);

    const auto exactCopy = resources->resolvePercussionMidiKey(kitUuid, 4097, NoteTypeMatch::Exact);
    EXPECT_FALSE(exactCopy.midiKey);
    EXPECT_EQ(exactCopy.failure, PlaybackLookupFailure::NoteTypeNotListed);

    const auto crashCopy = resources->resolvePercussionMidiKey(kitUuid, 4260);
    EXPECT_EQ(crashCopy.midiKey, 42);
    EXPECT_FALSE(crashCopy.matchedBaseType);

    EXPECT_EQ(resources->resolvePercussionMidiKey(altKitUuid, 1).midiKey, 50);

    const auto unlisted = resources->resolvePercussionMidiKey(kitUuid, 5);
    EXPECT_FALSE(unlisted.midiKey);
    EXPECT_EQ(unlisted.failure, PlaybackLookupFailure::NoteTypeNotListed);
    EXPECT_TRUE(unlisted.sound);
    EXPECT_TRUE(unlisted.noteList);

    EXPECT_EQ(resources->resolvePercussionMidiKey("", 1).failure, PlaybackLookupFailure::NoSoundUuid);
    EXPECT_EQ(resources->resolvePercussionMidiKey("12345678-0000-4000-8000-000000000000", 1).failure, PlaybackLookupFailure::SoundNotFound);
    const auto noMap = resources->resolvePercussionMidiKey(pianoUuid, 1);
    EXPECT_EQ(noMap.failure, PlaybackLookupFailure::NoPercussionMidiMap);
    EXPECT_TRUE(noMap.sound);
    EXPECT_FALSE(noMap.noteList);
    EXPECT_EQ(resources->resolvePercussionMidiKey(missingFileUuid, 1).failure, PlaybackLookupFailure::DeviceFileNotSupplied);
    EXPECT_EQ(resources->resolvePercussionMidiKey(missingListUuid, 1).failure, PlaybackLookupFailure::NoteListNotFound);
}

namespace {

constexpr int scoreRouteIndex = 0;
constexpr int routeWithoutSoundIndex = 1;
constexpr int alternateKitRouteIndex = 2;
constexpr int missingRouteIndex = 5;

/// Loads drumset.enigmaxml with staff 1's score route set to the test kit, plus an unshared
/// part 1 play data whose layer 1 uses @p partRouteIndex.
std::vector<char> createDrumsetXml(int partRouteIndex)
{
    std::vector<char> contents;
    musxtest::readFile(musxtest::getInputPath() / "drumset.enigmaxml", contents);
    std::string xml(contents.begin(), contents.end());
    const std::string scoreSound = "<soundUUID>db30b478-5871-3ba5-9553-6f964bb3aeb7</soundUUID>";
    const auto soundPos = xml.find(scoreSound);
    EXPECT_NE(soundPos, std::string::npos);
    if (soundPos != std::string::npos) {
        xml.replace(soundPos, scoreSound.size(), "<soundUUID>" + std::string(kitUuidUpper) + "</soundUUID>");
    }
    const std::string anchor = "<playbackRouteName cmper=\"1\">";
    const auto anchorPos = xml.find(anchor);
    EXPECT_NE(anchorPos, std::string::npos);
    if (anchorPos != std::string::npos) {
        xml.insert(anchorPos,
            "<playbackRoute cmper=\"3\"><soundUUID>" + std::string(altKitUuid) + "</soundUUID></playbackRoute>\n"
            "    <staffPlayData cmper=\"1\" part=\"1\" shared=\"false\"><layer1><playbackRouteNum>"
            + std::to_string(partRouteIndex) + "</playbackRouteNum><play/></layer1></staffPlayData>\n    ");
    }
    return std::vector<char>(xml.begin(), xml.end());
}

DocumentPtr createDrumsetDocument(int partRouteIndex, PlaybackResourcesPtr resources)
{
    musx::factory::DocumentFactory::CreateOptions options;
    options.playbackResources = std::move(resources);
    return musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(createDrumsetXml(partRouteIndex), std::move(options));
}

std::vector<PercussionPlaybackKey> calcLayerOneKeys(const DocumentPtr& doc, Cmper partId)
{
    std::vector<PercussionPlaybackKey> result;
    auto gfHold = details::GFrameHoldContext(doc, partId, 1, 1);
    EXPECT_TRUE(gfHold);
    if (!gfHold) {
        return result;
    }
    auto entryFrame = gfHold.createEntryFrame(0);
    EXPECT_TRUE(entryFrame);
    if (!entryFrame) {
        return result;
    }
    for (size_t index = 0; index < entryFrame->getEntries().size(); ++index) {
        result.push_back(NoteInfoPtr(EntryInfoPtr(entryFrame, index), 0).calcPercussionPlaybackKey());
    }
    return result;
}

} // namespace

TEST(PercussionPlaybackKeyTest, ScoreAndLinkedPart)
{
    const auto resources = createTestResources<musx::xml::tinyxml2::Document>();
    const auto doc = createDrumsetDocument(alternateKitRouteIndex, resources);
    ASSERT_TRUE(doc);
    EXPECT_EQ(doc->getPlaybackResources(), resources);

    const auto score = calcLayerOneKeys(doc, SCORE_PARTID);
    ASSERT_EQ(score.size(), 3u);
    EXPECT_EQ(score[0].midiKey, 38); // Kick Drum
    ASSERT_TRUE(score[0].noteList);
    EXPECT_EQ(score[0].noteList->getListName(), "Test Kit");
    EXPECT_FALSE(score[1].midiKey);
    EXPECT_EQ(score[1].failure, PlaybackLookupFailure::NoteTypeNotListed);
    EXPECT_EQ(score[2].failure, PlaybackLookupFailure::NotPercussionNote);

    const auto part = calcLayerOneKeys(doc, 1);
    ASSERT_EQ(part.size(), 3u);
    EXPECT_EQ(part[0].midiKey, 51);
    ASSERT_TRUE(part[0].noteList);
    EXPECT_EQ(part[0].noteList->getListName(), "Alt Kit");
    EXPECT_EQ(part[1].failure, PlaybackLookupFailure::NoteTypeNotListed);
    EXPECT_EQ(part[2].failure, PlaybackLookupFailure::NotPercussionNote);
}

TEST(PercussionPlaybackKeyTest, RouteFailures)
{
    const auto resources = createTestResources<musx::xml::tinyxml2::Document>();
    {
        const auto doc = createDrumsetDocument(routeWithoutSoundIndex, resources);
        ASSERT_TRUE(doc);
        const auto part = calcLayerOneKeys(doc, 1);
        ASSERT_FALSE(part.empty());
        EXPECT_EQ(part[0].failure, PlaybackLookupFailure::NoSoundUuid);
    }
    {
        const auto doc = createDrumsetDocument(missingRouteIndex, resources);
        ASSERT_TRUE(doc);
        const auto part = calcLayerOneKeys(doc, 1);
        ASSERT_FALSE(part.empty());
        EXPECT_EQ(part[0].failure, PlaybackLookupFailure::NoPlaybackRoute);
        const auto score = calcLayerOneKeys(doc, SCORE_PARTID);
        ASSERT_FALSE(score.empty());
        EXPECT_EQ(score[0].midiKey, 38);
    }
}

TEST(PercussionPlaybackKeyTest, ResourcesNotSupplied)
{
    for (const auto& resources : {PlaybackResourcesPtr{}, PlaybackResourcesFactory::create<musx::xml::tinyxml2::Document>({})}) {
        const auto doc = createDrumsetDocument(scoreRouteIndex, resources);
        ASSERT_TRUE(doc);
        for (const auto& key : calcLayerOneKeys(doc, SCORE_PARTID)) {
            EXPECT_FALSE(key.midiKey);
            EXPECT_EQ(key.failure, PlaybackLookupFailure::ResourcesNotSupplied);
        }
    }
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "drumset.enigmaxml", xml);
    const auto plainDoc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
    ASSERT_TRUE(plainDoc);
    EXPECT_FALSE(plainDoc->getPlaybackResources());
}

TEST(PlaybackResourcesTest, LocalFinaleResources)
{
    if (musxtest::localPlaybackResourceFiles.empty()) {
        GTEST_SKIP() << "Set MUSX_LOCAL_PLAYBACK_RESOURCES to GIFF.soundmap and \"Garritan Instruments for Finale.xml\" to run this test.";
    }
    std::vector<std::vector<char>> contents;
    contents.reserve(musxtest::localPlaybackResourceFiles.size());
    PlaybackResourceInputs inputs;
    for (const auto path : musxtest::localPlaybackResourceFiles) {
        const std::filesystem::path filePath{std::string(path)};
        contents.emplace_back();
        musxtest::readFile(filePath, contents.back());
        ASSERT_FALSE(contents.back().empty()) << path;
        auto& files = filePath.extension() == ".soundmap" ? inputs.soundmaps : inputs.deviceAnnotations;
        files.emplace_back(musxtest::pathString(filePath.filename()), contents.back());
    }
    const auto resources = PlaybackResourcesFactory::create<musx::xml::pugi::Document>(inputs);
    ASSERT_TRUE(resources);

    constexpr std::string_view percussionSoundUuid = "2ec77132-cf8d-3dd9-a932-9c4f623941ef";
    const auto sound = resources->findSoundByUuid(percussionSoundUuid);
    ASSERT_TRUE(sound);
    EXPECT_EQ(sound.value->id, "Percussion");
    EXPECT_EQ(sound.value->percussionMidiMap, "Basic Orch Percussion");
    EXPECT_EQ(sound.value->percussionMidiMapFile, "Garritan Instruments for Finale.xml");
    EXPECT_EQ(resources->getSoundmaps()[sound.value->soundmapIndex].getDescription().pluginId, "GARA");

    const auto noteList = resources->resolveNoteList(*sound.value);
    ASSERT_TRUE(noteList);
    EXPECT_EQ(noteList.value->getListName(), "Basic Orch Percussion");

    constexpr PercussionNoteTypeId snareDrum = 1;
    constexpr PercussionNoteTypeId bassDrum = 13;
    constexpr PercussionNoteTypeId kickDrum = 2;
    EXPECT_EQ(resources->resolvePercussionMidiKey(percussionSoundUuid, snareDrum).midiKey, 60);
    EXPECT_EQ(resources->resolvePercussionMidiKey(percussionSoundUuid, bassDrum).midiKey, 36);
    const auto unlisted = resources->resolvePercussionMidiKey(percussionSoundUuid, kickDrum);
    EXPECT_FALSE(unlisted.midiKey);
    EXPECT_EQ(unlisted.failure, PlaybackLookupFailure::NoteTypeNotListed);

    const auto mixedCase = resources->findSoundByUuid("2194f875-774e-484f-9caa-532447400938");
    ASSERT_TRUE(mixedCase);
    EXPECT_EQ(mixedCase.value->id, "Percussion [single instrument]");

    const auto mapped = resources->findMappedSound("GARA", musx::dom::uuid::PercussionGeneral);
    ASSERT_TRUE(mapped);
    EXPECT_EQ(mapped.value->id, "Percussion");
    EXPECT_EQ(mapped.value->uuid, percussionSoundUuid);
}
