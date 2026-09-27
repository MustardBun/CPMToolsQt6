//---------------------------------------------------------------------------
#include "MkfsDialog.h"

// Copyright (C) 2026 Joseph Kwok (@MustardBun)
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>

//---------------------------------------------------------------------------
MkfsDialog::MkfsDialog(cpm::Backend *backend, QWidget *parent)
    : QDialog(parent)
    , m_backend(backend)
{
    setWindowTitle(tr("New CP/M image"));
    setModal(true);
    setMinimumSize(600, 420);

    // ------------------------------------------------------------- image row
    m_imageEdit = new QLineEdit(this);
    m_imageEdit->setReadOnly(true);
    m_imageEdit->setClearButtonEnabled(true);

    auto *selectImage = new QPushButton(tr("Select..."), this);
    connect(selectImage, &QPushButton::clicked, this, &MkfsDialog::chooseImage);

    auto *imageRow = new QHBoxLayout;
    imageRow->addWidget(m_imageEdit, 1);
    imageRow->addWidget(selectImage);

    auto *imageGroup = new QGroupBox(tr("Image file"), this);
    imageGroup->setLayout(imageRow);

    // -------------------------------------------------------------- formats
    m_formatList = new QListWidget(this);
    m_formatList->setAlternatingRowColors(true);

    auto *formatGroup = new QGroupBox(tr("Format"), this);
    auto *formatLayout = new QVBoxLayout(formatGroup);
    formatLayout->addWidget(m_formatList);

    // ----------------------------------------------------------- boot block
    auto *bootGroup = new QGroupBox(tr("Boot block (IPL/CCP/BDOS/BIOS)"), this);
    auto *bootLayout = new QGridLayout(bootGroup);
    for (int i = 0; i < 4; ++i) {
        bootLayout->addWidget(new QLabel(tr("File %1").arg(i + 1), bootGroup), i, 0);
        bootLayout->addWidget(createBootRow(i, bootGroup), i, 1);
    }
    bootLayout->setColumnStretch(1, 1);

    // ------------------------------------------------------------- options
    m_fullSizeCheck = new QCheckBox(tr("Whole capacity size"), this);
    m_bootSkewCheck = new QCheckBox(tr("Skew in boot image"), this);
    m_fullSizeCheck->setChecked(m_backend ? m_backend->fullSize() : true);
    m_bootSkewCheck->setChecked(m_backend ? m_backend->bootSkew() : true);

    connect(m_fullSizeCheck, &QCheckBox::toggled, this, [this](bool on) {
        if (m_backend) m_backend->setFullSize(on);
    });
    connect(m_bootSkewCheck, &QCheckBox::toggled, this, [this](bool on) {
        if (m_backend) m_backend->setBootSkew(on);
    });

    // -------------------------------------------------------------- actions
    auto *makeButton = new QPushButton(tr("Make"), this);
    makeButton->setDefault(true);
    connect(makeButton, &QPushButton::clicked, this, &MkfsDialog::makeImage);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    buttonBox->addButton(makeButton, QDialogButtonBox::AcceptRole);

    // --------------------------------------------------------------- layout
    auto *middle = new QHBoxLayout;
    middle->addWidget(formatGroup, 2);
    middle->addWidget(bootGroup, 3);

    auto *optionsRow = new QHBoxLayout;
    optionsRow->addWidget(m_fullSizeCheck);
    optionsRow->addWidget(m_bootSkewCheck);
    optionsRow->addStretch(1);

    auto *main = new QVBoxLayout(this);
    main->addWidget(imageGroup);
    main->addLayout(middle, 1);
    main->addLayout(optionsRow);
    main->addWidget(buttonBox);

    refreshFormats();
}

//---------------------------------------------------------------------------
QWidget *MkfsDialog::createBootRow(int index, QWidget *parent)
{
    auto *edit = new QLineEdit(parent);
    edit->setReadOnly(true);
    m_bootEdits[index] = edit;

    auto *browse = new QPushButton(tr("..."), parent);
    browse->setFixedWidth(32);
    browse->setToolTip(tr("Choose boot block file %1").arg(index + 1));
    browse->setProperty("bootIndex", index);
    connect(browse, &QPushButton::clicked, this, &MkfsDialog::chooseBootFile);

    auto *holder = new QWidget(parent);
    auto *row = new QHBoxLayout(holder);
    row->setContentsMargins(0, 0, 0, 0);
    row->addWidget(edit, 1);
    row->addWidget(browse);
    return holder;
}

//---------------------------------------------------------------------------
void MkfsDialog::chooseImage()
{
    const QString path = QFileDialog::getSaveFileName(
        this, tr("Create CP/M image"), m_imageEdit->text(),
        tr("Disk images (*.dsk *.ddi *.img *.bin);;All files (*)"));
    if (path.isEmpty())
        return;

    m_imageEdit->setText(path);

    // Formats may come from a diskdefs sitting next to the image.
    if (m_backend)
        m_backend->useDiskdefsFrom(path);
    refreshFormats();
}

//---------------------------------------------------------------------------
void MkfsDialog::chooseBootFile()
{
    auto *button = qobject_cast<QPushButton *>(sender());
    if (button == nullptr)
        return;

    const int index = button->property("bootIndex").toInt();
    if (index < 0 || index > 3)
        return;

    const QString path = QFileDialog::getOpenFileName(
        this, tr("Select boot block file %1").arg(index + 1), QString(),
        tr("All files (*)"));
    if (!path.isEmpty())
        m_bootEdits[index]->setText(path);
}

//---------------------------------------------------------------------------
void MkfsDialog::refreshFormats()
{
    if (m_backend == nullptr)
        return;

    const QString previous = format();

    QStringList formats;
    const cpm::Result result = m_backend->listFormats(formats);

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
        if (!matches.isEmpty())
            m_formatList->setCurrentItem(matches.constFirst());
    }
    if (m_formatList->currentRow() < 0 && m_formatList->count() > 0)
        m_formatList->setCurrentRow(0);
}

//---------------------------------------------------------------------------
QString MkfsDialog::imagePath() const
{
    return m_imageEdit ? m_imageEdit->text() : QString();
}

//---------------------------------------------------------------------------
QString MkfsDialog::format() const
{
    if (m_formatList == nullptr || m_formatList->currentItem() == nullptr)
        return QString();
    return m_formatList->currentItem()->text();
}

//---------------------------------------------------------------------------
void MkfsDialog::makeImage()
{
    const QString image  = imagePath();
    const QString fmt    = format();

    if (image.isEmpty()) {
        QMessageBox::warning(this, tr("New CP/M image"),
                             tr("Please choose an image file to create."));
        return;
    }
    if (fmt.isEmpty()) {
        QMessageBox::warning(this, tr("New CP/M image"),
                             tr("Please select a format."));
        return;
    }

    QStringList bootFiles;
    for (auto *edit : m_bootEdits)
        bootFiles << (edit ? edit->text() : QString());

    if (m_backend)
        m_backend->useDiskdefsFrom(image);

    const cpm::Result result = m_backend
        ? m_backend->createImage(image, fmt, bootFiles)
        : cpm::Result(QStringLiteral("backend unavailable"));

    if (result.failed()) {
        QMessageBox::critical(this, tr("Could not create image"), result.error());
        return;
    }

    emit imageCreated(image, fmt);
    QMessageBox::information(this, tr("New CP/M image"), tr("Completed!"));
    accept();
}
