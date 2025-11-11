# OBS Wayland Hotkeys

An OBS Studio plugin to integrate OBS hotkeys with the [wayland global shortcuts portal](https://flatpak.github.io/xdg-desktop-portal/docs/doc-org.freedesktop.portal.GlobalShortcuts.html).

<img src="screenshots/1.png" alt="OBS Open with the KDE Plasma shortcuts portal dialog asking to register the shortcuts">
<img src="screenshots/2.png" alt="OBS Shortcuts on KDE Plasma system settings Shortcuts page">

Only tested on Arch Linux with KDE Plasma 6.5 dev branch and Qt 6.9.

Requires somewhat modern Qt, but not like distros with old Qt (like Ubuntu 24.04, where it doesn't even build) have desktops that support the shortcuts portal anyway lol.

Thanks to [qt_wayland_globalshortcut_via_portal](https://github.com/slbtty/qt_wayland_globalshortcut_via_portal/) which I used as a reference to implement the portal.

## Hotkeys

All the hotkeys provided by [obs_enum_hotkeys](https://github.com/obsproject/obs-studio/blob/d84ff0eb6fa7b32ba9629d3d27c92b391b0310cc/libobs/obs-hotkey.c#L970) are registered.

Additionally, since KDE and Gnome do not allow binding multiple keybindings to the same action, the following extra hotkeys are added:

```
:_toggle_recording -> Toggle Recording
:_toggle_replay_buffer -> Toggle Replay Buffer
:_toggle_streaming -> Toggle Streaming
:_toggle_studio_mode -> Toggle Studio Mode
:_toggle_virtualcam -> Toggle Virtual Camera
```

## Configuring after initial portal dialog

To configure shortcuts after the initial setup screen you have to go to your desktop environment settings page for them.

### KDE Plasma
`System Settings -> Keyboard -> Shortcuts -> OBS Studio`

### Gnome
`Settings -> Apps -> OBS Studio -> Global Shortcuts`

### Hyprland

Follow the instructions from the [Hyprland wiki](https://wiki.hypr.land/Configuring/Binds/#dbus-global-shortcuts).


## Build instructions

Uh go look at the OBS plugin template docs:
https://github.com/obsproject/obs-plugintemplate/wiki/


