set pluginName to "visimusic.vst3"
set installDir to POSIX path of (path to home folder) & "Library/Audio/Plug-Ins/VST3/"
set pluginDest to installDir & pluginName

set pluginSource to do shell script "find /Volumes -maxdepth 2 -name " & quoted form of pluginName & " 2>/dev/null | head -1"

if pluginSource is "" then
    display dialog "Could not find " & pluginName & " on any mounted volume." buttons {"OK"} default button "OK" with icon stop
    return
end if

display dialog "This will install Visimusic to your VST3 folder." & return & return & installDir buttons {"Cancel", "Install"} default button "Install" with icon note

try
    do shell script "mkdir -p " & quoted form of installDir & ¬
        " && rm -rf " & quoted form of pluginDest & ¬
        " && ditto " & quoted form of pluginSource & " " & quoted form of pluginDest

    do shell script "xattr -dr com.apple.quarantine " & quoted form of pluginDest

    display dialog "Visimusic installed successfully!" & return & return & "Please restart your DAW." buttons {"OK"} default button "OK" with icon note
on error errMsg
    display dialog "Installation failed:" & return & return & errMsg buttons {"OK"} default button "OK" with icon stop
end try