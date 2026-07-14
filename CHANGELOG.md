# Changelog

## 1.5.2 — 2026-07-15

- MIDI-mappable host trigger parameters for DICE, SAMPLES, PARAMS, and every slot DICE action
- Immediate voice, Burst, delayed-trigger, and glitch-tail reset on DAW transport changes
- Pitch knobs now reset exactly to 0 st on double-click
- New cyan six-pip die icon for the standalone application
- Separate native Apple Silicon and Intel x86_64 macOS builds

## 1.5.1 — 2026-07-14

- Standalone-only Quick Export with WAV/AIFF format, destination folder, and filename template settings
- Updated BTC and ETH donation addresses in the About window and README
- Added the official [sampledicer.pro](https://sampledicer.pro/) link to the About window

## 1.5.0 — 2026-07-13

- Optional Glitch Mode with per-note inverted colour flashes and regenerated Zalgo text
- Glitch waveform artefacts with randomized delay, stutter, crackle, and sample-hold processing
- Random Round Robin mode that triggers one loaded slot and prevents immediate slot repeats
- Whole-semitone Pitch control by default with Shift-modified 0.1-semitone precision
- Real-time-safe fixed audio buffers, atomic mode parameters, and audio-engine random generation

## 1.4.0 — 2026-07-13

- Options panel with persistent 60%, 100%, 120%, 150%, and 200% interface scale presets
- Twenty persistent colour schemes, including dark and light themes
- Real-time-safe master volume control
- Visual fade-in and fade-out curves with Option/Alt + mouse-wheel curve shaping
- Draggable Start–End waveform ranges that preserve their length and contract at waveform edges
- Mouse-wheel slot volume control from the selected waveform range
- Waveform amplitude display linked to each slot's volume
- Per-slot random sample buttons with animated dice faces
- Per-slot clear buttons
- Theme-aware About and Options panels

## 1.3.0 — 2026-07-13

- Smooth proportional interface resizing from 65% to 200%
- Centred, non-stretching layout when a host requests a non-proportional window size
- Hover highlighting and pointer feedback for Start and Fade markers
- Separate selection of overlapping Start and Fade markers from the waveform's upper and lower halves
- Once-daily background check for newer GitHub releases with an in-plug-in notification
- Centred cryptocurrency donation details in the About window

## 1.2.0 — 2026-07-13

- Start and fade-end markers can no longer cross
- Adjustable fade duration by scrolling over the fade area in the waveform
- Visual fade envelope with duration feedback on every waveform
- Optional KEY chromatic MIDI keytracking relative to note 60
- Donate link and BTC, ETH, and USDT ERC20 details in the About window

## 1.1.0 — 2026-07-12

- Smooth 60 Hz waveform playheads
- Draggable start markers synchronized with plug-in parameters
- Visual waveform shift and draggable fade-end markers with click-free tail trimming
- Per-slot LOCK controls
- Per-trigger parameter randomisation (PTE)
- BACK recall for the previous generated combination
- Real-time-safe trigger snapshots, cached atomic parameter pointers, and a dedicated audio RNG
- Automated Windows x64 VST3 release build

## 1.0.0 — 2026-07-12

- Four sample slots with drag-and-drop and folder navigation
- Waveform display and per-slot volume, pitch, start and shift controls
- Controlled sample and parameter randomisation with DICE
- 1–16 voice polyphony
- Hold-to-repeat Burst mode with adjustable rate
- Persistent last-used sample folder
- VST3, Audio Unit and standalone macOS formats
