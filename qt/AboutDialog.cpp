//---------------------------------------------------------------------------
#include "AboutDialog.h"

#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QFile>
#include <QFontDatabase>
#include <QTabWidget>
#include <QTextBrowser>
#include <QTextStream>
#include <QUrl>
#include <QVBoxLayout>

// Copyright (C) 2026 Joseph Kwok (@MustardBun)
// SPDX-License-Identifier: GPL-3.0-or-later

const char *AboutDialog::kProjectUrl =
    "https://github.com/MustardBun";
const char *AboutDialog::kVersion = "v0.1-preview";

//---------------------------------------------------------------------------
AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("About CPMToolsQt6"));
    setModal(true);
    resize(680, 580);

    m_aboutView = new QTextBrowser(this);
    m_aboutView->setOpenExternalLinks(true);
    m_aboutView->setHtml(aboutHtml());

    m_licenseView = new QTextBrowser(this);
    m_licenseView->setOpenExternalLinks(false);
    m_licenseView->setLineWrapMode(QTextEdit::NoWrap);
    m_licenseView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    m_licenseView->setPlainText(licenseText());

    auto *tabs = new QTabWidget(this);
    tabs->addTab(m_aboutView, tr("About"));
    tabs->addTab(m_licenseView, tr("License"));

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(tabs, 1);
    layout->addWidget(buttons);
}

//---------------------------------------------------------------------------
QString AboutDialog::aboutHtml() const
{
    const QString projectUrl = QString::fromLatin1(kProjectUrl).toHtmlEscaped();
    const QString version    = QString::fromLatin1(kVersion).toHtmlEscaped();

    return QStringLiteral(R"HTML(
<html><body style="font-family: sans-serif;">

<h2>CPMToolsQt6</h2>
<p><b>%1</b></p>

<p>
  Browse, create and edit CP/M disk images, including Z80-MBC2 CP/M&nbsp;3
  volumes. Qt&nbsp;6 front end for the portable <i>cpmtools</i> engine.
</p>

<h3>Author</h3>
<p>
  Joseph Kwok (@MustardBun)<br/>
  <a href="%2">%2</a>
</p>

<h3>License</h3>
<p>
  <b>GNU General Public License v3.0 (GPLv3)</b><br/>
  This program is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.<br/>
  See the <i>License</i> tab for the full text.
</p>

<h3>Credits and attributions</h3>
<table cellpadding="4" cellspacing="0" width="100%">
  <tr>
    <td valign="top" width="34%"><b>Original CP/M tools GUI</b><br/>
        neko Java</td>
    <td valign="top">CPMToolsQt6 is a port of <i>CPMToolsGUI</i> by neko Java,
        including the <tt>diskdefs</tt> format definitions and the
        MITS Altair 88-DISK support.</td>
  </tr>
  <tr>
    <td valign="top"><b>cpmtools engine</b><br/>
        &copy; Michael Haardt</td>
    <td valign="top">GPL. The portable C library that reads and writes the
        CP/M filesystem: <tt>cpmfs.c</tt>, <tt>cpmcp.c</tt>,
        <tt>mkfs.cpm.c</tt>, <tt>device_posix.c</tt>.</td>
  </tr>
  <tr>
    <td valign="top"><b>Z80-MBC2 disk definitions</b><br/>
        Just4Fun (Fabio Defabis)</td>
    <td valign="top">Disk geometries for the Z80-MBC2 and Z80-MBC2-CPM3
        single-board computers (<tt>z80mbc2-d0</tt>, <tt>z80mbc2-d1</tt>,
        <tt>z80mbc2-cpm3</tt>).</td>
  </tr>
  <tr>
    <td valign="top"><b>Qt6 port</b><br/>
        &copy; 2026 Joseph Kwok (@MustardBun)</td>
    <td valign="top">The Qt&nbsp;6 user interface, build system, packaging and
        internationalisation. <b>Developed with AI assistance.</b></td>
  </tr>
</table>

<hr/>
<p style="font-size: small; color: gray;">
  Pre-release preview build. Built with Qt %3.
  CP/M is a trademark of Digital Research / DRDOS, Inc.
  This project is not affiliated with or endorsed by any of the parties above.
</p>

</body></html>
)HTML").arg(version,
             projectUrl,
             QString::fromLatin1(qVersion()).toHtmlEscaped());
}

//---------------------------------------------------------------------------
QString AboutDialog::licenseText()
{
    // LICENSE is embedded at the resource root by qt_add_resources() in
    // CMakeLists.txt, so it is always available at run time.
    QFile file(QStringLiteral(":/LICENSE"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QObject::tr("The licence text could not be loaded from the "
                           "embedded resources.");

    QTextStream stream(&file);
    return stream.readAll();
}

//---------------------------------------------------------------------------
void AboutDialog::openProjectPage()
{
    QDesktopServices::openUrl(QUrl(QString::fromLatin1(kProjectUrl)));
}
