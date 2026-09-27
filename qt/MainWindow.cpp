//---------------------------------------------------------------------------
// MainWindow - the main CP/M image browser window.
//
// Copyright (C) 2026 Joseph Kwok (@MustardBun)
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Derived from the original CpmtoolsGUI by neko Java / Koji Suzuki, which is
// in turn built on the cpmtools engine by Michael Haardt.
//---------------------------------------------------------------------------
#include "MainWindow.h"

#include "AboutDialog.h"
#include "AppSettings.h"
#include "FileDropLineEdit.h"
#include "FileDropListWidget.h"
#include "MkfsDialog.h"
#include "Translator.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QDrag>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeySequence>
#include <QLabel>
#include <QListView>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QSplitter>
#include <QStatusBar>
#include <QStyle>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QTreeView>
#include <QUrl>
#include <QVBoxLayout>

namespace {

//! CP/M wildcard patterns offered for the image listing.
const char *const kPatterns[] = {
    "*.*", "*.com", "*.hex", "*.mac", "*.asm", "*.c",
    "*.h", "*.prn", "*.lib", "*.bas", "*.rel", "*.pas"
};

//! Name filters offered for the host file browser.
const char *const kHostFilters[] = {
    "*.*", "*.c", "*.h", "*.com", "*.hex", "*.asm",
    "*.mac", "*.prn", "*.lib", "*.pas", "*.bin", "*.dsk", "*.ddi"
};

} // namespace

//---------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_backend = new cpm::Backend();

    buildUi();
    buildMenus();
    connectSignals();

    refreshFormats();
    refreshDirectory();

    // Start in the user's home folder rather than showing bare drive letters.
    setHostFolder(QDir::homePath());
    m_hostFolderPinned = false;   // the image may still claim it

    updateActions();

    statusBar()->showMessage(tr("Ready"));
}

//---------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    delete m_backend;
}

