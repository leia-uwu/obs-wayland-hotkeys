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

OBS_DECLARE_MODULE()

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

void obs_module_post_load()
{
    portal = new ShortcutsPortal();
    QMainWindow* mainWindow = static_cast<QMainWindow*>(obs_frontend_get_main_window());
    portal->setWindow(mainWindow);
    portal->createSession();

    if (portal->getVersion() >= 2) {
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
