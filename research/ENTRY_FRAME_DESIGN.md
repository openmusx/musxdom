# Entry-frame tuplet boundary timing

Status: implemented in `GFrameHoldContext::createEntryFrame`. Tests are `GFrameHold.TupletCompletionTruncatesEntry` (`tests/details/gfhold.cpp`), with the `tests/data/tuplet_completion.enigmaxml` fixture.

## Finale reference

The rule comes from Finale 27.4.1 `LTGRAF::ComputeEduPositions` (`../finale-dec/recovered/Finale/arm64/classes/LTGRAF_867187d8.cpp`), plus `ReadTupletArray` / `TupCmpFunc` (`global/block_10075.cpp`) and `LTGRAF::UpdateTupletEnd`.

Finale keeps one tuplet stack per voice. For each slot it tracks:
- symbolic total and reference total;
- the actual span, scaled by the enclosing slots;
- symbolic duration consumed at that level only;
- raw actual time accumulated;
- the start position.

For each non-grace entry:
1. Every open slot accumulates the entry's raw actual duration. Only the innermost slot consumes the entry's symbolic duration.
2. Slots then close from the innermost outward. A slot closes when `consumed >= symTotal` or `accum >= span - 2/64 EDU`.
3. A closing slot snaps the entry's end to `slotStart + span`, which discards the unused display value.
4. The parent consumes exactly the closed child's reference total.
5. Tuplets that start on the same entry are sorted by symbolic total descending, then by reference total descending.

## musxdom implementation

musxdom follows the same algorithm in exact `Fraction` arithmetic, with no 1/64-EDU rounding and no tolerance.

`EntryInfo::untruncatedActualDuration` keeps the pre-snap value, so callers can detect truncated entries. Because a truncated entry never matches its siblings, overfilled tuplets are no longer classified by `TupletInfo::calcIsTremolo`.

Not replicated:
- A grace entry with `tupletStart` creates zero-length tuplet records in Finale. musxdom ignores tuplets on grace entries.
- At each `v2Launch`, both Finale and musxdom clear the open v2 tuplet stack, so open tuplets do not continue into the new v2 sequence. Only the record differs. Finale adds a tuplet to its `TupletRec` list only in `UpdateTupletEnd`, so a v2 tuplet still open at the next `v2Launch` never gets a record. Open tuplets in the frame's last v2 sequence are recorded at frame end. musxdom creates its `TupletInfo` when the tuplet starts and keeps it, ending at the tuplet's last entry.
- Finale rounds positions to integer EDUs.

## Downstream follow-ups
- **MuseScore** (`importfinaleentries.cpp`): done (uncommitted, not yet built). `processEntryInfo` gives a truncated entry the longest `TDuration` that fits the shortened written length (`actualDuration / cumulativeRatio`), so it does not overlap the next chord. It logs info when the fit is exact and a warning otherwise.
- **denigma MNX**: done (uncommitted). A truncated event is written with the note value that fills the tuplet when one exists, and otherwise keeps its written duration with a warning. MusicXML needs no change, because `<duration>` already uses the actual duration.