//---------------------------------------------------------------------------
void MainWindow::buildUi()
{
    setTr(this, "windowTitle", tr("CPMToolsQt6"));
    resize(1100, 720);

    setIconSize(QSize(16, 16));

    // ------------------------------------------------------------- image row
    m_imageEdit = new FileDropLineEdit(this);
    m_imageEdit->setReadOnly(true);
    setTr(m_imageEdit, "placeholderText",
          tr("Drop a disk image here, or press Select..."));
    setTr(m_imageEdit, "toolTip", tr("Disk image to inspect (.dsk, .ddi, .img)"));

    m_selectButton = new QPushButton(this);
    setTr(m_selectButton, "text", tr("Select..."));

    auto *imageLabel = new QLabel(this);
    setTr(imageLabel, "text", tr("Image file"));

    auto *imageRow = new QHBoxLayout;
    imageRow->addWidget(imageLabel);
    imageRow->addWidget(m_imageEdit, 1);
    imageRow->addWidget(m_selectButton);

    auto *imageGroup = new QGroupBox(this);
    imageGroup->setLayout(imageRow);

    // ----------------------------------------------------------- format pane
    m_formatList = new QListWidget(this);
    m_formatList->setAlternatingRowColors(true);
    m_formatList->setUniformItemSizes(true);

    auto *formatPane = new QGroupBox(this);
    setTr(formatPane, "title", tr("Format"));
    auto *formatLayout = new QVBoxLayout(formatPane);
    formatLayout->addWidget(m_formatList);

    // ------------------------------------------------------- image file pane
    m_entryList = new FileDropListWidget(this);
    setTr(m_entryList, "toolTip",
          tr("Files inside the image. Drag out to copy them to your desktop, "
             "or drop files here to add them."));

    m_patternCombo = new QComboBox(this);
    m_patternCombo->setEditable(false);
    for (const char *pattern : kPatterns)
        m_patternCombo->addItem(QString::fromLatin1(pattern));

    auto *patternLabel = new QLabel(this);
    setTr(patternLabel, "text", tr("Show"));

    auto *patternRow = new QHBoxLayout;
    patternRow->addWidget(patternLabel);
    patternRow->addWidget(m_patternCombo, 1);

    auto *entryPane = new QGroupBox(this);
    setTr(entryPane, "title", tr("Files in image"));
    auto *entryLayout = new QVBoxLayout(entryPane);
    entryLayout->addWidget(m_entryList, 1);
    entryLayout->addLayout(patternRow);

    // ----------------------------------------------------------- host pane
    m_fsModel = new QFileSystemModel(this);
    m_fsModel->setRootPath(QString());
    m_fsModel->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Drives);
    m_fsModel->setNameFilterDisables(false);

    m_dirTree = new QTreeView(this);
    m_dirTree->setObjectName(QStringLiteral("hostDirTree"));
    m_dirTree->setModel(m_fsModel);
    m_dirTree->setHeaderHidden(true);
    m_dirTree->setAnimated(true);
    m_dirTree->setUniformRowHeights(true);
    for (int column = 1; column < m_fsModel->columnCount(); ++column)
        m_dirTree->hideColumn(column);

    m_hostFiles = new QListView(this);
    m_hostFiles->setObjectName(QStringLiteral("hostFileList"));
    m_hostFiles->setModel(m_fsModel);
    m_hostFiles->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_hostFiles->setViewMode(QListView::ListMode);
    m_hostFiles->setUniformItemSizes(true);
    m_hostFiles->setAlternatingRowColors(true);

    m_filterCombo = new QComboBox(this);
    m_filterCombo->setEditable(false);
    for (const char *filter : kHostFilters)
        m_filterCombo->addItem(QString::fromLatin1(filter));

    auto *filterLabel = new QLabel(this);
    setTr(filterLabel, "text", tr("Filter"));

    auto *filterRow = new QHBoxLayout;
    filterRow->addWidget(filterLabel);
    filterRow->addWidget(m_filterCombo, 1);

    // A folder path that can be typed or pasted directly, plus a browse button
    // that accepts either a folder or a single file. The tree below stays for
    // quick navigation, but it is no longer the only way to reach a folder.
    m_hostPathEdit = new FileDropLineEdit(this);
    m_hostPathEdit->setObjectName(QStringLiteral("hostPathEdit"));
    setTr(m_hostPathEdit, "placeholderText",
          tr("Folder path - type or paste, or press Browse..."));
    setTr(m_hostPathEdit, "toolTip",
          tr("Path of the folder shown below. A file path selects that file "
             "in its folder."));

    m_hostBrowseButton = new QPushButton(this);
    m_hostBrowseButton->setObjectName(QStringLiteral("hostBrowseButton"));
    setTr(m_hostBrowseButton, "text", tr("Browse..."));
    setTr(m_hostBrowseButton, "toolTip",
          tr("Choose a folder, or a file inside a folder"));

    auto *hostPathRow = new QHBoxLayout;
    hostPathRow->addWidget(m_hostPathEdit, 1);
    hostPathRow->addWidget(m_hostBrowseButton);

    auto *hostPane = new QGroupBox(this);
    setTr(hostPane, "title", tr("Files on this computer"));
    auto *hostLayout = new QVBoxLayout(hostPane);
    hostLayout->addLayout(hostPathRow);
    hostLayout->addWidget(m_dirTree, 1);
    hostLayout->addWidget(m_hostFiles, 2);
    hostLayout->addLayout(filterRow);

    // --------------------------------------------------------------- splitter
    auto *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(formatPane);
    splitter->addWidget(entryPane);
    splitter->addWidget(hostPane);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);
    splitter->setStretchFactor(2, 2);
    splitter->setChildrenCollapsible(false);

    // ------------------------------------------------------------ action bar
    m_getAction = new QAction(this);
    m_putAction = new QAction(this);
    m_deleteAction = new QAction(this);

    m_getAction->setObjectName(QStringLiteral("getAction"));
    m_putAction->setObjectName(QStringLiteral("putAction"));
    m_deleteAction->setObjectName(QStringLiteral("deleteAction"));

    setTr(m_getAction, "text", tr("&Get from image"));
    setTr(m_putAction, "text", tr("&Put into image"));
    setTr(m_deleteAction, "text", tr("&Delete"));

    // Arrows point in the direction the data travels: the image pane is on the
    // left and the host file pane on the right.
    //   Put into image  ->  arrow points left, towards the image
    //   Get from image  ->  arrow points right, towards the host files
    m_putAction->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
    m_getAction->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
    m_deleteAction->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));

    setTr(m_getAction, "toolTip",
          tr("Copy the selected image files to the folder shown on the right"));
    setTr(m_putAction, "toolTip",
          tr("Copy the files selected on the right into the image"));
    setTr(m_deleteAction, "toolTip",
          tr("Delete the selected files from the image"));

    // QToolButton understands QActions directly and keeps text + icon in sync
    // with the menu entries (enabled state included).
    auto *getButton    = new QToolButton(this);
    auto *putButton    = new QToolButton(this);
    auto *deleteButton = new QToolButton(this);

    for (QToolButton *button : { getButton, putButton, deleteButton }) {
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setAutoRaise(false);
    }
    getButton->setDefaultAction(m_getAction);
    putButton->setDefaultAction(m_putAction);
    deleteButton->setDefaultAction(m_deleteAction);

    auto *actionBar = new QHBoxLayout;
    actionBar->addWidget(getButton);
    actionBar->addWidget(putButton);
    actionBar->addWidget(deleteButton);
    actionBar->addStretch(1);

    // ------------------------------------------------------------------ main
    auto *central = new QWidget(this);
    auto *layout  = new QVBoxLayout(central);
    layout->addWidget(imageGroup);
    layout->addWidget(splitter, 1);
    layout->addLayout(actionBar);
    setCentralWidget(central);
}

