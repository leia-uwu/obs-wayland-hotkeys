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

#include <callback/signal.h>
#include <obs-frontend-api.h>
#include <obs-hotkey.h>
#include <obs-source.h>
#include <obs.h>

#include <QMessageBox>

#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
#include <private/qdesktopunixservices_p.h>
#else
#include <private/qgenericunixservices_p.h>
#endif
#include <private/qguiapplication_p.h>
#include <qpa/qplatformintegration.h>

using namespace Qt::Literals::StringLiterals;

static const QString FREEDESKTOP_DEST = u"org.freedesktop.portal.Desktop"_s;
static const QString FREEDESKTOP_PATH = u"/org/freedesktop/portal/desktop"_s;
static const QString GLOBAL_SHORTCUTS_INTERFACE = u"org.freedesktop.portal.GlobalShortcuts"_s;

ShortcutsPortal::ShortcutsPortal(QMainWindow* window)
    : m_parentWindow(window) { };

void ShortcutsPortal::createSession()
{
    QDBusMessage createSessionCall = QDBusMessage::createMethodCall(
        FREEDESKTOP_DEST,
        FREEDESKTOP_PATH,
        GLOBAL_SHORTCUTS_INTERFACE,
        u"CreateSession"_s
    );

    QList<QVariant> createSessionArgs;

    QMap<QString, QVariant> sessionOptions;
    sessionOptions.insert(u"handle_token"_s, m_handleToken);
    sessionOptions.insert(u"session_handle_token"_s, m_sessionHandleToken);
    createSessionArgs.append(sessionOptions);
    createSessionCall.setArguments(createSessionArgs);

    QDBusMessage call = QDBusConnection::sessionBus().call(createSessionCall);
    if (call.type() != QDBusMessage::ReplyMessage) {
        QMessageBox::critical(m_parentWindow, u"Failed to create global shortcuts session"_s, call.errorMessage());
    }

    this->m_responseHandle = call.arguments().first().value<QDBusObjectPath>();

    qDBusRegisterMetaType<std::pair<QString, QVariantMap>>();
    qDBusRegisterMetaType<QList<QPair<QString, QVariantMap>>>();

    QDBusConnection::sessionBus().connect(
        FREEDESKTOP_DEST,
        m_responseHandle.path(),
        u"org.freedesktop.portal.Request"_s,
        u"Response"_s,
        this,
        SLOT(onCreateSessionResponse(uint, QVariantMap))
    );
}

int ShortcutsPortal::getVersion()
{
    QDBusMessage message = QDBusMessage::createMethodCall(
        FREEDESKTOP_DEST,
        FREEDESKTOP_PATH,
        u"org.freedesktop.DBus.Properties"_s,
        u"Get"_s
    );

    message.setArguments({GLOBAL_SHORTCUTS_INTERFACE, u"version"_s});
    QDBusMessage reply = QDBusConnection::sessionBus().call(message);
    auto version = reply.arguments().first().value<QDBusVariant>().variant().toUInt();
    return version;
};

void ShortcutsPortal::createShortcut(
    const QString& name,
    const QString& description,
    const std::function<void(bool pressed)>& callback
)
{
    if (m_shortcuts.contains(name)) {
        qWarning() << "Shortcut" << name << "already registered!";
        return;
    }
    m_shortcuts[name] = {
        .name = name,
        .description = description,
        .callback = callback
    };
};

QPair<QString, QString> ShortcutsPortal::getHotkeyNameAndDesc(obs_hotkey_t* hotkey)
{
    QString name = obs_hotkey_get_name(hotkey);
    QString description = obs_hotkey_get_description(hotkey);

    void* registerer = obs_hotkey_get_registerer(hotkey);
    auto regType = obs_hotkey_get_registerer_type(hotkey);

    // for source hotkeys we add the source name to the name and description
    // example: "Switch to scene" for "Scene 2" is "OBSBasic.SelectScene.Scene_2"
    if (regType == OBS_HOTKEY_REGISTERER_SOURCE) {
        auto* weakSource = (obs_weak_source_t*)registerer;
        auto* source = obs_weak_source_get_source(weakSource);

        QString sourceName = obs_source_get_name(source);

        name += u".%1"_s.arg(sourceName.replace(" ", "_"));
        description += u" (%1)"_s.arg(obs_source_get_name(source));

        obs_source_release(source);
    }

    return {name, description};
};

void ShortcutsPortal::createOBSShortcut(obs_hotkey_id id, obs_hotkey_t* hotkey)
{
    auto name = getHotkeyNameAndDesc(hotkey);

    createShortcut(name.first, name.second, [id](bool pressed) {
        obs_hotkey_trigger_routed_callback(id, pressed);
    });
}

