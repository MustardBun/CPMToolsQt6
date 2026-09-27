//---------------------------------------------------------------------------
// MainWindow - the main CP/M image browser window.
//
// Copyright (C) 2026 Joseph Kwok (@MustardBun)
// SPDX-License-Identifier: GPL-3.0-or-later
//---------------------------------------------------------------------------
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QStringList>
#include <QVector>

#include "CpmBackend.h"

class QAction;
class QActionGroup;
class QCloseEvent;
class QComboBox;
class QEvent;
class QFileSystemModel;
class QListView;
class QListWidget;
class QPushButton;
class QToolBar;
class QTreeView;

class FileDropLineEdit;
class FileDropListWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

public slots:
    //! Show \a path as the current image (also used for the command line).
    void openImage(const QString &path);

private slots:
    void chooseImage();
    void imageTextDropped(const QString &path);
    void imageChanged();
    void refreshFormats();
    void refreshDirectory();
    void getFiles();
    void putFiles();
    void deleteSelected();
    void showMkfs();
    void showAbout();
    void openOnlineHelp();
    void onFilesDropped(const QStringList &paths);
    void startDragOut();
    void updateActions();
    void changeFontSize(QAction *action);
    void changeLanguage(QAction *action);
    //! Browse for a host folder or file, and show it.
    void chooseHostFolder();
    //! Apply a path typed or pasted into the host path field.
    void hostPathEdited();
    //! Apply a folder or file dropped onto the host path field.
    void hostPathDropped(const QString &path);

protected:
    void closeEvent(QCloseEvent *event) override;
    //! Re-applies every registered UI string after a language switch.
    void changeEvent(QEvent *event) override;

private:
    //! A UI string that must be re-translated when the language changes.
    struct TrEntry
    {
        QObject    *target;    //!< widget, action or menu
        const char *property;  //!< e.g. "text", "title", "windowTitle"
        QString     source;    //!< the original English source string
    };

    void buildUi();
    void buildSettingsMenu();
    void buildMenus();
    void connectSignals();

    //! Set \a property now and remember it for later retranslation.
    void setTr(QObject *object, const char *property, const QString &text);

    //! Re-apply every registered string; called on QEvent::LanguageChange.
    void retranslateUi();

    QString  currentFormat() const;
    QString  currentPattern() const;
    QString  currentImage() const;
    QString  hostDirectory() const;
    //! Files chosen on the host side, from either the folder tree or the list.
    QStringList selectedHostFiles() const;

    //! Show \a path as the host folder. Does nothing if it is not a directory.
    void setHostFolder(const QString &path);

    //! Show \a path, treating a directory as the folder and a file as an
    //! entry inside its parent folder. Reports a warning if it is missing.
    void applyHostPath(const QString &path);

    //! Highlight \a filePath in the host file list.
    void revealHostFile(const QString &filePath);
    void     reportError(const QString &title, const cpm::Result &result);
    void     cleanupTempDir();

    cpm::Backend *m_backend = nullptr;

    // image side
    FileDropLineEdit   *m_imageEdit    = nullptr;
    QPushButton        *m_selectButton = nullptr;
    QListWidget        *m_formatList   = nullptr;
    FileDropListWidget *m_entryList    = nullptr;
    QComboBox          *m_patternCombo = nullptr;

    // host side
    QFileSystemModel *m_fsModel     = nullptr;
    QTreeView        *m_dirTree     = nullptr;
    QListView        *m_hostFiles   = nullptr;
    QComboBox        *m_filterCombo = nullptr;
    //! Editable path of the host folder, so a path can be pasted in.
    FileDropLineEdit *m_hostPathEdit     = nullptr;
    QPushButton      *m_hostBrowseButton = nullptr;
    //! The host folder currently shown in the pane.
    QString m_hostFolder;
    //! True once the user has chosen a folder; stops the image from moving it.
    bool    m_hostFolderPinned = false;

    // actions
    QAction *m_getAction    = nullptr;
    QAction *m_putAction    = nullptr;
    QAction *m_deleteAction = nullptr;
    QActionGroup *m_fontSizeGroup = nullptr;
    QActionGroup *m_languageGroup = nullptr;

    //! Every registered UI string, replayed by retranslateUi().
    QVector<TrEntry> m_trEntries;

    QToolBar *m_toolBar = nullptr;

    QString m_tempDir;          //!< temp folder used for the current drag-out
    QString m_pendingTempDir;   //!< temp folder awaiting deletion
    int     m_cleanupRetries = 0;
};

#endif // MAINWINDOW_H