//---------------------------------------------------------------------------
void MainWindow::buildMenus()
{
    // ------------------------------------------------------------------ File
    auto *fileMenu = menuBar()->addMenu(QString());
    setTr(fileMenu, "title", tr("&File"));

    auto *openAction = fileMenu->addAction(QString());
    setTr(openAction, "text", tr("&Open image..."));
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::chooseImage);

    auto *newAction = fileMenu->addAction(QString());
    setTr(newAction, "text", tr("&New image..."));
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::showMkfs);

    fileMenu->addSeparator();

    auto *exitAction = fileMenu->addAction(QString());
    setTr(exitAction, "text", tr("E&xit"));
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    // ------------------------------------------------------------------ Edit
    auto *editMenu = menuBar()->addMenu(QString());
    setTr(editMenu, "title", tr("&Edit"));
    editMenu->addAction(m_getAction);
    editMenu->addAction(m_putAction);
    m_deleteAction->setShortcut(QKeySequence::Delete);
    editMenu->addAction(m_deleteAction);

    connect(m_getAction,    &QAction::triggered, this, &MainWindow::getFiles);
    connect(m_putAction,    &QAction::triggered, this, &MainWindow::putFiles);
    connect(m_deleteAction, &QAction::triggered, this, &MainWindow::deleteSelected);

    // -------------------------------------------------------------- Settings
    buildSettingsMenu();

    // ------------------------------------------------------------------ Help
    auto *helpMenu = menuBar()->addMenu(QString());
    setTr(helpMenu, "title", tr("&Help"));

    auto *onlineAction = helpMenu->addAction(QString());
    setTr(onlineAction, "text", tr("On&line"));
    connect(onlineAction, &QAction::triggered, this, &MainWindow::openOnlineHelp);

    auto *aboutAction = helpMenu->addAction(QString());
    setTr(aboutAction, "text", tr("&About"));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);

    // --------------------------------------------------------------- toolbar
    // Get/Put/Delete also appear in the button row below the panes, so the
    // toolbar carries only the file-level commands.
    m_toolBar = addToolBar(QString());
    setTr(m_toolBar, "windowTitle", tr("Main"));
    m_toolBar->setMovable(false);
    m_toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_toolBar->addAction(openAction);
    m_toolBar->addAction(newAction);
}

//---------------------------------------------------------------------------
void MainWindow::buildSettingsMenu()
{
    auto *settingsMenu = menuBar()->addMenu(QString());
    setTr(settingsMenu, "title", tr("&Settings"));

    // ------------------------------------------------------------ font size
    auto *fontMenu = settingsMenu->addMenu(QString());
    setTr(fontMenu, "title", tr("UI &Font Size"));

    // A radio group; the checked entry mirrors the stored preference.
    m_fontSizeGroup = new QActionGroup(this);
    m_fontSizeGroup->setExclusive(true);

    auto *smallAction = fontMenu->addAction(QString());
    setTr(smallAction, "text", tr("&Small"));
    smallAction->setCheckable(true);
    smallAction->setData(static_cast<int>(app::FontSize::Small));
    m_fontSizeGroup->addAction(smallAction);

    auto *mediumAction = fontMenu->addAction(QString());
    setTr(mediumAction, "text", tr("&Medium"));
    setTr(mediumAction, "toolTip", tr("The system default size"));
    mediumAction->setCheckable(true);
    mediumAction->setData(static_cast<int>(app::FontSize::Medium));
    m_fontSizeGroup->addAction(mediumAction);

    auto *largeAction = fontMenu->addAction(QString());
    setTr(largeAction, "text", tr("&Large"));
    largeAction->setCheckable(true);
    largeAction->setData(static_cast<int>(app::FontSize::Large));
    m_fontSizeGroup->addAction(largeAction);

    auto *extraLargeAction = fontMenu->addAction(QString());
    setTr(extraLargeAction, "text", tr("&Extra large"));
    extraLargeAction->setCheckable(true);
    extraLargeAction->setData(static_cast<int>(app::FontSize::ExtraLarge));
    m_fontSizeGroup->addAction(extraLargeAction);

    const int stored = static_cast<int>(app::loadFontSize());
    for (QAction *action : m_fontSizeGroup->actions())
        action->setChecked(action->data().toInt() == stored);

    connect(m_fontSizeGroup, &QActionGroup::triggered,
            this, &MainWindow::changeFontSize);

    // ------------------------------------------------------------- language
    auto *languageMenu = settingsMenu->addMenu(QString());
    setTr(languageMenu, "title", tr("&Language"));

    m_languageGroup = new QActionGroup(this);
    m_languageGroup->setExclusive(true);

    auto *systemAction = languageMenu->addAction(QString());
    setTr(systemAction, "text", tr("&System default"));
    systemAction->setCheckable(true);
    systemAction->setData(QLatin1String(i18n::kSystemCode));
    m_languageGroup->addAction(systemAction);

    languageMenu->addSeparator();

    // Language names are shown in their own language, so they are deliberately
    // not passed through tr().
    for (const i18n::Language &language : i18n::languages()) {
        auto *action = languageMenu->addAction(language.nativeName);
        action->setCheckable(true);
        action->setData(language.code);
        m_languageGroup->addAction(action);
    }

    const QString storedLanguage = i18n::loadLanguage();
    for (QAction *action : m_languageGroup->actions())
        action->setChecked(action->data().toString() == storedLanguage);

    connect(m_languageGroup, &QActionGroup::triggered,
            this, &MainWindow::changeLanguage);
}

