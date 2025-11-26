# OBS Wayland Hotkeys

An OBS Studio plugin to integrate OBS hotkeys with the [Wayland Global Shortcuts Portal](https://flatpak.github.io/xdg-desktop-portal/docs/doc-org.freedesktop.portal.GlobalShortcuts.html).

This allows you to use push-to-talk, scene switching, and other hotkeys while OBS is in the background on Wayland sessions (where traditional hotkeys often fail).

<img src="screenshots/1.png" alt="OBS Open with the KDE Plasma shortcuts portal dialog asking to register the shortcuts" width="400">
<img src="screenshots/2.png" alt="OBS Shortcuts on KDE Plasma system settings Shortcuts page" width="400">

## Table of Contents
1. [Installation](#installation)
2. [First Time Setup](#first-time-setup)
3. [Configuring Shortcuts](#configuring-shortcuts)
4. [System Settings Locations](#system-settings-locations)
    - [Ubuntu / GNOME](#ubuntu--gnome)
    - [KDE Plasma](#kde-plasma)
5. [Updating or Adding New Shortcuts](#updating-or-adding-new-shortcuts-important)
6. [Build Instructions](#build-instructions)

---

## Installation

### Flatpak (Recommended)
This plugin is available in the official OBS Studio Flatpak Add-ons list.
1. Open your software manager or terminal.
2. Install the **OBS Studio** Flatpak.
3. Look for **OBS Wayland Hotkeys** in the add-ons/extensions list and install it.

### Manual Build
See [Build Instructions](#build-instructions) at the bottom if you are compiling from source.

---

## First Time Setup

When you launch OBS Studio for the first time after installing this plugin:

1. A system window titled **Add Keyboard Shortcuts** will appear.
2. This window lists all available OBS actions (like "Toggle Recording" or "Switch to Scene").
3. You can click on actions here to assign your keys immediately.
4. Click **Add** to finish setup.

**Important:**
- This system **replaces** the standard OBS Hotkeys menu for these actions.
- The keys you set here are managed by your Operating System, not OBS.
- It does not import your existing hotkeys from OBS; you must set them up in this new dialog.

*Note: If you do not see this dialog, ensure you are running a Wayland session and have a compatible desktop environment (GNOME 49+, KDE Plasma 6+, etc).*

---

## Configuring Shortcuts

If you need to change your keys later, you manage them in your **System Settings**, not inside OBS.

### The "Tools" Menu
Inside OBS, you can try to trigger the configuration dialog manually:
1. Go to the **Tools** menu at the top of OBS.
2. Select **Configure Wayland Hotkeys**.

*Note: This button attempts to open the system's shortcut configuration window. On some desktop environments, this button may do nothing. In that case, use the System Settings locations below.*

---

## System Settings Locations

To change which keys trigger your actions manually:

### Ubuntu / GNOME
*Tested on Ubuntu 25.10 (GNOME 49)*

1. Open system **Settings**.
2. Navigate to **Apps** -> **OBS Studio**.
3. Click on **Global Shortcuts**.
4. You will see the list of actions. Click them to assign keys.
5. To reset (see below), scroll to the very bottom to find **Remove All Shortcuts**.

### KDE Plasma
*Tested on Arch Linux with Plasma 6.5*

1. Open **System Settings**.
2. Navigate to **Keyboard** -> **Shortcuts**.
3. Select **OBS Studio** from the list.

---

## Updating or Adding New Shortcuts (Important)

The Wayland Portal handles shortcuts in "Sessions". How updates are handled depends on the version of the portal your system uses.

### Version 1 vs Version 2 Behavior

**The Issue:**
If you add a new Scene in OBS, or if a new feature is added to this plugin, the new shortcut might not appear in your System Settings immediately because the Operating System "remembers" the old list of shortcuts from the previous session.

**How to Fix (Resetting the Session):**
If you are missing shortcuts (like a newly created Scene) in your system settings:

1. **Close OBS.**
2. Go to your System Settings location (as described above).
3. **Remove the shortcuts.**
   - **Ubuntu/GNOME:** Scroll to the very bottom of the Global Shortcuts list and click the **Remove All Shortcuts** button.
   - **KDE:** Uncheck or remove the entry for OBS.
4. **Open OBS.**
5. The **Add Keyboard Shortcuts** dialog will appear again.
6. Click **Add**. The list in your System Settings will now be updated with all current scenes and actions.

---

## Build Instructions

*For developers or users on distributions without Flatpak support.*

Requirements:
- Qt 6 (Modern version required, e.g., Qt 6.9)
- CMake
- OBS Studio development headers

Refer to the [OBS Plugin Template Documentation](https://github.com/obsproject/obs-plugintemplate/wiki/) for standard build steps.

