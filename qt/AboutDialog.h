//---------------------------------------------------------------------------
// AboutDialog - program information, attributions and the full GPLv3 notice.
//
// Copyright (C) 2026 Joseph Kwok (@MustardBun)
// SPDX-License-Identifier: GPL-3.0-or-later
//---------------------------------------------------------------------------
#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QString>

class QTextBrowser;

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);

    //! Open the project's wiki page in the default browser.
    static void openProjectPage();

private:
    //! HTML for the About tab, including all upstream attributions.
    QString aboutHtml() const;

    //! Full GPLv3 text, read from the embedded :/LICENSE resource.
    static QString licenseText();

    static const char *kProjectUrl;
    static const char *kVersion;

    QTextBrowser *m_aboutView   = nullptr;
    QTextBrowser *m_licenseView = nullptr;
};

#endif // ABOUTDIALOG_H
