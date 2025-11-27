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

#pragma once

#include <QMainWindow>
#include <QtDBus/QtDBus>

#include <functional>
#include <obs-frontend-api.h>
#include <obs-hotkey.h>

struct Shortcut
{
    QString name;
    QString description;

    std::function<void(bool pressed)> callback;
};

class ShortcutsPortal : public QObject
{
    Q_OBJECT
public:
    ShortcutsPortal(QMainWindow* window);
    ~ShortcutsPortal() override;

    void createSession();

    static int getVersion();

    void bindShortcuts();
    void configureShortcuts();

    void createShortcut(
        const char* name,
        const char* description,
        const std::function<void(bool pressed)>& callback
    );

    void createShortcuts();

private:
    Q_SLOT void onCreateSessionResponse(uint, const QVariantMap& results);

    Q_SLOT void onActivatedSignal(
        const QDBusObjectPath& sessionHandle,
        const QString& shortcutName,
        qulonglong timestamp,
        const QVariantMap& options
    );

    Q_SLOT void onDeactivatedSignal(
        const QDBusObjectPath& sessionHandle,
        const QString& shortcutName,
        qulonglong timestamp,
        const QVariantMap& options
    );

    QString getWindowId();

    QMap<QString, Shortcut> m_shortcuts;

    const QString m_handleToken = "obs_portal_shortcuts";
    const QString m_sessionHandleToken = "obs_portal_shortcuts";

    QMainWindow* m_parentWindow = nullptr;

    QDBusObjectPath m_responseHandle;
    QDBusObjectPath m_sessionObjPath;
};
