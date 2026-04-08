# The documentation of VisiMusic

Welcome to the full documentation of VisiMusic

VisiMusic is a vision-based VST3 plugin, capable of audio modulation based on visual features. 

---

## List of Features

- A custom effect chain containing a wide array of effects
- Real-time functionality with offline export for sample-accurate results
- Sleek, modern, intuitive GUI
- Processing of many common visual elements of video
- Easily extendable design for both effects and visual feature extractors
- Smooth integration into music production workflow thanks to the VST standard

## The conceptual model

VisiMusic contains an effect chain that gets applied to the incoming samples.

These effects all expose various parameters that change their signal processing, e.g., a filter has a parameter to gradually fade from low pass to high pass

All effects are connected to some characteristic of the input video, e.g. a filter is mapped to brightness.

These visual features are extracted in real-time and constantly update the configuration of the effects.

Visit [this page](architecture.md) for more details on the inner-workings of the plugin.

## External dependencies

- Core plugin dev kit: [VST3SDK](https://github.com/steinbergmedia/vst3sdk)
- Video decoding: [FFMPEG](https://ffmpeg.org/)
- Image decoding: [STB](https://github.com/nothings/stb)
- Signal processing: [DaisySP](https://github.com/electro-smith/DaisySP)

## System requirements

- **Windows**: 64-bit Windows 10 or later
- **macOS**: Apple Silicon (ARM64) macOS 11 or later
- 64-bit DAW with VST3 support

## DAW compatibility table

The following table lists all DAWs on which the plugin has been tested on.

|     **DAW**      | **Operating system** |  **Compatibility**   |
|:----------------:|----------------------|:--------------------:|
| Waveform 13 Free | macOS, Windows       |         Full         |
| Bitwig Studio 6  | macOS, Windows       |         Full         |
|     FLStudio     | macOS, Windows       | Scaling issue on mac |
|      Reaper      | macOS                |         Full         |
|     ProTools     | macOS                |  Plugin undetected   |

## Installation process

For a detailed installation guide complete with common pitfalls, visit [this page.](build_guide.md)