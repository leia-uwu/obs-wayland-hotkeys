# OBS Wayland Hotkeys

An OBS Studio plugin to integrate OBS hotkeys with the [wayland global shortcuts portal](https://flatpak.github.io/xdg-desktop-portal/docs/doc-org.freedesktop.portal.GlobalShortcuts.html)

<img src="screenshots/1.png" alt="OBS Open with the KDE Plasma shortcuts portal dialog asking to register the shortcuts">
<img src="screenshots/2.png" alt="OBS Shortcuts on KDE Plasma system settings Shortcuts page">

Only tested on Arch Linux with KDE Plasma 6.5 dev branch and Qt 6.9

Requires somewhat modern Qt, but not like distros with old Qt (like ubuntu 24.04, where it doesn't even build) have desktops that support the  shortcuts portal anyway lol

Thanks to https://github.com/slbtty/qt_wayland_globalshortcut_via_portal/ which i used as a reference to implement the portal

## Configuring after initial portal dialog

To configure shortcuts after the initial setup screen you have to go to your desktop environment settings page for them.

On KDE Plasma its on System Settings -> Keyboard -> Shortcuts -> OBS Studio

On Gnome its on Settings -> Apps -> OBS Studio -> Global Shortcuts


## Build instructions

Uh go look at the OBS plugin template docs:
https://github.com/obsproject/obs-plugintemplate/wiki/

Requires the `Qt6WaylandClient` and `Qt6GuiPrivate` development files, which should also pull in `Qt6Base`.