//---------------------------------------------------------------------------
void MainWindow::changeLanguage(QAction *action)
{
    if (action == nullptr)
        return;

    const QString code = action->data().toString();

    if (!i18n::setLanguage(code)) {
        QMessageBox::warning(this, tr("Language"),
                             tr("The translation for \"%1\" could not be "
                                "loaded.").arg(code));
        return;
    }

    i18n::saveLanguage(code);

    // Installing a translator posts QEvent::LanguageChange, which triggers
    // changeEvent() -> retranslateUi() on this window and its children.
    statusBar()->showMessage(
        tr("Language set to %1")
            .arg(code == QLatin1String(i18n::kSystemCode)
                     ? tr("System default")
                     : i18n::nativeName(code)),
        4000);
}

//---------------------------------------------------------------------------
void MainWindow::setTr(QObject *object, const char *property, const QString &text)
{
    if (object == nullptr)
        return;
    object->setProperty(property, text);
    m_trEntries.append(TrEntry{object, property, text});
}

//---------------------------------------------------------------------------
void MainWindow::retranslateUi()
{
    for (const TrEntry &entry : m_trEntries) {
        if (entry.target == nullptr)
            continue;
        entry.target->setProperty(
            entry.property,
            QCoreApplication::translate("MainWindow",
                                        entry.source.toUtf8().constData()));
    }
}

//---------------------------------------------------------------------------
void MainWindow::changeEvent(QEvent *event)
{
    if (event != nullptr && event->type() == QEvent::LanguageChange)
        retranslateUi();

    QMainWindow::changeEvent(event);
}

//---------------------------------------------------------------------------
void MainWindow::changeFontSize(QAction *action)
{
    if (action == nullptr)
        return;

    const auto size = static_cast<app::FontSize>(action->data().toInt());
    app::applyFontSize(size);
    app::saveFontSize(size);

    statusBar()->showMessage(tr("UI font size set to %1").arg(app::fontSizeName(size)),
                             4000);
}

//---------------------------------------------------------------------------
void MainWindow::connectSignals()
{
    connect(m_selectButton, &QPushButton::clicked, this, &MainWindow::chooseImage);
    connect(m_imageEdit, &FileDropLineEdit::fileDropped,
            this, &MainWindow::imageTextDropped);

    connect(m_formatList, &QListWidget::currentRowChanged,
            this, &MainWindow::refreshDirectory);
    connect(m_patternCombo, &QComboBox::currentTextChanged,
            this, &MainWindow::refreshDirectory);

    connect(m_entryList, &FileDropListWidget::filesDropped,
            this, &MainWindow::onFilesDropped);
    connect(m_entryList, &FileDropListWidget::dragOutRequested,
            this, &MainWindow::startDragOut);
    connect(m_entryList, &QListWidget::itemSelectionChanged,
            this, &MainWindow::updateActions);

    connect(m_dirTree, &QTreeView::clicked, this, [this](const QModelIndex &index) {
        const QString path = m_fsModel->filePath(index);
        if (path.isEmpty())
            return;

        const QFileInfo info(path);
        if (info.isDir()) {
            // Show the contents of the clicked folder. This is an explicit
            // choice, so it pins the pane against the image moving it later.
            setHostFolder(path);
        } else {
            // A file was clicked: show the folder that contains it and
            // highlight the file, so its neighbours are visible as well.
            setHostFolder(info.absolutePath());
            revealHostFile(path);
        }
    });

    connect(m_hostFiles, &QListView::doubleClicked,
            this, &MainWindow::putFiles);
    connect(m_hostFiles->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::updateActions);
    connect(m_dirTree->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::updateActions);

    // Host path field: typed/pasted text, the browse button, and a dropped
    // folder or file all lead to the same navigation.
    connect(m_hostPathEdit, &QLineEdit::returnPressed,
            this, &MainWindow::hostPathEdited);
    connect(m_hostPathEdit, &FileDropLineEdit::fileDropped,
            this, &MainWindow::hostPathDropped);
    connect(m_hostBrowseButton, &QPushButton::clicked,
            this, &MainWindow::chooseHostFolder);

    connect(m_filterCombo, &QComboBox::currentTextChanged, this, [this](const QString &filter) {
        if (filter == QLatin1String("*.*"))
            m_fsModel->setNameFilters(QStringList());
        else
            m_fsModel->setNameFilters(QStringList{filter});
    });
}

