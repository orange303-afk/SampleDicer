# Sample Dicer

Four-layer one-shot sampler for AU, VST3 and standalone use. One MIDI note triggers all four layers. **DICE** selects samples from their source folders and randomises volume, pitch, start point and delay.

<img width="816" height="929" alt="image" src="https://github.com/user-attachments/assets/902e6251-1160-4eb2-82e8-952a89a0af9d" />


## Features

- Four drag-and-drop sample slots with waveform display and previous/next folder navigation
- Refined arc-style controls with parameter units
- Per-slot volume, pitch, draggable start point, positive shift (0–250 ms), and fade-end controls
- Smooth real-time playheads and visual start, shift, and fade editing on every waveform
- MIDI note-on triggering
- Selectable 1–16 voice polyphony
- Hold-to-repeat Burst mode with adjustable 1–30 Hz rate
- Per-trigger parameter evolution (PTE) with a real-time-safe audio path
- Per-slot LOCK controls to exclude layers from DICE and PTE
- Separate DICE, SAMPLES and PARAMS actions
- BACK recalls the previous generated combination
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

## Downloads

Ready-to-use macOS VST3, Audio Unit, standalone, and Windows x64 VST3 builds are available on the [Releases](https://github.com/orange303-afk/SampleDicer/releases) page.

## License

Sample Dicer is free for noncommercial use under the
[PolyForm Noncommercial License 1.0.0](LICENSE). Commercial use, commercial
development, and use in revenue-generating products or services require a
separate paid license. Contact Ilya Orange through
[Facebook](https://www.facebook.com/orange303/),
[X](https://x.com/ilyaorangee), or
[Telegram](https://t.me/orange303).

## Support

If you enjoy Sample Dicer and would like to support its development, you can
[make a donation via PayPal](https://www.paypal.com/paypalme/ilyaorange303).
BTC: 3PZSPAgXpLUtnH2LH9TmxjizVUETHPa9cW
ETH: 0x6144548f3f6071136fdf18134a99345cf12ae6b5
USDT ERC20: 0x0f49f2cddf673214646a3154f60aa0c63a414ad3
