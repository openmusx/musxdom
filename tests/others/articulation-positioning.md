# Articulation positioning evidence and validation

## Status

The former note-association helper inferred a note from rounded assignment
offsets and stem direction. That inference is not an adequate position contract.
The replacement `calcHandleStaffPosition` returns a fractional handle coordinate
for supported manual placement. Its reference and offset behavior are backed by
the user-reported Finale observations below; it makes no note-association claim.

## Behavioral evidence

Analyzed Finale 27.4.1.146 arm64 recovery, binary SHA-256
`857a416db19a5ec2548940af32016c2faa9774bbecdcb8b8aba5ac5ca5bd4f4d`.
These are observations from decompilation and targeted static instruction
inspection, not an independently measured specification. No recovered code is
included here.

| Function | Address | Observation |
| --- | --- | --- |
| `ENTREPORT::CreateEntryDataForArticDrawObj` | `100652ae8` | The entry reference coordinates come from its first and last note reports. |
| `ArticDefEdit::SetUseTopNote` | `1009b2ed4` | Identifies the definition flag selecting the reference. |
| `ArticDefDraw::CalcAssignmentVertPosition` | `1009e34c8` | The manual branch adds the assignment offset, then applies outside-staff/staff-line adjustment. |
| `ArticDefDraw::ComputeArticulationPosition` | `1009e5848` | Selects the reference using `useTopNote`, selects the symbol from the computed side, and adds its definition offsets. |
| `ArticDefDraw::DrawArticulation` | `1009e2748` | Stores the drawing origin in the articulation report; copying uses a separate path. |
| `ArticSelection::PositionSelHandle` | `10015cffc` | Converts the stored origin to the primary selection-handle coordinate. |
| `TENTRY::CalcNoteVertPos` | `100acbf28` | Includes separate cross-staff, percussion, and tablature paths. |

The supported manual calculation is the selected reference note's staff position
plus assignment and selected-symbol vertical offsets converted to staff-position
units. Symbol selection precedes adding the definition offset. The user reports
that `defVertPos` is an assignment-time UI default and does not contribute to
an existing assignment's calculated position.

Staff-line count is not an exclusion when outside-staff and staff-line avoidance
are disabled. Coordinate scaling and font scaling are separate in the inspected
paths; neither staff resizing nor fixed-size fonts should be rejected merely
because they change visible dimensions. The user confirmed that staff and entry
size percentages do not change the vertical position.

Automatic placement uses rendered bounds, stems/beams, stacking, and slurs.
There is no established general error bound for a layout-free approximation.
Even an exact handle coordinate does not identify a glyph's visual center or an
associated notehead.

## Symbol-selection audit

`ArticDefBase::IsRefPointAbove` (`1009abadc`) supplies a reference-side policy.
Notehead-side placement remains opposite the stem even when stems are forced.
Auto-note/stem placement uses the notehead side for unforced stems and the forced
stem direction otherwise. Above-entry placement switches below only with the
multilayer modifier, multiple layers, and a forced down stem. Stem-side modes
follow the stem; below-entry stays below.

This policy does not itself determine the rendered symbol. The ordinary drawing
path compares the computed, rounded vertical position with the reference selected
by `useTopNote`; equality selects the above-side mapping. Symbol offsets are added
after that decision. Manual positioning without outside-staff or staff-line
avoidance reduces the comparison to the sign of the assignment offset, even with
a placement override. Character copying bypasses this selection and uses the
main character, main font, and main offsets, regardless of alternate-symbol flags.

`SelectedSymbolContext::symbolIsEstimate` exposes the distinction. It is false
for copying, for ordinary manual selection without those avoidance settings,
and when both sides select the same symbol. Other cases retain a policy-based
prediction. The existing `placement` field remains semantic export policy and
need not match the side used to choose a symbol.

The automatic prediction uses musxdom's forced-stem and multilayer helpers.
Finale uses layout reports for these inputs; in particular its multilayer check
examines loaded entry layers whereas musxdom counts nonzero frame identifiers.
These are further reasons not to treat automatic predictions as exact. Regression
tests cover the inspected rules, not independent Finale rendering measurements.
Denigma can continue consuming the predicted symbol; callers requiring certainty
must inspect the new flag. No downstream classification policy is changed here.

## User-reported Finale measurements

The user supplied the following results for the first manual-position fixture.
Coordinates are EVPU relative to the staff reference line, positive upward.
Reversing or flipping the stem does not change the results.

| `useTopNote` | Assignment vertical offset | Measured handle position |
| --- | ---: | ---: |
| Off | 0 | -96 |
| On | 0 | -48 |
| Off | +24 | -72 |
| On | +24 | -24 |
| Off | -24 | -120 |
| On | -24 | -72 |

These observations independently support additive assignment offsets and a
reference at the bottom or top note selected by `useTopNote`, independent of
stem direction. The user also confirmed that the definition's default vertical
position is only a convenience when creating the assignment.

The requested baseline fixture used standard line spacing and zero main/alternate
symbol offsets. The report did not specify measurement method, precision, or
Finale version. In follow-up testing the user confirmed:

- The symbol's definition Y offset moves the handle, irrespective of glyph.
  Character handles appear to align with the character baseline.
- Neither staff nor entry percentage size changes the vertical position.
- Actual staff line spacing changes how much of a staff space an offset consumes,
  but does not change the zero reference.

Consequently the conversion uses half of the effective staff's actual line
spacing, not a fixed 12 EVPU. Positive odd-valued line spacing retains its
fractional half-space width. The implementation adds assignment and selected
symbol offsets before conversion and performs no rounding to a note position.

The six coordinate rows above are measurements. Additional test values exercise
these reported rules and the inspected symbol-selection behavior; they are not
represented as further measured Finale coordinates.

## Independent measurement protocol

Start with a treble-clef E4/G4/B4 chord, manual vertical positioning, standard
line spacing, no placement override or copying, and outside-staff/staff-line
avoidance disabled. Set both symbol vertical offsets to zero. Measure the primary
handle relative to the staff reference line, positive upward. Retain the saved
assignment/definition data, not just dialog readouts.

Record these cases independently:

- Zero assignment offset, `useTopNote` off/on, both stem directions.
- Positive, negative, zero, and fractional-staff-position assignment offsets.
- Main/alternate offsets, including a symbol offset crossing the reference.
- Different fonts/glyphs and a simple shape with otherwise identical settings.
- Changed `defVertPos` with unchanged saved assignment offsets.
- Different staff-line counts with the reference coordinate held constant.
- Staff, system, and page resizing; entry and individual notehead resizing;
  fixed-size fonts, each varied separately.

For each case record Finale version, settings, saved input, measurement method,
coordinate units, measured handle coordinate, and uncertainty. Prefer coordinate
reports when available. Calibrated high-zoom UI measurements must have uncertainty
no greater than one EVPU. Expected test results must come from these measurements,
not from the candidate formula.

## Validation limits

Require agreement at the coordinate report's precision, or within the recorded UI
measurement uncertainty. Unexplained differences block support for the affected
configuration; do not fit correction constants. If the ordinary manual case
cannot be validated, retain the removal-only outcome.

The API returns a fractional staff position or no result for an
unsupported valid configuration. It declines automatic placement, copying,
placement overrides, outside-staff/staff-line avoidance, slur interactions, cross-staff notation,
rests/empty entries, grace notes, percussion, tablature, notation substitutions, active part voicing,
and vertical notehead alterations. Positive line spacing and arbitrary staff-line
counts are supported. Invalid input is tested separately from unsupported placement.
