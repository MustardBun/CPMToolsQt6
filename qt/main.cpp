//---------------------------------------------------------------------------
// CPMToolsQt6 - application entry point.
//
// Copyright (C) 2026 Joseph Kwok (@MustardBun)
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Ported from CPMToolsGUI by neko Java.
// cpmtools engine (c) Michael Haardt.
//---------------------------------------------------------------------------
#include "MainWindow.h"

#include "AppSettings.h"
#include "Translator.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("CPMToolsQt6"));
    QCoreApplication::setApplicationVersion(QStringLiteral("v0.1-preview"));
    QCoreApplication::setOrganizationName(QStringLiteral("MustardBun"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("github.com/MustardBun"));

    // Restore the saved language and font size before any window is created,
    // so the window is laid out at the right size from the first frame.
    i18n::applyStoredLanguage();
    app::applyStoredFontSize();

    MainWindow window;
    window.show();

    // Optional image passed on the command line (mirrors the old ParamStr(1)).
    if (argc > 1) {
        const QString candidate = QString::fromLocal8Bit(argv[1]);
        if (QFileInfo::exists(candidate))
            window.openImage(QDir::toNativeSeparators(candidate));
    }

    return app.exec();
}