//---------------------------------------------------------------------------
void MainWindow::openImage(const QString &path)
{
    if (path.isEmpty())
        return;
    m_imageEdit->setText(QDir::toNativeSeparators(path));
    imageChanged();
}

//---------------------------------------------------------------------------
void MainWindow::imageTextDropped(const QString &path)
{
    openImage(path);
}

//---------------------------------------------------------------------------
void MainWindow::chooseImage()
{
    const QString path = QFileDialog::getOpenFileName(
        this, tr("Open CP/M disk image"), m_imageEdit->text(),
        tr("Disk images (*.dsk *.ddi *.img *.bin);;All files (*)"));
    if (path.isEmpty())
        return;

    openImage(path);
}

//---------------------------------------------------------------------------
void MainWindow::imageChanged()
{
    const QString image = currentImage();
    if (image.isEmpty())
        return;

    // A diskdefs next to the image takes precedence over the installed one.
    m_backend->useDiskdefsFrom(image);

    // Follow the image by default, but never override a folder the user chose
    // deliberately - that is a common workflow (image here, files there).
    if (!m_hostFolderPinned)
        setHostFolder(QFileInfo(image).absolutePath());

    refreshFormats();
    refreshDirectory();

    statusBar()->showMessage(tr("Image: %1").arg(QDir::toNativeSeparators(image)), 5000);
}

//---------------------------------------------------------------------------
void MainWindow::refreshFormats()
{
    const QString previous = currentFormat();

    QStringList formats;
    const cpm::Result result = m_backend->listFormats(formats);

    const QSignalBlocker blocker(m_formatList);
    m_formatList->clear();

    if (result.failed()) {
        auto *item = new QListWidgetItem(result.error(), m_formatList);
        item->setFlags(Qt::NoItemFlags);
        item->setForeground(Qt::red);
        return;
    }

    m_formatList->addItems(formats);

    if (!previous.isEmpty()) {
        const auto matches = m_formatList->findItems(previous, Qt::MatchExactly);
        if (!matches.isEmpty()) {
            m_formatList->setCurrentItem(matches.constFirst());
            return;
        }
    }
    if (m_formatList->count() > 0)
        m_formatList->setCurrentRow(0);
}

//---------------------------------------------------------------------------
void MainWindow::refreshDirectory()
{
    const QString image   = currentImage();
    const QString format  = currentFormat();
    const QString pattern = currentPattern();

    m_entryList->clear();

    if (image.isEmpty() || format.isEmpty())
        return;

    QList<cpm::Entry> entries;
    const cpm::Result result = m_backend->listDirectory(image, format, pattern, entries);

    if (result.failed()) {
        statusBar()->showMessage(result.error(), 8000);
        return;
    }

    const QIcon fileIcon = style()->standardIcon(QStyle::SP_FileIcon);
    for (const cpm::Entry &entry : entries) {
        auto *item = new QListWidgetItem(fileIcon, entry.name, m_entryList);
        item->setData(Qt::UserRole, entry.name);
        item->setToolTip(tr("%1 (user %2)").arg(entry.name).arg(entry.user));
    }

    statusBar()->showMessage(tr("%n file(s)", "", entries.size()), 4000);
    updateActions();
}