void ShortcutsPortal::createShortcuts()
{
    m_shortcuts.clear();

    obs_enum_hotkeys(
        [](void* data, obs_hotkey_id id, obs_hotkey_t* hotkey) {
            ((ShortcutsPortal*)data)->createOBSShortcut(id, hotkey);
            return true;
        },
        this
    );

    // KDE and Gnome don't allow binding multiple key combinations to the same action like obs does...
    // so add custom "toggle" shortcuts for actions that can be started / stopped

    createShortcut("_toggle_recording", "Toggle Recording", [](bool pressed) {
        // only want this to trigger when we press the bind, not when we release it
        if (!pressed)
            return;

        if (obs_frontend_recording_active()) {
            obs_frontend_recording_stop();
        } else {
            obs_frontend_recording_start();
        }
    });

    createShortcut("_toggle_streaming", "Toggle Streaming", [](bool pressed) {
        if (!pressed)
            return;

        if (obs_frontend_streaming_active()) {
            obs_frontend_streaming_stop();
        } else {
            obs_frontend_streaming_start();
        }
    });

    createShortcut("_toggle_replay_buffer", "Toggle Replay Buffer", [](bool pressed) {
        if (!pressed)
            return;

        if (obs_frontend_replay_buffer_active()) {
            obs_frontend_replay_buffer_stop();
        } else {
            obs_frontend_replay_buffer_start();
        }
    });

    createShortcut("_toggle_virtualcam", "Toggle Virtual Camera", [](bool pressed) {
        if (!pressed)
            return;

        if (obs_frontend_virtualcam_active()) {
            obs_frontend_stop_virtualcam();
        } else {
            obs_frontend_start_virtualcam();
        }
    });

    // https://github.com/obsproject/obs-studio/pull/12580
    /* Update release version number and uncomment when related request is merged.

    if (QVersionNumber::fromString(obs_get_version_string()) >= QVersionNumber(32, 1, 0))
        createShortcut("_toggle_preview", "Toggle Preview", [](bool pressed) {
            if (!pressed)
                return;

            if (obs_frontend_preview_enabled()) {
                obs_frontend_set_preview_enabled(false);
            } else {
                obs_frontend_set_preview_enabled(true);
            }
        });
    */

    createShortcut("_toggle_studio_mode", "Toggle Studio Mode", [](bool pressed) {
        if (!pressed)
            return;

        if (obs_frontend_preview_program_mode_active()) {
            obs_frontend_set_preview_program_mode(false);
        } else {
            obs_frontend_set_preview_program_mode(true);
        }
    });
}

static void reloadHotkeys(void* data, calldata_t* /* unused */)
{
    auto* portal = (ShortcutsPortal*)data;
    portal->createShortcuts();
    portal->bindShortcuts();
}

void ShortcutsPortal::onCreateSessionResponse(uint /*unused*/, const QVariantMap& results)
{
    if (results.contains(u"session_handle"_s)) {
        QString sessionHandle = results[u"session_handle"_s].toString();
        this->m_sessionObjPath = QDBusObjectPath(sessionHandle);
    };

    QDBusConnection::sessionBus().disconnect(
        FREEDESKTOP_DEST,
        m_responseHandle.path(),
        u"org.freedesktop.portal.Request"_s,
        u"Response"_s,
        this,
        SLOT(onCreateSessionResponse(uint, QVariantMap))
    );

    QDBusConnection::sessionBus().connect(
        FREEDESKTOP_DEST,
        FREEDESKTOP_PATH,
        GLOBAL_SHORTCUTS_INTERFACE,
        u"Activated"_s,
        this,
        SLOT(onActivatedSignal(
            QDBusObjectPath, QString, qulonglong, QVariantMap
        ))
    );

    QDBusConnection::sessionBus().connect(
        FREEDESKTOP_DEST,
        FREEDESKTOP_PATH,
        GLOBAL_SHORTCUTS_INTERFACE,
        u"Deactivated"_s,
        this,
        SLOT(onDeactivatedSignal(
            QDBusObjectPath, QString, qulonglong, QVariantMap
        ))
    );

    createShortcuts();
    bindShortcuts();

    // listen to hotkey registered so shortcuts added after initialization
    // (example: adding new sources / scenes, enabling replay buffer etc)
    // will not require restarting OBS
    //
    // NOTE: also listening to hotkey_unregister and removing the hotkeys from the portal is a bad idea..
    // it ends up unregistering a bunch of hotkeys on shutdown
    // and prompting the user to register them again on next startup...
    signal_handler_connect(obs_get_signal_handler(), "hotkey_register", reloadHotkeys, this);
};

