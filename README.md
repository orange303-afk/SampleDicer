# Sample Dicer

Four-layer one-shot sampler for AU, VST3 and standalone use. One MIDI note triggers all four layers. **DICE** selects samples from their source folders and randomises volume, pitch, start point and delay.

<img width="815" height="911" alt="image" src="https://github.com/user-attachments/assets/3a3005f0-bb10-4a8f-ac0b-aacb39a5d0fd" />


## Prototype features

- Four drag-and-drop sample slots with waveform display and previous/next folder navigation
- Refined arc-style controls with parameter units
- Per-slot volume, pitch, start and positive shift (0–250 ms)
- MIDI note-on triggering
- Selectable 1–16 voice polyphony
- Hold-to-repeat Burst mode with adjustable 1–30 Hz rate
- Separate DICE, SAMPLES and PARAMS actions
- Four global randomisation-depth controls
- DAW state restoration for parameters and sample paths
- Last-used sample folder is remembered globally; DICE can populate empty slots from it

## Build (macOS)

Install full Xcode, CMake 3.22+ and clone JUCE 8 into `JUCE/`, then run:

```sh
cmake -S . -B build -G Xcode
cmake --build build --config Release
```

The plug-ins are created below `build/SampleDicer_artefacts/Release/`; automatic copying into system plug-in folders is disabled.

## Next milestone

Add per-parameter min/max ranges and locks, slot locks, waveform/start-point editing, polyphony, click-free fades, undo/redo for DICE, embedded sample assets and automated plug-in validation.
