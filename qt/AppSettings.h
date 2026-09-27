//---------------------------------------------------------------------------
// AppSettings - persisted user preferences for the Qt front end.
//
// Currently just the UI font size. The size is stored as an enum and applied
// by scaling the platform's default font, so it behaves sensibly at any DPI
// and never compounds when the user switches back and forth.
//
// Copyright (C) 2026 Joseph Kwok (@MustardBun)
// SPDX-License-Identifier: GPL-3.0-or-later
//---------------------------------------------------------------------------
#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QString>

namespace app {

//! Selectable UI font sizes. Keep the order - it is the menu order.
enum class FontSize {
    Small  = 0,
    Medium = 1,   //!< the platform default
    Large  = 2,
    ExtraLarge = 3
};

//! Human readable name, e.g. "Medium".
QString fontSizeName(FontSize size);

//! Read the stored preference, falling back to Medium.
FontSize loadFontSize();

//! Persist the preference (does not apply it; see applyFontSize).
void saveFontSize(FontSize size);

//! Scale and install the application-wide font. Safe to call repeatedly.
void applyFontSize(FontSize size);

//! Load the stored preference and apply it. Call once during start-up.
void applyStoredFontSize();

} // namespace app

#endif // APPSETTINGS_H