//---------------------------------------------------------------------------
void MainWindow::getFiles()
{
    const QString image  = currentImage();
    const QString format = currentFormat();

    const QList<QListWidgetItem *> selected = m_entryList->selectedItems();
    if (image.isEmpty() || format.isEmpty() || selected.isEmpty())
        return;

    const QString dir = hostDirectory();
    if (dir.isEmpty())
        return;

    QStringList failures;
    int copied = 0;

    for (auto *item : selected) {
        const QString name = item->data(Qt::UserRole).toString();
        const QString dest = QDir(dir).filePath(name);

        if (QFileInfo::exists(dest)) {
            const auto answer = QMessageBox::question(
                this, tr("Overwrite?"),
                tr("%1 already exists.\nOverwrite it?").arg(name),
                QMessageBox::Yes | QMessageBox::No);
            if (answer != QMessageBox::Yes)
                continue;
        }

        const cpm::Result result = m_backend->extract(image, format, name, dest);
        if (result.failed())
            failures << QStringLiteral("%1: %2").arg(name, result.error());
        else
            ++copied;
    }

    m_fsModel->setNameFilters(m_fsModel->nameFilters());   // refresh the view

    if (!failures.isEmpty())
        QMessageBox::warning(this, tr("Some files could not be copied"),
                             failures.join(QLatin1Char('\n')));

    statusBar()->showMessage(tr("Copied %1 file(s) to %2")
                                 .arg(copied)
                                 .arg(QDir::toNativeSeparators(dir)),
                             5000);
}

//---------------------------------------------------------------------------
void MainWindow::putFiles()
{
    const QString image  = currentImage();
    const QString format = currentFormat();

    if (image.isEmpty() || format.isEmpty()) {
        QMessageBox::information(this, tr("Put into image"),
                                 tr("Open a disk image first."));
        return;
    }

    const QStringList sources = selectedHostFiles();
    if (sources.isEmpty()) {
        QMessageBox::information(this, tr("Put into image"),
                                 tr("Select one or more files on the right first."));
        return;
    }

    QStringList failures;
    int copied = 0;

    for (const QString &source : sources) {
        const QString name = QFileInfo(source).fileName();

        const cpm::Result result = m_backend->insert(image, format, source, name);
        if (result.failed())
            failures << QStringLiteral("%1: %2").arg(name, result.error());
        else
            ++copied;
    }

    if (!failures.isEmpty())
        QMessageBox::warning(this, tr("Some files could not be copied"),
                             failures.join(QLatin1Char('\n')));

    if (copied > 0)
        refreshDirectory();

    statusBar()->showMessage(tr("Copied %1 of %2 file(s) into the image")
                                 .arg(copied)
                                 .arg(sources.size()),
                             5000);
}

//---------------------------------------------------------------------------
void MainWindow::deleteSelected()
{
    const QString image  = currentImage();
    const QString format = currentFormat();

    const QList<QListWidgetItem *> selected = m_entryList->selectedItems();
    if (image.isEmpty() || format.isEmpty() || selected.isEmpty())
        return;

    const auto answer = QMessageBox::question(
        this, tr("Delete"),
        tr("Delete %n selected file(s) from the image?", "", selected.size()),
        QMessageBox::Yes | QMessageBox::No);
    if (answer != QMessageBox::Yes)
        return;

    QStringList failures;
    for (auto *item : selected) {
        const QString name = item->data(Qt::UserRole).toString();
        const cpm::Result result = m_backend->remove(image, format, name);
        if (result.failed())
            failures << QStringLiteral("%1: %2").arg(name, result.error());
    }

    if (!failures.isEmpty())
        QMessageBox::warning(this, tr("Some files could not be deleted"),
                             failures.join(QLatin1Char('\n')));

    refreshDirectory();
}

//---------------------------------------------------------------------------
void MainWindow::onFilesDropped(const QStringList &paths)
{
    const QString image  = currentImage();
    const QString format = currentFormat();

    if (image.isEmpty() || format.isEmpty()) {
        QMessageBox::information(this, tr("Add files"),
                                 tr("Open a disk image first."));
        return;
    }

    QStringList failures;
    int copied = 0;

    for (const QString &path : paths) {
        // Ignore leftovers from our own drag-out staging folder.
        if (!m_tempDir.isEmpty() && path.startsWith(m_tempDir))
            continue;

        const QFileInfo info(path);
        if (!info.isFile())
            continue;

        const cpm::Result result =
            m_backend->insert(image, format, path, info.fileName());
        if (result.failed())
            failures << QStringLiteral("%1: %2").arg(info.fileName(), result.error());
        else
            ++copied;
    }

    if (!failures.isEmpty())
        QMessageBox::warning(this, tr("Some files could not be copied"),
                             failures.join(QLatin1Char('\n')));

    if (copied > 0)
        refreshDirectory();
}

