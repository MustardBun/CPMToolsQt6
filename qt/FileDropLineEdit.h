//---------------------------------------------------------------------------
// FileDropLineEdit - a QLineEdit that accepts a single file dropped onto it
// (replaces the VCL DragAcceptFiles + WM_DROPFILES handling).
//
// Copyright (C) 2026 Joseph Kwok (@MustardBun)
// SPDX-License-Identifier: GPL-3.0-or-later
//---------------------------------------------------------------------------
#ifndef FILEDROPLINEEDIT_H
#define FILEDROPLINEEDIT_H

#include <QLineEdit>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>

class FileDropLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit FileDropLineEdit(QWidget *parent = nullptr)
        : QLineEdit(parent)
    {
        setAcceptDrops(true);
        setClearButtonEnabled(true);
    }

signals:
    void fileDropped(const QString &path);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override
    {
        if (event->mimeData()->hasUrls() && event->mimeData()->urls().size() == 1)
            event->acceptProposedAction();
    }

    void dragMoveEvent(QDragMoveEvent *event) override
    {
        if (event->mimeData()->hasUrls())
            event->acceptProposedAction();
    }

    void dropEvent(QDropEvent *event) override
    {
        const QList<QUrl> urls = event->mimeData()->urls();
        if (urls.isEmpty())
            return;

        const QString path = urls.constFirst().toLocalFile();
        if (!path.isEmpty()) {
            event->acceptProposedAction();
            emit fileDropped(path);
        }
    }
};

#endif // FILEDROPLINEEDIT_H
