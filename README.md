# OBS Wayland Hotkeys

An OBS Studio plugin to integrate OBS hotkeys with the [wayland global shortcuts portal](https://flatpak.github.io/xdg-desktop-portal/docs/doc-org.freedesktop.portal.GlobalShortcuts.html).

<p float="left" align="center">
    <img src="screenshots/1.png" width="400" alt="OBS Open with the KDE Plasma shortcuts portal dialog asking to register the shortcuts">
    <img src="screenshots/2.png" width="400" alt="OBS Shortcuts on KDE Plasma system settings Shortcuts page">
</p>

Requires a distribution with somewhat up to date Qt (e.g Ubuntu 24.04 LTS with Qt 6.4 will **not** work)

Thanks to [qt_wayland_globalshortcut_via_portal](https://github.com/slbtty/qt_wayland_globalshortcut_via_portal/) which I used as a reference to implement the portal.

## Installation

### Flatpak:
Use the official Flathub package: https://flathub.org/en/apps/com.obsproject.Studio.Plugin.WaylandHotkeys

### Arch Linux:
There's an unofficial AUR package: https://aur.archlinux.org/cgit/aur.git/tree/PKGBUILD?h=obs-wayland-hotkeys-git

### Fedora:
There's an unofficial COPR package made by [@mihawk90](https://github.com/mihawk90): https://copr.fedorainfracloud.org/coprs/tarulia/obs-studio-plugins/package/obs-studio-plugin-wayland-hotkeys/

### Compiling it yourself:

Requires the `Qt6WaylandClient` and `Qt6GuiPrivate` header files, which should also pull in `Qt6Base`, and also OBS Studio installed so it can pull the OBS API headers.

See https://github.com/obsproject/obs-plugintemplate/wiki/Build-Dependencies for more information.

Building:
```bash
git clone https://github.com/leia-uwu/obs-wayland-hotkeys
cd obs-wayland-hotkeys
cmake --preset=ubuntu-x86_64
cmake --build build_x86_64
```

Installing:
```bash
mkdir -p ~/.config/obs-studio/plugins/obs-wayland-hotkeys/bin/64bit/
cp build_x86_64/obs-wayland-hotkeys.so ~/.config/obs-studio/plugins/obs-wayland-hotkeys/bin/64bit/obs-wayland-hotkeys.so
```

## Hotkeys

All the hotkeys provided by [obs_enum_hotkeys](https://github.com/obsproject/obs-studio/blob/d84ff0eb6fa7b32ba9629d3d27c92b391b0310cc/libobs/obs-hotkey.c#L970) are registered.

Hotkeys that have a source or scene attached to them will have an ID with a period + the source name with spaces replaced with underscores

Example: `Switch to scene` for an scene named `Scene 2` is `OBSBasic.SelectScene.Scene_2`

Additionally, since KDE Plasma and GNOME do not allow binding multiple keybindings to the same action, the following extra hotkeys are added:

```
:_toggle_recording -> Toggle Recording
:_toggle_replay_buffer -> Toggle Replay Buffer
:_toggle_streaming -> Toggle Streaming
:_toggle_studio_mode -> Toggle Studio Mode
:_toggle_virtualcam -> Toggle Virtual Camera
```

## Configuring

Usually desktop environments will show a dialog asking to register the shortcuts after the first time OBS was opened with the plugin.

Rejecting that dialog will make it show again after restarting OBS.

To configure shortcuts after the initial setup screen you have to go to your desktop environment settings page for them:

### KDE Plasma
`System Settings -> Keyboard -> Shortcuts -> OBS Studio`

### GNOME
`Settings -> Apps -> OBS Studio -> Global Shortcuts`

### Hyprland

Follow the instructions from the [Hyprland wiki](https://wiki.hypr.land/Configuring/Binds/#dbus-global-shortcuts).

