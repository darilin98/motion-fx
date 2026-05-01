# VisiMusic

Vision-based effect modulation VST plugin 

This project was created as part of a **Bachelor's thesis at Charles University**, titled: *"A VST plugin for video-controlled audio manipulation"*.

<p align="center" width="100%">
    <img src="docs/resources/preview_bird.gif" alt="preview-bird" width="600"/>
</p>

## Concept & Functionality

VisiMusic is an effect plugin that applies audio modulation, configured based on a live video input.
This creates unique soundscapes connected to visual content.

- Compatible with a broad scale of DAWs
- Creative control over what aspects of the media file get detected
- Works on all common image and video file types.
- Contains multiple signal processing effects, each mapped to a different visual feature
- Open source, with easy extendability & documentation on how to detect more features or add more effects

<p align="center" width="100%">
    <img src="docs/resources/preview_sunset.gif" alt="preview-sunset" width="600"/>
</p>

---

## System requirements

- **Windows**: 64-bit Windows 10 or later
- **macOS**: Apple Silicon (ARM64) macOS 11 or later
- 64-bit DAW with VST3 support

## Installing the plugin

Binaries for both Windows and macOS are available for download in the *Releases* section.

For a description of the installation process and troubleshooting help visit [this page.](docs/build_guide.md)

## License

This project is licensed under the GNU GPL v3.
See the [LICENSE](LICENSE.txt) file for full details.

## Third-party dependencies

The project uses the following libraries.

- [VST3SDK](https://github.com/steinbergmedia/vst3sdk)
- [FFMPEG](https://ffmpeg.org/)
- [STB](https://github.com/nothings/stb)
- [DaisySP](https://github.com/electro-smith/DaisySP)
- [Catch2](https://github.com/catchorg/Catch2)

See the [THIRD_PARTY_NOTICES](THIRD_PARTY_NOTICES.txt) file for details and licensing information.

## Input videos starter pack

To help you get started with using Visimusic, you can visit [this page](docs/example_input_videos.md) for a list of videos that have been used during testing and react with the plugin in interesting ways.

The videos are provided as download links to comply with the license of the provider - [PixaBay](https://pixabay.com/).

## Help & Documentation

- [Detailed user guide](docs/user_guide.md)
- [Installation guide](docs/build_guide.md)
- [Full project documentation](https://darilin98.github.io/visimusic/index.html)
- Thesis full text pdf
