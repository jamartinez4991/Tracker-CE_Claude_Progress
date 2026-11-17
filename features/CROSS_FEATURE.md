# Cross-feature map (baseline `ed513015`)

Every feature lives in its own folder (`features/<name>/`: CHANGED_FILES.txt, `<name>.patch`, `files/`, IMPLEMENTATION.md, tests). Nothing was pushed to upstream. Deferred (not implemented): Master Timing Multiplier, Step Jump, Chance.

## Opt-in flags (add to `CUSTOM_DEFINES`)
| Feature | Flag | Depends on |
|---|---|---|
| Mono sampling | `MOD_MONO_SAMPLING` | — |
| Sample instrument FX (sample-rate reduction, DJ filter) | `MOD_SAMPLE_INSTRUMENT_FX` | shared instrument ext block (auto) |
| Live recording toggles | `MOD_LIVE_REC_TOGGLES` | — |
| Lazy chop | `MOD_LAZY_CHOP` | — |
| Extended pitch range | `MOD_PITCH_RANGE` | shared ext block (auto) |
| Signed LFO depth | `MOD_LFO_SIGNED_DEPTH` | canonical LFO stage (auto) |
| LFO polarity | `MOD_LFO_POLARITY` | canonical LFO stage + shared ext block (auto) |

Example (everything): `CUSTOM_DEFINES="-DMOD_MONO_SAMPLING -DMOD_SAMPLE_INSTRUMENT_FX -DMOD_LIVE_REC_TOGGLES -DMOD_LAZY_CHOP -DMOD_PITCH_RANGE -DMOD_LFO_SIGNED_DEPTH -DMOD_LFO_POLARITY"`.
Any subset is valid. Use the same flags for every translation unit (they change `strInstrument`).

## Prerequisites introduced
* **Shared instrument extension block** (`MOD_INSTRUMENT_EXT`, derived in mtStructs.h): 8 bytes appended to `strInstrument` when Sample FX, Pitch Range or Polarity is on → `INSTRUMENT_FILE_VERSION` 10, size 380. Layout is fixed regardless of which flags are on, so files are interchangeable between mod builds. `recoveryInstrumentVersion9` sets every field's legacy value. Identical text is in both patches (sample-instrument-fx was refactored to it; its earlier 376-byte layout no longer exists).
* **Canonical LFO stage** (`MOD_LFO_CANONICAL`): shared by signed depth and polarity.
* `MOD_AMOUNT_SUBPARAMS` (editor Amount-column sub-parameters): Pitch Range and Polarity.
* Reserved for later: Page 3 slot 2 of the pattern editor is left free for the Master Multiplier.

## Files edited by more than one feature (changes do not conflict)
| File | Features |
|---|---|
| core/source/mtStructs.h | sample-instrument-fx, pitch-lfo-extensions (identical shared block; masks/other edits separate) |
| core/FileManager/mtFileManagerOldVersionRecovery.cpp, fileManagerInstrument.cpp | sample-instrument-fx, pitch-lfo-extensions (identical recovery/defaults text; differs only in line endings → trivial merge conflict, take either side) |
| core/interface/instrumentEditor/{instrumentEditor.h,fIinstrEdit.cpp,gInstrEdit.cpp} | sample-instrument-fx (Effects page), pitch-lfo-extensions (Envelope page) |
| core/AudioEngine/mtAudioEngine.{h,cpp} | lazy-chop, sample-instrument-fx, pitch-lfo-extensions |
| core/AudioEngine/mtAudioUpdateHandle.cpp | sample-instrument-fx, pitch-lfo-extensions |
| core/libs/Audio/play_memory.h | lazy-chop (`isReverse`), pitch-lfo-extensions |
| core/source/mtSequencer.{h,cpp} | lazy-chop, live-recording-toggles |

## Verified
A trial merge of all five branches (`git merge`, one trivial EOL-only conflict as above) compiled (`-fsyntax-only`) for original/plus/mini with all flags on and with none, 28 .cpp files, no errors. Not linked; nothing run on hardware.
