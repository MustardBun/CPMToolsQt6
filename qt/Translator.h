//---------------------------------------------------------------------------
// Translator - runtime language selection for the Qt front end.
//
// The compiled .qm catalogues are embedded in the executable under the
// resource prefix /i18n, so no files need to ship alongside the binary.
//
// Copyright (C) 2026 Joseph Kwok (@MustardBun)
// SPDX-License-Identifier: GPL-3.0-or-later
//---------------------------------------------------------------------------
#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QString>
#include <QVector>

namespace i18n {

//! A selectable interface language.
struct Language
{
    QString code;        //!< catalogue suffix, e.g. "ja" or "zh_TW"
    QString nativeName;  //!< shown in the menu, e.g. "日本語"
};

//! Sentinel used by setLanguage() to mean "follow the operating system".
extern const char *const kSystemCode;

//! All languages that have a compiled catalogue, in menu order.
QVector<Language> languages();

//! Native display name for \a code, or the code itself when unknown.
QString nativeName(const QString &code);

//! Best catalogue for the current OS locale (never empty).
QString systemLanguage();

//! The language currently installed, or kSystemCode when following the system.
QString currentLanguage();

//! Install the catalogue for \a code. Pass kSystemCode to follow the OS.
//! Returns false only when an explicitly requested catalogue is missing.
bool setLanguage(const QString &code);

//! Read the stored preference and install it. Call once during start-up.
void applyStoredLanguage();

//! Persist \a code as the preference.
void saveLanguage(const QString &code);

//! Load the stored preference (may be kSystemCode).
QString loadLanguage();

} // namespace i18n

#endif // TRANSLATOR_H
