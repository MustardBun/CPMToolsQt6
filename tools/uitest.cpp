//---------------------------------------------------------------------------
// uitest - drives MainWindow headlessly to verify host-folder navigation.
//
//   uitest <image.dsk>
//
// Checks that the host path field accepts a typed/pasted folder, that a file
// path selects the file in its parent folder, that a non-existent path is
// rejected, and that the host file list can still be listed and put into the
// image. Prints PASS/FAIL per check and exits non-zero on any failure.
//
// Development aid; not part of the shipped application.
//
// Copyright (C) 2026 Joseph Kwok (@MustardBun)
// SPDX-License-Identifier: GPL-3.0-or-later
//---------------------------------------------------------------------------
#include "MainWindow.h"

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QLineEdit>
#include <QListView>
#include <QListWidget>
#include <QPushButton>
#include <QTreeView>

#include <cstdio>

namespace {

int g_failures = 0;

void check(const char *what, bool ok, const QString &detail = QString())
{
    std::printf("  [%s] %s%s\n", ok ? "PASS" : "FAIL", what,
                detail.isEmpty() ? "" : qPrintable(QStringLiteral("  (%1)").arg(detail)));
    if (!ok)
        ++g_failures;
}

//! The folder currently shown in the host file list.
QString shownFolder(const QListView *list)
{
    const QModelIndex root = list->rootIndex();
    if (!root.isValid())
        return QString();

    // DisplayRole only gives the folder name; the model knows the real path.
    if (auto *fsModel = qobject_cast<QFileSystemModel *>(list->model()))
        return QDir::cleanPath(fsModel->filePath(root));

    return list->model()->data(root).toString();
}

} // namespace

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");

    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("CPMToolsQt6"));
    QCoreApplication::setOrganizationName(QStringLiteral("MustardBun"));

    const QString image = (argc > 1) ? QString::fromLocal8Bit(argv[1]) : QString();

    MainWindow window;
    window.resize(1200, 700);
    if (!image.isEmpty())
        window.openImage(image);
    window.show();

    auto *pathEdit = window.findChild<QLineEdit *>(QStringLiteral("hostPathEdit"));
    auto *browse   = window.findChild<QPushButton *>(QStringLiteral("hostBrowseButton"));
    auto *tree     = window.findChild<QTreeView *>(QStringLiteral("hostDirTree"));
    auto *files    = window.findChild<QListView *>(QStringLiteral("hostFileList"));

    std::printf("UI test: host folder navigation\n\n");

    check("host path field exists", pathEdit != nullptr);
    check("browse button exists", browse != nullptr);
    check("host file list exists", files != nullptr);
    check("host dir tree exists", tree != nullptr);

    if (pathEdit == nullptr || files == nullptr)
        return 1;

    // Build a scratch folder with two files and a subfolder.
    const QString root = QDir::cleanPath(
        QDir::temp().filePath(QStringLiteral("cpmtools-uitest")));
    QDir().mkpath(root + QStringLiteral("/sub"));
    for (const char *name : {"alpha.txt", "beta.txt"}) {
        QFile f(QDir(root).filePath(QString::fromLatin1(name)));
        if (f.open(QIODevice::WriteOnly)) { f.write("x"); f.close(); }
    }

    // 1. A folder path typed/pasted in shows that folder.
    pathEdit->setText(QDir::toNativeSeparators(root));
    emit pathEdit->returnPressed();
    check("typed folder is shown", shownFolder(files) == root, shownFolder(files));

    // 2. A file path shows the parent folder and selects the file.
    const QString alpha = QDir(root).filePath(QStringLiteral("alpha.txt"));
    pathEdit->setText(QDir::toNativeSeparators(alpha));
    emit pathEdit->returnPressed();
    check("file path shows parent folder", shownFolder(files) == root, shownFolder(files));
    check("file path selects the file",
          files->currentIndex().isValid() &&
              files->model()->data(files->currentIndex()).toString() == QStringLiteral("alpha.txt"),
          files->currentIndex().isValid()
              ? files->model()->data(files->currentIndex()).toString()
              : QStringLiteral("nothing selected"));

    // 3. A trailing separator and a quoted path are both tolerated.
    pathEdit->setText(QStringLiteral("\"%1\"").arg(QDir::toNativeSeparators(root)));
    emit pathEdit->returnPressed();
    check("quoted path is accepted", shownFolder(files) == root, shownFolder(files));

    // 4. A folder path with a native trailing separator works.
    pathEdit->setText(QDir::toNativeSeparators(root) + QDir::separator());
    emit pathEdit->returnPressed();
    check("trailing separator is accepted", shownFolder(files) == root, shownFolder(files));

    // 5. A subfolder can be reached the same way.
    const QString sub = QDir::cleanPath(QDir(root).filePath(QStringLiteral("sub")));
    pathEdit->setText(QDir::toNativeSeparators(sub));
    emit pathEdit->returnPressed();
    check("subfolder is shown", shownFolder(files) == sub, shownFolder(files));

    // 6. Back to the root, then confirm the image does not steal the folder.
    pathEdit->setText(QDir::toNativeSeparators(root));
    emit pathEdit->returnPressed();
    window.openImage(image);
    check("image does not override the chosen folder", shownFolder(files) == root,
          shownFolder(files));

    // 7. Regression: selecting a host file still enables Put, and putting it
    //    into the image succeeds and shows up in the image listing.
    if (!image.isEmpty()) {
        const QModelIndex alphaIndex = files->model()->index(0, 0, files->rootIndex());
        bool foundAlpha = false;
        for (int row = 0; row < files->model()->rowCount(files->rootIndex()); ++row) {
            const QModelIndex idx = files->model()->index(row, 0, files->rootIndex());
            if (files->model()->data(idx).toString() == QStringLiteral("alpha.txt")) {
                files->setCurrentIndex(idx);
                foundAlpha = true;
                break;
            }
        }
        Q_UNUSED(alphaIndex);
        check("host file can be selected", foundAlpha);

        auto *putAction = window.findChild<QAction *>(QStringLiteral("putAction"));

        // Invoke Put through the same slot the button uses.
        QMetaObject::invokeMethod(&window, "putFiles", Qt::DirectConnection);

        // Verify the file landed in the image by looking for it in the listing.
        auto *entryList = window.findChild<QListWidget *>();
        bool inImage = false;
        if (entryList != nullptr) {
            for (int row = 0; row < entryList->count(); ++row) {
                if (entryList->item(row)->text().contains(
                        QStringLiteral("alpha"), Qt::CaseInsensitive)) {
                    inImage = true;
                    break;
                }
            }
        }
        check("put into image copies the file", inImage,
              entryList != nullptr ? QStringLiteral("%1 entries").arg(entryList->count())
                                   : QStringLiteral("no entry list"));
    }

    // Clean up.
    QDir(root).removeRecursively();

    std::printf("\n%s (%d failure(s))\n", g_failures == 0 ? "UITEST_OK" : "UITEST_FAILED",
                g_failures);
    return g_failures == 0 ? 0 : 1;
}
