# Installation

The plugin is distributed through pre-compiled binaries.

The binaries can be found in the *"Releases"* section of the project's GitHub page. From them you can simply download the file matching your platform.

### macOS

The macOS version of the plugin is packaged as a file with the `.dmg` suffix. 

Once you open it, you will be prompted with a Finder window containing an installation script (with the suffix `.app`) and the plugin bundle (with the suffix `.vst3`)

To continue, double-click the installation script. macOS might flag it as foreign software and show a warning.
To remove this, go to the Privacy & Security section in the settings, scroll down and explicitly allow `Install.app` to run.

Running the installation script should prompt you with a pop-up window, accept it and the script should both copy the plugin to the destination and white-list its files.

In the event that the installation script fails, you can execute its steps manually by first copying the `visimusic.vst3` folder (might be showing as just `visimusic` in some Finder configurations) to the VST plugin destination: `~/Library/Audio/Plug-Ins/VST3/`

If you manually copied the plugin files to the destination folder, run the following command in the terminal, it ensures that all of the files contained in the plugin have been white-listed:

```
xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/visimusic.vst3
```

Finally, restart your DAW, and it should either automatically detect and add the plugin or prompt you to include it during start up.

### Windows

To install the plugin on the Windows platform, first download the file prefixed with `win_` and ending with `.zip`. Unzip that file and inside you will see a `.vst3` folder.

Copy that folder in its entirety (it is important that you copy the full folder with all of its sub-folders) to the shared plugin location, on most Windows machines it is `C:\Program Files (x86)\Common Files\VST3`.

Finally, restart your DAW, and it should either automatically detect and add the plugin or prompt you to include it during start up.

### Compiling the plugin

It is possible to compile the plugin yourself, but the process is not so straightforward.

For an extensive guide please refer to the build section in the thesis.