# Sample Dicer

Four-layer one-shot sampler for AU, VST3 and standalone use. One MIDI note normally triggers all four layers, or one non-repeating random layer in Round Robin mode. **DICE** selects samples from their source folders and randomises volume, pitch, start point and delay.

<img width="1224" height="1310" alt="image" src="https://github.com/user-attachments/assets/2f367d59-bc23-4f26-b22c-51aff6c1439f" />


## Features

- Four drag-and-drop sample slots with waveform display and previous/next folder navigation
- Smooth proportional interface resizing from 60% to 200%, plus persistent scale presets
- Twenty persistent colour schemes with dark, light and colourful variations
- Optional Glitch Mode for unpredictable audio and visual variations
- Refined arc-style controls with parameter units
- Per-slot volume, whole-semitone pitch with Shift fine mode (0.1 st), draggable start point,
  positive shift (0–250 ms), and fade-end controls
- Smooth real-time playheads and visual start, shift, and fade editing on every waveform
- Hover-highlighted Start and Fade markers
- Non-crossing Start/Fade markers, draggable range selection, and mouse-wheel fade editing
- Option/Alt + mouse wheel adjusts fade-in and fade-out curve shape
- Mouse-wheel slot volume editing with matching waveform amplitude scaling
- MIDI note-on triggering
- Selectable 1–16 voice polyphony
- Hold-to-repeat Burst mode with adjustable 1–30 Hz rate
- Per-trigger parameter evolution (PTE) with a real-time-safe audio path
- Optional chromatic MIDI keytracking relative to note 60
- Random Round Robin mode plays one loaded slot per trigger without immediate slot repeats
- Per-slot LOCK controls to exclude layers from DICE and PTE
- Per-slot random-sample and clear controls
- Separate DICE, SAMPLES and PARAMS actions
- BACK recalls the previous generated combination
- Four global randomisation-depth controls
- Real-time-safe master volume control
- DAW state restoration for parameters and sample paths
- Last-used sample folder is remembered globally; DICE can populate empty slots from it
- Once-daily background check for newer GitHub releases with an in-plug-in notification

## Use Cases

- **Game development** — create non-repetitive gunshots, explosions, impacts, magical effects, creature sounds, and foley
- **Film, video, and podcasts** — quickly build evolving background effects such as footsteps, crowds, battles, machinery, and environmental textures
- **Music production** — generate glitch textures, IDM variations, layered percussion, fills, transitions, and constantly changing one-shots
- **Sound design** — combine and randomize multiple layers to discover new impacts, weapons, interfaces, and cinematic effects
- **Live performance** — trigger controlled variations, bursts, and round-robin patterns without preparing every variation in advance
- **Rapid prototyping** — turn a folder of raw samples into playable, varied audio while testing a game, scene, or musical idea
- **Sample-library creation** — produce batches of related variations from the same source material while keeping pitch, timing, level, and fades within defined limits

## Build (macOS)

Install full Xcode, CMake 3.22+ and clone JUCE 8 into `JUCE/`, then run:

```sh
cmake -S . -B build -G Xcode
cmake --build build --config Release
```

The plug-ins are created below `build/SampleDicer_artefacts/Release/`; automatic copying into system plug-in folders is disabled.

## Downloads

Ready-to-use macOS VST3, Audio Unit, standalone, and Windows x64 VST3 builds are available on the [Releases](https://github.com/orange303-afk/SampleDicer/releases) page.

## Manual

[Sample_Dicer_Manual_EN_v1.5.0.pdf](https://github.com/user-attachments/files/29986770/Sample_Dicer_Manual_EN_v1.5.0.pdf)
[Sample_Dicer_Manual_RU_v1.5.0.pdf](https://github.com/user-attachments/files/29986773/Sample_Dicer_Manual_RU_v1.5.0.pdf)

## License

Sample Dicer is free for noncommercial use under the
[PolyForm Noncommercial License 1.0.0](LICENSE). Commercial use, commercial
development, and use in revenue-generating products or services require a
separate paid license. Contact Ilya Orange via
[Facebook](https://www.facebook.com/orange303/),
[X](https://x.com/ilyaorangee), or
[Telegram](https://t.me/orange303).

## Support

If you enjoy Sample Dicer and would like to support its development, you can
[make a donation via PayPal](https://www.paypal.com/paypalme/ilyaorange303).

BTC: 3PZSPAgXpLUtnH2LH9TmxjizVUETHPa9cW

ETH: 0x6144548f3f6071136fdf18134a99345cf12ae6b5

USDT ERC20: 0x0f49f2cddf673214646a3154f60aa0c63a414ad3
