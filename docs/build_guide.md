# Installation

The plugin is distributed through pre-compiled binaries.

The binaries can be found in the *"Releases"* section of the project's GitHub page. From them you can simply download the file matching your platform.

### macOS

The macOS version of the plugin is packaged as a file with the `.dmg` suffix. 

Once you open it, you will be prompted with a Finder window containing an installation script (with the suffix `.app`) and the plugin bundle (with the suffix `.vst3`)

To continue, simply double-click the installation script. It should prompt you with a pop-up window, accept it and the script will ask you for your system password.
Entering your password is necessary in order to give the script admin privileges for white-listing the plugin files.

After successfully running the script, the plugin should already be copied to `~/Library/Audio/Plug-Ins/VST3`. 
If not, manually copy the entire `.vst3` file from the installer to the plugin location.

Finally, restart your DAW, and it should either automatically detect and add the plugin or prompt you to include it during start up.

### Windows

Installing the plugin on Windows is very simple. 

Simply download the windows-specific file ending with `.zip`. Unzip that file and inside you will see a `.vst3` folder.

Copy that folder in its entirety (it is important that you copy the full folder with all of its sub-folders) to the shared plugin location, on most Windows machines it is `C:\Program Files (x86)\Common Files\VST3`.

Finally, restart your DAW, and it should either automatically detect and add the plugin or prompt you to include it during start up.

### Compiling the plugin

It is possible to compile the plugin yourself, but the process is not so straightforward.

For an extensive guide please refer to the build section in the thesis.