void ShortcutsPortal::onActivatedSignal(
    const QDBusObjectPath& /*unused*/,
    const QString& shortcutName,
    qulonglong /*unused*/,
    const QVariantMap& /*unused*/
)
{
    if (m_shortcuts.contains(shortcutName)) {
        m_shortcuts[shortcutName].callback(true);
    }
};

void ShortcutsPortal::onDeactivatedSignal(
    const QDBusObjectPath& /*unused*/,
    const QString& shortcutName,
    qulonglong /*unused*/,
    const QVariantMap& /*unused*/
)
{
    if (m_shortcuts.contains(shortcutName)) {
        m_shortcuts[shortcutName].callback(false);
    }
};

void ShortcutsPortal::bindShortcuts()
{
    QDBusMessage bindShortcuts = QDBusMessage::createMethodCall(
        FREEDESKTOP_DEST,
        FREEDESKTOP_PATH,
        GLOBAL_SHORTCUTS_INTERFACE,
        u"BindShortcuts"_s
    );

    QList<QPair<QString, QVariantMap>> shortcuts;

    for (const auto& shortcut : m_shortcuts) {
        QPair<QString, QVariantMap> dbusShortcut;

        QVariantMap shortcutOptions;
        dbusShortcut.first = shortcut.name;
        shortcutOptions.insert(u"description"_s, shortcut.description);
        dbusShortcut.second = shortcutOptions;

        shortcuts.append(dbusShortcut);
    }

    QMap<QString, QVariant> bindOptions;
    bindOptions.insert(u"handle_token"_s, m_handleToken);

    QList<QVariant> shortcutArgs;
    shortcutArgs.append(m_sessionObjPath);
    shortcutArgs.append(QVariant::fromValue(shortcuts));

    shortcutArgs.append(getWindowId());
    shortcutArgs.append(bindOptions);
    bindShortcuts.setArguments(shortcutArgs);

    QDBusMessage msg = QDBusConnection::sessionBus().call(bindShortcuts);
    if (msg.type() != QDBusMessage::ReplyMessage) {
        QMessageBox::critical(m_parentWindow, u"Failed to bind shortcuts"_s, msg.errorMessage());
    }
};

QString ShortcutsPortal::getWindowId()
{
    // copied from https://invent.kde.org/plasma/plasma-integration/-/blob/20581c0be9357afe052fda94c62c065d298455d9/qt6/src/platformtheme/kioopenwith.cpp#L60-71
    QString windowId;
    m_parentWindow->window()->winId(); // ensure we have a handle so we can export a window (without this windowHandle() may be null)
    auto* services = QGuiApplicationPrivate::platformIntegration()->services();
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    if (auto* unixServices = dynamic_cast<QDesktopUnixServices*>(services)) {
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
        FREEDESKTOP_DEST,
        FREEDESKTOP_PATH,
        GLOBAL_SHORTCUTS_INTERFACE,
        u"ConfigureShortcuts"_s
    );

    QMap<QString, QVariant> bindOptions;
    bindOptions.insert(u"handle_token"_s, m_handleToken);

    QList<QVariant> shortcutArgs;
    shortcutArgs.append(m_sessionObjPath);

    shortcutArgs.append(getWindowId());
    shortcutArgs.append(bindOptions);
    bindShortcuts.setArguments(shortcutArgs);

    QDBusMessage msg = QDBusConnection::sessionBus().call(bindShortcuts);
    if (msg.type() != QDBusMessage::ReplyMessage) {
        QMessageBox::critical(m_parentWindow, u"Failed to configure shortcuts"_s, msg.errorMessage());
    }
};

ShortcutsPortal::~ShortcutsPortal()
{
    signal_handler_disconnect(obs_get_signal_handler(), "hotkey_register", reloadHotkeys, this);

    QDBusConnection::sessionBus().disconnect(
        FREEDESKTOP_DEST,
        FREEDESKTOP_PATH,
        GLOBAL_SHORTCUTS_INTERFACE,
        u"Activated"_s,
        this,
        SLOT(onActivatedSignal(
            QDBusObjectPath, QString, qulonglong, QVariantMap
        ))
    );
    QDBusConnection::sessionBus().disconnect(
        FREEDESKTOP_DEST,
        FREEDESKTOP_PATH,
        GLOBAL_SHORTCUTS_INTERFACE,
        u"Deactivated"_s,
        this,
        SLOT(onActivatedSignal(
            QDBusObjectPath, QString, qulonglong, QVariantMap
        ))
    );
}

// NOLINTNEXTLINE: bugprone-suspicious-include
#include "moc_shortcutsPortal.cpp"
