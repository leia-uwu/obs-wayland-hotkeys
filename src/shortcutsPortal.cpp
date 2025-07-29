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

#include "shortcutsPortal.h"

#include <obs-frontend-api.h>
#include <obs-hotkey.h>

#include <QMessageBox>

#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
#include <private/qdesktopunixservices_p.h>
#else
#include <private/qgenericunixservices_p.h>
#endif
#include <private/qguiapplication_p.h>
#include <qpa/qplatformintegration.h>

static const QString freedesktopDest = QLatin1String("org.freedesktop.portal.Desktop");
static const QString freedesktopPath = QLatin1String("/org/freedesktop/portal/desktop");
static const QString globalShortcutsInterface = QLatin1String("org.freedesktop.portal.GlobalShortcuts");

void ShortcutsPortal::createSession()
{
    QDBusMessage createSessionCall = QDBusMessage::createMethodCall(
        freedesktopDest,
        freedesktopPath,
        globalShortcutsInterface,
        QLatin1String("CreateSession")
    );

    QList<QVariant> createSessionArgs;

    QMap<QString, QVariant> sessionOptions;
    sessionOptions.insert(QLatin1String("handle_token"), m_handleToken);
    sessionOptions.insert(QLatin1String("session_handle_token"), m_sessionHandleToken);
    createSessionArgs.append(sessionOptions);
    createSessionCall.setArguments(createSessionArgs);

    QDBusMessage call = QDBusConnection::sessionBus().call(createSessionCall);
    if (call.type() != QDBusMessage::ReplyMessage) {
        auto errMsg = QMessageBox(m_parentWindow);
        errMsg.critical(m_parentWindow, QLatin1String("Failed to create global shortcuts session"), call.errorMessage());
    }

    this->m_responseHandle = call.arguments().first().value<QDBusObjectPath>();

    qDBusRegisterMetaType<std::pair<QString, QVariantMap>>();
    qDBusRegisterMetaType<QList<QPair<QString, QVariantMap>>>();

    QDBusConnection::sessionBus().connect(
        freedesktopDest,
        m_responseHandle.path(),
        QLatin1String("org.freedesktop.portal.Request"),
        QLatin1String("Response"),
        this,
        SLOT(onCreateSessionResponse(uint, QVariantMap))
    );
}

int ShortcutsPortal::getVersion()
{
    QDBusMessage message = QDBusMessage::createMethodCall(
        freedesktopDest,
        freedesktopPath,
        QLatin1String("org.freedesktop.DBus.Properties"),
        QLatin1String("Get")
    );

    message.setArguments({globalShortcutsInterface, QLatin1String("version")});
    QDBusMessage reply = QDBusConnection::sessionBus().call(message);
    auto version = reply.arguments().first().value<QDBusVariant>().variant().toUInt();
    return version;
};

void ShortcutsPortal::createShortcut(const char* name, const char* description, obs_hotkey_id id)
{
    m_shortcuts[name] = {
        .name = name,
        .description = description,
        .id = id
    };
};

void ShortcutsPortal::createShortcuts()
{
    m_shortcuts.clear();

    obs_enum_hotkeys(
        [](void* data, obs_hotkey_id id, obs_hotkey_t* binding) {
            auto t = static_cast<ShortcutsPortal*>(data);

            auto name = obs_hotkey_get_name(binding);
            auto description = obs_hotkey_get_description(binding);

            t->createShortcut(name, description, id);

            return true;
        },
        this
    );
}

void ShortcutsPortal::onCreateSessionResponse(uint, const QVariantMap& results)
{
    if (results.contains(QLatin1String("session_handle"))) {
        QString sessionHandle = results[QLatin1String("session_handle")].toString();
        this->m_sessionObjPath = QDBusObjectPath(sessionHandle);
    };

    QDBusConnection::sessionBus().disconnect(
        freedesktopDest,
        m_responseHandle.path(),
        QLatin1String("org.freedesktop.portal.Request"),
        QLatin1String("Response"),
        this,
        SLOT(onCreateSessionResponse(uint, QVariantMap))
    );

    QDBusConnection::sessionBus().connect(
        freedesktopDest,
        freedesktopPath,
        globalShortcutsInterface,
        QLatin1String("Activated"),
        this,
        SLOT(onActivatedSignal(
            QDBusObjectPath, QString, qulonglong, QVariantMap
        ))
    );

    QDBusConnection::sessionBus().connect(
        freedesktopDest,
        freedesktopPath,
        globalShortcutsInterface,
        QLatin1String("Deactivated"),
        this,
        SLOT(onDeactivatedSignal(
            QDBusObjectPath, QString, qulonglong, QVariantMap
        ))
    );

    createShortcuts();
    bindShortcuts();
};

