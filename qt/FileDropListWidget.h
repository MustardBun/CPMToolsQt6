//---------------------------------------------------------------------------
// FileDropListWidget - a QListWidget for the CP/M directory listing.
//   * accepts files dropped from Explorer / the file manager
//   * reports drag-out attempts so the window can start a real QDrag
//
// Copyright (C) 2026 Joseph Kwok (@MustardBun)
// SPDX-License-Identifier: GPL-3.0-or-later
//---------------------------------------------------------------------------
#ifndef FILEDROPLISTWIDGET_H
#define FILEDROPLISTWIDGET_H

#include <QListWidget>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>

class FileDropListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit FileDropListWidget(QWidget *parent = nullptr)
        : QListWidget(parent)
    {
        setAcceptDrops(true);
        setDragEnabled(true);
        setDropIndicatorShown(true);
        setSelectionMode(QAbstractItemView::ExtendedSelection);
        setDragDropMode(QAbstractItemView::DragDrop);
        setDefaultDropAction(Qt::CopyAction);

        // One entry per row: small icon on the left, name filling the rest of
        // the width. The name elides at the right edge when the pane is narrow
        // and gets progressively more room as the user widens it - no fixed
        // grid, so no mid-name clipping to a handful of characters.
        setViewMode(QListView::ListMode);
        setFlow(QListView::TopToBottom);
        setWrapping(false);
        setResizeMode(QListView::Adjust);
        setMovement(QListView::Static);
        setWordWrap(false);
        setUniformItemSizes(true);
        setTextElideMode(Qt::ElideRight);
        // Elide rather than scroll: a horizontal scrollbar would hide the very
        // overflow the user wants to see by resizing the pane.
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setAlternatingRowColors(true);
        setIconSize(QSize(16, 16));
    }

signals:
    void filesDropped(const QStringList &paths);
    void dragOutRequested();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override
    {
        if (event->mimeData()->hasUrls())
            event->acceptProposedAction();
    }

    void dragMoveEvent(QDragMoveEvent *event) override
    {
        if (event->mimeData()->hasUrls())
            event->acceptProposedAction();
    }

    void dropEvent(QDropEvent *event) override
    {
        QStringList paths;
        const QList<QUrl> urls = event->mimeData()->urls();
        for (const QUrl &url : urls) {
            const QString path = url.toLocalFile();
            if (!path.isEmpty())
                paths << path;
        }
        if (!paths.isEmpty()) {
            event->acceptProposedAction();
            emit filesDropped(paths);
        }
    }

    void startDrag(Qt::DropActions /*supportedActions*/) override
    {
        // The owner window extracts the selected entries and performs the drag.
        emit dragOutRequested();
    }
};

#endif // FILEDROPLISTWIDGET_H
