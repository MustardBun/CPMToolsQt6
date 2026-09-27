//---------------------------------------------------------------------------
// uishot - development aid: render the main window off-screen to a PNG.
//
//   uishot <image.dsk> <output.png> [width] [fontSize 0|1|2] [lang]
//
// \a lang forces a language code (en, ja, zh_CN, ...); otherwise the stored
// preference is used. Handy for checking a translation without changing the
// user's settings.
//
// Used to inspect the layout without a display. Not part of the shipped app.
//---------------------------------------------------------------------------
#include "AppSettings.h"
#include "MainWindow.h"
#include "Translator.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QPixmap>
#include <QTimer>

#include <cstdio>

int main(int argc, char *argv[])
{
    // Run without a window server so this works over SSH / in CI.
    qputenv("QT_QPA_PLATFORM", "offscreen");

    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("CPMToolsQt6"));
    QCoreApplication::setOrganizationName(QStringLiteral("MustardBun"));

    const QString image  = (argc > 1) ? QString::fromLocal8Bit(argv[1]) : QString();
    const QString output = (argc > 2) ? QString::fromLocal8Bit(argv[2]) : QStringLiteral("shot.png");
    const int     width  = (argc > 3) ? QString::fromLocal8Bit(argv[3]).toInt() : 1200;
    const int     font   = (argc > 4) ? QString::fromLocal8Bit(argv[4]).toInt() : 1;
    const QString lang   = (argc > 5) ? QString::fromLocal8Bit(argv[5]) : QString();

    app::applyFontSize(static_cast<app::FontSize>(font));

    // Diagnostic: show what the resources actually contain when asked.
    if (qEnvironmentVariableIsSet("UISHOT_DEBUG")) {
        const QStringList entries = QDir(QStringLiteral(":/i18n")).entryList();
        std::fprintf(stderr, "[dbg] :/i18n -> %s\n",
                     qPrintable(entries.join(QStringLiteral(", "))));
        std::fprintf(stderr, "[dbg] :/LICENSE present: %s\n",
                     QFile::exists(QStringLiteral(":/LICENSE")) ? "yes" : "no");
    }

    if (lang.isEmpty())
        i18n::applyStoredLanguage();
    else
        i18n::setLanguage(lang);

    if (qEnvironmentVariableIsSet("UISHOT_DEBUG"))
        std::fprintf(stderr, "[dbg] requested '%s' -> current '%s'\n",
                     qPrintable(lang), qPrintable(i18n::currentLanguage()));

    MainWindow window;
    window.resize(width, 640);
    if (!image.isEmpty())
        window.openImage(image);
    window.show();

    // Give Qt a moment to lay out, load the directory listing and paint.
    QTimer::singleShot(1200, &app, [&window, output]() {
        const QPixmap shot = window.grab();
        const bool ok = shot.save(output);
        qInfo("saved %s: %s (%dx%d)", qPrintable(output), ok ? "ok" : "FAILED",
              shot.width(), shot.height());
        QCoreApplication::quit();
    });

    return app.exec();
}
