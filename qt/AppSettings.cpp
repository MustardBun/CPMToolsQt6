//---------------------------------------------------------------------------
#include "AppSettings.h"

// Copyright (C) 2026 Joseph Kwok (@MustardBun)
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>
#include <QFont>
#include <QSettings>

namespace app {

namespace {

//! QSettings key holding the FontSize enum value.
const char *const kFontSizeKey = "ui/fontSize";

//! The platform default font, captured before we ever modify it. Scaling is
//! always relative to this so repeated changes cannot accumulate rounding
//! error or drift.
QFont baseFont()
{
    static QFont base;
    static bool captured = false;
    if (!captured) {
        base = QApplication::font();
        captured = true;
    }
    return base;
}

//! Relative scale for each setting.
double scaleFor(FontSize size)
{
    switch (size) {
    case FontSize::Small:      return 0.85;
    case FontSize::Large:      return 1.25;
    case FontSize::ExtraLarge: return 1.6;
    case FontSize::Medium:     break;
    }
    return 1.0;
}

} // namespace

//---------------------------------------------------------------------------
QString fontSizeName(FontSize size)
{
    switch (size) {
    case FontSize::Small:      return QStringLiteral("Small");
    case FontSize::Large:      return QStringLiteral("Large");
    case FontSize::ExtraLarge: return QStringLiteral("Extra large");
    case FontSize::Medium:     break;
    }
    return QStringLiteral("Medium");
}

//---------------------------------------------------------------------------
FontSize loadFontSize()
{
    QSettings settings;
    const int stored = settings.value(QLatin1String(kFontSizeKey),
                                      static_cast<int>(FontSize::Medium)).toInt();

    switch (stored) {
    case static_cast<int>(FontSize::Small):      return FontSize::Small;
    case static_cast<int>(FontSize::Large):      return FontSize::Large;
    case static_cast<int>(FontSize::ExtraLarge): return FontSize::ExtraLarge;
    default:                                     return FontSize::Medium;
    }
}

//---------------------------------------------------------------------------
void saveFontSize(FontSize size)
{
    QSettings settings;
    settings.setValue(QLatin1String(kFontSizeKey), static_cast<int>(size));
}

//---------------------------------------------------------------------------
void applyFontSize(FontSize size)
{
    QFont font = baseFont();
    const double scale = scaleFor(size);

    // Point size is the normal case; fall back to pixels for fonts that only
    // define one or the other.
    if (font.pointSizeF() > 0.0) {
        font.setPointSizeF(font.pointSizeF() * scale);
    } else if (font.pixelSize() > 0) {
        font.setPixelSize(qMax(1, qRound(font.pixelSize() * scale)));
    }

    QApplication::setFont(font);
}

//---------------------------------------------------------------------------
void applyStoredFontSize()
{
    applyFontSize(loadFontSize());
}

} // namespace app
