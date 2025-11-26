/*
    OBS Wayland Hotkeys
    Copyright (C) 2025 Leia <leia@tutamail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include "src/shortcutsPortal.h"

#include <obs-frontend-api.h>
#include <obs-module.h>

OBS_DECLARE_MODULE();

MODULE_EXPORT const char *obs_module_text(void)
{
    return "OBS Wayland Hotkeys";
}

MODULE_EXPORT const char *obs_module_description(void)
{
    return "Provides global hotkey support for OBS Studio on Wayland using the Freedesktop GlobalShortcuts portal.";
}

#include <QAction>
#include <QGuiApplication>
#include <QMainWindow>

using namespace Qt::Literals::StringLiterals;

ShortcutsPortal* portal = nullptr;

bool obs_module_load(void)
{
    bool isWayland = QGuiApplication::platformName().contains(u"wayland"_s);
    return isWayland;
}

void on_frontend_event(enum obs_frontend_event event, void *)
{
    if (event == OBS_FRONTEND_EVENT_FINISHED_LOADING) {
        if (portal) {
            portal->createSession();
        }
    }
}

void obs_module_post_load()
{
    portal = new ShortcutsPortal();
    QMainWindow* mainWindow = static_cast<QMainWindow*>(obs_frontend_get_main_window());
    portal->setWindow(mainWindow);
    
    obs_frontend_add_event_callback(on_frontend_event, nullptr);

    if (true) {
        QAction* action = (QAction*)obs_frontend_add_tools_menu_qaction("Configure Wayland Hotkeys");

        QObject::connect(action, &QAction::triggered, []() {
            portal->configureShortcuts();
        });
    }
}

void obs_module_unload(void)
{
    if (portal) {
        delete portal;
    }
}
