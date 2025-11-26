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
#include <QSet>
#include <QtDBus/QtDBus>
#include <functional>
#include <obs-frontend-api.h>

struct PortalShortcut
{
    QString name;
    QString description;

    std::function<void(bool pressed)> callbackFunc;
};

class ShortcutsPortal : public QObject
{
    Q_OBJECT
public:
    explicit ShortcutsPortal(QObject* parent = nullptr);
    ~ShortcutsPortal();

    void createSession();

    int getVersion();

    void bindShortcuts();
    void configureShortcuts();

    void createShortcut(
        const QString& name,
        const QString& description,
        const std::function<void(bool pressed)>& callbackFunc
    );

    void createShortcuts();

    void setWindow(QMainWindow* window)
    {
        m_parentWindow = window;
    }

    static void obsFrontendEvent(enum obs_frontend_event event, void* private_data);

public Q_SLOTS:
    void onCreateSessionResponse(uint response, const QVariantMap& results);

    void onActivatedSignal(
        const QDBusObjectPath& sessionHandle,
        const QString& shortcutName,
        qulonglong timestamp,
        const QVariantMap& options
    );

    void onDeactivatedSignal(
        const QDBusObjectPath& sessionHandle,
        const QString& shortcutName,
        qulonglong timestamp,
        const QVariantMap& options
    );

private:
    QString getWindowId();

    QMap<QString, PortalShortcut> m_shortcuts;

    const QString m_handleToken = "obs_portal_shortcuts";
    const QString m_sessionHandleToken = "obs_portal_shortcuts_session";

    QMainWindow* m_parentWindow = nullptr;

    QDBusObjectPath m_responseHandle;
    QDBusObjectPath m_sessionObjPath;

    bool m_isLoaded = false;
    void* m_currentValidSources = nullptr;
};
