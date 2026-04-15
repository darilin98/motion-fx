set pluginName to "visimusic.vst3"
set installDir to POSIX path of (path to home folder) & "Library/Audio/Plug-Ins/VST3/"
set appDir to do shell script "dirname " & quoted form of POSIX path of (path to me)
set pluginSource to appDir & "/" & pluginName

-- Prompt dialog window
display dialog "This will install Visimusic to your VST3 folder." & return & return & installDir buttons {"Cancel", "Install"} default button "Install" with icon note

-- Install
try
    do shell script "mkdir -p " & quoted form of installDir & " && cp -R " & quoted form of pluginSource & " " & quoted form of installDir & " && xattr -dr com.apple.quarantine " & quoted form of (installDir & pluginName) with administrator privileges
    display dialog "Visimusic installed successfully!" & return & return & "Please restart your DAW." buttons {"OK"} default button "OK" with icon note
on error errMsg
    display dialog "Installation failed:" & return & return & errMsg buttons {"OK"} default button "OK" with icon stop
end try