//---------------------------------------------------------------------------
void MainWindow::startDragOut()
{
    const QString image  = currentImage();
    const QString format = currentFormat();

    const QList<QListWidgetItem *> selected = m_entryList->selectedItems();
    if (image.isEmpty() || format.isEmpty() || selected.isEmpty())
        return;

    cleanupTempDir();

    // Stage the selected entries in a private temp folder.
    m_tempDir = QDir::temp().filePath(
        QStringLiteral("cpmtools-%1-%2")
            .arg(QCoreApplication::applicationPid())
            .arg(QDateTime::currentMSecsSinceEpoch()));

    if (!QDir().mkpath(m_tempDir))
        return;

    QList<QUrl> urls;
    for (auto *item : selected) {
        const QString name = item->data(Qt::UserRole).toString();
        const QString dest = QDir(m_tempDir).filePath(name);
        if (m_backend->extract(image, format, name, dest).ok())
            urls << QUrl::fromLocalFile(dest);
    }

    if (urls.isEmpty()) {
        cleanupTempDir();
        return;
    }

    auto *mimeData = new QMimeData;
    mimeData->setUrls(urls);

    auto *drag = new QDrag(m_entryList);
    drag->setMimeData(mimeData);
    drag->exec(Qt::CopyAction);

    // The drop target is still reading the files, so delete them later.
    m_pendingTempDir = m_tempDir;
    m_tempDir.clear();
    m_cleanupRetries = 0;
    QTimer::singleShot(1500, this, &MainWindow::cleanupTempDir);
}

//---------------------------------------------------------------------------
void MainWindow::cleanupTempDir()
{
    if (m_pendingTempDir.isEmpty())
        return;

    QDir dir(m_pendingTempDir);
    if (dir.exists() && !dir.removeRecursively()) {
        if (++m_cleanupRetries < 20) {
            QTimer::singleShot(1000, this, &MainWindow::cleanupTempDir);
            return;
        }
    }
    m_pendingTempDir.clear();
}

//---------------------------------------------------------------------------
void MainWindow::updateActions()
{
    const bool hasImage = !currentImage().isEmpty() && !currentFormat().isEmpty();
    const bool hasEntries = m_entryList->selectedItems().size() > 0;
    const bool hasHostFiles = !selectedHostFiles().isEmpty();

    m_getAction->setEnabled(hasImage && hasEntries);
    m_deleteAction->setEnabled(hasImage && hasEntries);
    m_putAction->setEnabled(hasImage && hasHostFiles);
}

//---------------------------------------------------------------------------
void MainWindow::showMkfs()
{
    MkfsDialog dialog(m_backend, this);
    connect(&dialog, &MkfsDialog::imageCreated, this, [this](const QString &image, const QString &) {
        openImage(image);
    });
    dialog.exec();
}

//---------------------------------------------------------------------------
void MainWindow::showAbout()
{
    AboutDialog dialog(this);
    dialog.exec();
}

//---------------------------------------------------------------------------
void MainWindow::openOnlineHelp()
{
    AboutDialog::openProjectPage();
}

//---------------------------------------------------------------------------
QString MainWindow::currentImage() const
{
    return m_imageEdit ? m_imageEdit->text() : QString();
}

//---------------------------------------------------------------------------
QString MainWindow::currentFormat() const
{
    if (m_formatList == nullptr || m_formatList->currentItem() == nullptr)
        return QString();
    if (!(m_formatList->currentItem()->flags() & Qt::ItemIsSelectable))
        return QString();
    return m_formatList->currentItem()->text();
}

//---------------------------------------------------------------------------
QString MainWindow::currentPattern() const
{
    return m_patternCombo ? m_patternCombo->currentText() : QStringLiteral("*.*");
}

//---------------------------------------------------------------------------
// Files chosen on the host side. The folder tree takes precedence when it holds
// a file selection, otherwise the file list is used. Directories are ignored -
// only real files can be put into an image.
QStringList MainWindow::selectedHostFiles() const
{
    QStringList files;

    const auto collect = [this, &files](QAbstractItemView *view) {
        if (view == nullptr || view->selectionModel() == nullptr)
            return;
        const QModelIndexList selected = view->selectionModel()->selectedIndexes();
        for (const QModelIndex &index : selected) {
            if (index.column() != 0)
                continue;
            const QString path = m_fsModel->filePath(index);
            if (path.isEmpty() || QFileInfo(path).isDir())
                continue;
            if (!files.contains(path))
                files << path;
        }
    };

    collect(m_dirTree);
    if (files.isEmpty())
        collect(m_hostFiles);

    return files;
}

//---------------------------------------------------------------------------
QString MainWindow::hostDirectory() const
{
    if (!m_hostFolder.isEmpty() && QFileInfo(m_hostFolder).isDir())
        return m_hostFolder;

    const QModelIndex index = m_hostFiles->rootIndex();
    if (index.isValid()) {
        const QString path = m_fsModel->filePath(index);
        if (QFileInfo(path).isDir())
            return path;
    }

    const QString image = currentImage();
    if (!image.isEmpty())
        return QFileInfo(image).absolutePath();
    return QDir::homePath();
}

