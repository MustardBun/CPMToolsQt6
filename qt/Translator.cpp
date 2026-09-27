//---------------------------------------------------------------------------
#include "Translator.h"

#include <QCoreApplication>
#include <QLocale>
#include <QSettings>
#include <QTranslator>

// Copyright (C) 2026 Joseph Kwok (@MustardBun)
// SPDX-License-Identifier: GPL-3.0-or-later

namespace i18n {

const char *const kSystemCode = "system";

namespace {

//! Resource directory holding the embedded catalogues.
const char *const kResourceDir = ":/i18n";

//! QSettings key holding the chosen language.
const char *const kLanguageKey = "ui/language";

//! The single installed catalogue. Kept alive for the process lifetime.
QTranslator *g_translator = nullptr;

//! Catalogue base name, matching the TS_FILES in CMakeLists.txt.
const char *const kBaseName = "cpmtoolsgui";

//! Build the resource path for a language code.
QString resourcePath(const QString &code)
{
    return QStringLiteral("%1/%2_%3.qm")
        .arg(QLatin1String(kResourceDir),
             QLatin1String(kBaseName),
             code);
}

//! Install a catalogue from the embedded resources. Replaces any previous one.
bool install(const QString &code)
{
    if (g_translator != nullptr) {
        QCoreApplication::removeTranslator(g_translator);
        delete g_translator;
        g_translator = nullptr;
    }

    auto *translator = new QTranslator(qApp);
    if (!translator->load(resourcePath(code))) {
        qWarning("i18n: could not load catalogue %s",
                 qPrintable(resourcePath(code)));
        delete translator;
        return false;
    }

    QCoreApplication::installTranslator(translator);
    g_translator = translator;
    return true;
}

//! Remove any installed catalogue, leaving the source strings in place.
void uninstall()
{
    if (g_translator != nullptr) {
        QCoreApplication::removeTranslator(g_translator);
        delete g_translator;
        g_translator = nullptr;
    }
}

//! True when \a code names a language we ship.
bool isKnown(const QString &code)
{
    for (const Language &language : languages()) {
        if (language.code == code)
            return true;
    }
    return false;
}

} // namespace

//---------------------------------------------------------------------------
QVector<Language> languages()
{
    // Order here is the order shown in the Settings > Language menu.
    return {
        { QStringLiteral("en"),    QStringLiteral("English")    },
        { QStringLiteral("ja"),    QStringLiteral("日本語")      },
        { QStringLiteral("zh_CN"), QStringLiteral("简体中文")    },
        { QStringLiteral("zh_TW"), QStringLiteral("繁體中文")    },
        { QStringLiteral("it"),    QStringLiteral("Italiano")   },
        { QStringLiteral("fr"),    QStringLiteral("Français")   },
        { QStringLiteral("de"),    QStringLiteral("Deutsch")    },
        { QStringLiteral("es"),    QStringLiteral("Español")    },
    };
}

//---------------------------------------------------------------------------
QString nativeName(const QString &code)
{
    for (const Language &language : languages()) {
        if (language.code == code)
            return language.nativeName;
    }
    return code;
}

//---------------------------------------------------------------------------
QString systemLanguage()
{
    const QString localeName = QLocale::system().name();   // e.g. "ja_JP"

    // Exact match first, e.g. zh_TW or pt_BR.
    if (isKnown(localeName))
        return localeName;

    // Then fall back to the language part, e.g. "de_AT" -> "de".
    const QString language = localeName.section(QLatin1Char('_'), 0, 0);
    if (isKnown(language))
        return language;

    // Simplified Chinese is the sensible default for any other zh_* variant.
    if (language == QLatin1String("zh"))
        return QStringLiteral("zh_CN");

    return QStringLiteral("en");
}

//---------------------------------------------------------------------------
QString currentLanguage()
{
    return loadLanguage();
}

//---------------------------------------------------------------------------
bool setLanguage(const QString &code)
{
    if (code == QLatin1String(kSystemCode)) {
        const QString detected = systemLanguage();
        if (detected == QLatin1String("en")) {
            // English is the source language: nothing to install.
            uninstall();
            return true;
        }
        return install(detected);
    }

    if (code == QLatin1String("en")) {
        uninstall();
        return true;
    }

    if (!install(code)) {
        uninstall();
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------
QString loadLanguage()
{
    QSettings settings;
    const QString stored =
        settings.value(QLatin1String(kLanguageKey),
                       QLatin1String(kSystemCode)).toString();

    if (stored.isEmpty())
        return QLatin1String(kSystemCode);
    return stored;
}

//---------------------------------------------------------------------------
void saveLanguage(const QString &code)
{
    QSettings settings;
    settings.setValue(QLatin1String(kLanguageKey), code);
}

//---------------------------------------------------------------------------
void applyStoredLanguage()
{
    const QString stored = loadLanguage();

    if (stored == QLatin1String(kSystemCode)) {
        setLanguage(kSystemCode);
        return;
    }

    // A stale preference for a language we no longer ship: fall back to system.
    if (!isKnown(stored)) {
        setLanguage(kSystemCode);
        return;
    }

    setLanguage(stored);
}

} // namespace i18n