void ShortcutsPortal::onActivatedSignal(
    const QDBusObjectPath&,
    const QString& shortcutName,
    qulonglong,
    const QVariantMap&
)
{
    if (m_shortcuts.contains(shortcutName)) {
        obs_hotkey_trigger_routed_callback(m_shortcuts[shortcutName].id, true);
    }
};

void ShortcutsPortal::onDeactivatedSignal(
    const QDBusObjectPath&,
    const QString& shortcutName,
    qulonglong,
    const QVariantMap&
)
{
    if (m_shortcuts.contains(shortcutName)) {
        obs_hotkey_trigger_routed_callback(m_shortcuts[shortcutName].id, false);
    }
};

void ShortcutsPortal::bindShortcuts()
{
    QDBusMessage bindShortcuts = QDBusMessage::createMethodCall(
        freedesktopDest,
        freedesktopPath,
        globalShortcutsInterface,
        QLatin1String("BindShortcuts")
    );

    QList<QPair<QString, QVariantMap>> shortcuts;

    for (auto shortcut : m_shortcuts) {
        QPair<QString, QVariantMap> dbusShortcut;

        QVariantMap shortcutOptions;
        dbusShortcut.first = shortcut.name;
        shortcutOptions.insert(QLatin1String("description"), shortcut.description);
        dbusShortcut.second = shortcutOptions;

        shortcuts.append(dbusShortcut);
    }

    QMap<QString, QVariant> bindOptions;
    bindOptions.insert(QLatin1String("handle_token"), m_handleToken);

    QList<QVariant> shortcutArgs;
    shortcutArgs.append(m_sessionObjPath);
    shortcutArgs.append(QVariant::fromValue(shortcuts));

    shortcutArgs.append(getWindowId());
    shortcutArgs.append(bindOptions);
    bindShortcuts.setArguments(shortcutArgs);

    QDBusMessage msg = QDBusConnection::sessionBus().call(bindShortcuts);
    if (msg.type() != QDBusMessage::ReplyMessage) {
        auto errMsg = QMessageBox(m_parentWindow);
        errMsg.critical(m_parentWindow, QLatin1String("Failed to bind shortcuts"), msg.errorMessage());
    }
};

QString ShortcutsPortal::getWindowId()
{
    // copied from https://invent.kde.org/plasma/plasma-integration/-/blob/20581c0be9357afe052fda94c62c065d298455d9/qt6/src/platformtheme/kioopenwith.cpp#L60-71
    QString windowId;
    m_parentWindow->window()->winId(); // ensure we have a handle so we can export a window (without this windowHandle() may be null)
    auto services = QGuiApplicationPrivate::platformIntegration()->services();
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    if (auto unixServices = dynamic_cast<QDesktopUnixServices*>(services)) {
#else
    if (auto unixServices = dynamic_cast<QGenericUnixServices*>(services)) {
#endif
        windowId = unixServices->portalWindowIdentifier(m_parentWindow->window()->windowHandle());
    }

    return windowId;
}

void ShortcutsPortal::configureShortcuts()
{
    QDBusMessage bindShortcuts = QDBusMessage::createMethodCall(
        freedesktopDest,
        freedesktopPath,
        globalShortcutsInterface,
        QLatin1String("ConfigureShortcuts")
    );

    QMap<QString, QVariant> bindOptions;
    bindOptions.insert(QLatin1String("handle_token"), m_handleToken);

    QList<QVariant> shortcutArgs;
    shortcutArgs.append(m_sessionObjPath);

    shortcutArgs.append(getWindowId());
    shortcutArgs.append(bindOptions);
    bindShortcuts.setArguments(shortcutArgs);

    QDBusMessage msg = QDBusConnection::sessionBus().call(bindShortcuts);
    if (msg.type() != QDBusMessage::ReplyMessage) {
        auto errMsg = QMessageBox(m_parentWindow);
        errMsg.critical(m_parentWindow, QLatin1String("Failed to configure shortcuts"), msg.errorMessage());
    }
};

void ShortcutsPortal::shutdown()
{
    QDBusConnection::sessionBus().disconnect(
        freedesktopDest,
        freedesktopPath,
        globalShortcutsInterface,
        QLatin1String("Activated"),
        this,
        SLOT(onActivatedSignal(
            QDBusObjectPath, QString, qulonglong, QVariantMap
        ))
    );
}