//---------------------------------------------------------------------------
void MainWindow::setHostFolder(const QString &path)
{
    const QFileInfo info(path);
    if (!info.isDir())
        return;

    const QString folder = info.absoluteFilePath();

    // Show the folder's contents and point the tree at the same place.
    m_hostFiles->setRootIndex(m_fsModel->index(folder));
    m_dirTree->setCurrentIndex(m_fsModel->index(folder));

    m_hostFolder = folder;
    m_hostFolderPinned = true;

    if (m_hostPathEdit != nullptr && m_hostPathEdit->text() != QDir::toNativeSeparators(folder))
        m_hostPathEdit->setText(QDir::toNativeSeparators(folder));

    updateActions();
}

//---------------------------------------------------------------------------
void MainWindow::revealHostFile(const QString &filePath)
{
    const QModelIndex index = m_fsModel->index(filePath);
    if (!index.isValid())
        return;

    m_hostFiles->setCurrentIndex(index);
    m_hostFiles->scrollTo(index);
    m_hostFiles->setFocus();
    updateActions();
}

//---------------------------------------------------------------------------
void MainWindow::applyHostPath(const QString &rawPath)
{
    QString path = rawPath.trimmed();
    if (path.isEmpty())
        return;

    // Accept Windows paths as typed or pasted, and tolerate quotes left over
    // from copying a path out of a terminal.
    if ((path.startsWith(QLatin1Char('"')) && path.endsWith(QLatin1Char('"'))) ||
        (path.startsWith(QLatin1Char('\'')) && path.endsWith(QLatin1Char('\'')))) {
        path = path.mid(1, path.size() - 2).trimmed();
    }

    // Expand a leading "~" and normalise separators before inspecting it.
    if (path.startsWith(QLatin1Char('~')))
        path = QDir::homePath() + path.mid(1);

    const QFileInfo info(path);

    if (!info.exists()) {
        QMessageBox::warning(
            this, tr("Folder not found"),
            tr("There is no file or folder at:\n%1")
                .arg(QDir::toNativeSeparators(path)));
        return;
    }

    if (info.isDir()) {
        // A folder: show its contents.
        setHostFolder(info.absoluteFilePath());
        statusBar()->showMessage(
            tr("Showing %1").arg(QDir::toNativeSeparators(info.absoluteFilePath())),
            4000);
        return;
    }

    // A file: show its folder and highlight the file, so the neighbouring
    // files are visible and it can be put into the image straight away.
    setHostFolder(info.absolutePath());
    revealHostFile(info.absoluteFilePath());
    statusBar()->showMessage(
        tr("Showing %1 (selected %2)")
            .arg(QDir::toNativeSeparators(info.absolutePath()), info.fileName()),
        4000);
}

//---------------------------------------------------------------------------
void MainWindow::hostPathEdited()
{
    if (m_hostPathEdit == nullptr)
        return;
    applyHostPath(m_hostPathEdit->text());
}

//---------------------------------------------------------------------------
void MainWindow::hostPathDropped(const QString &path)
{
    applyHostPath(path);
}

//---------------------------------------------------------------------------
void MainWindow::chooseHostFolder()
{
    // A QFileDialog cannot select files and folders in one pass, so offer both
    // and use whichever the user picks.
    const QString start = hostDirectory();

    QMessageBox chooser(this);
    chooser.setWindowTitle(tr("Browse this computer"));
    chooser.setText(tr("What would you like to open?"));
    QPushButton *folderButton = chooser.addButton(tr("Folder..."), QMessageBox::AcceptRole);
    QPushButton *fileButton   = chooser.addButton(tr("File..."), QMessageBox::ActionRole);
    chooser.addButton(QMessageBox::Cancel);
    chooser.exec();

    if (chooser.clickedButton() == folderButton) {
        const QString folder = QFileDialog::getExistingDirectory(
            this, tr("Choose a folder"), start,
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (!folder.isEmpty())
            setHostFolder(folder);
        return;
    }

    if (chooser.clickedButton() == fileButton) {
        const QString file = QFileDialog::getOpenFileName(
            this, tr("Choose a file"), start, tr("All files (*)"));
        if (!file.isEmpty())
            applyHostPath(file);
    }
}

//---------------------------------------------------------------------------
void MainWindow::reportError(const QString &title, const cpm::Result &result)
{
    QMessageBox::critical(this, title, result.error());
}

//---------------------------------------------------------------------------
void MainWindow::closeEvent(QCloseEvent *event)
{
    cleanupTempDir();
    QMainWindow::closeEvent(event);
}
