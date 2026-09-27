//---------------------------------------------------------------------------
// MkfsDialog - the "new CP/M image" dialog.
//
// Copyright (C) 2026 Joseph Kwok (@MustardBun)
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Derived from the original CpmtoolsGUI by neko Java / Koji Suzuki.
//---------------------------------------------------------------------------
#ifndef MKFSDIALOG_H
#define MKFSDIALOG_H

#include <QDialog>
#include <QString>

#include "CpmBackend.h"

class QCheckBox;
class QLineEdit;
class QListWidget;
class QPushButton;

class MkfsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MkfsDialog(cpm::Backend *backend, QWidget *parent = nullptr);

    QString imagePath() const;
    QString format() const;

signals:
    void imageCreated(const QString &image, const QString &format);

private slots:
    void chooseImage();
    void chooseBootFile();
    void refreshFormats();
    void makeImage();

private:
    QWidget *createBootRow(int index, QWidget *parent);

    cpm::Backend *m_backend = nullptr;

    QLineEdit   *m_imageEdit  = nullptr;
    QListWidget *m_formatList = nullptr;
    QLineEdit   *m_bootEdits[4] = { nullptr, nullptr, nullptr, nullptr };
    QCheckBox   *m_fullSizeCheck = nullptr;
    QCheckBox   *m_bootSkewCheck = nullptr;
};

#endif // MKFSDIALOG_H
