//---------------------------------------------------------------------------
// CpmBackend.cpp - Qt wrapper around the portable C engine.
//---------------------------------------------------------------------------
#include "CpmBackend.h"

// Copyright (C) 2026 Joseph Kwok (@MustardBun)
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

#include <cstdlib>
#include <cstring>

// Not exposed through cpm_test.h; declared here so we can free the results of
// cpmglob() correctly (each name is a separate malloc).
extern "C" void cpmglobfree(char **dirent, int entries);

namespace cpm {

namespace {

//! Copy a QString into a fixed-size engine buffer.
void copyToBuffer(char *buffer, size_t size, const QString &value)
{
    const QByteArray bytes = value.toLocal8Bit();
    const size_t n = static_cast<size_t>(bytes.size());
    const size_t copy = (n < size - 1) ? n : size - 1;
    if (copy > 0)
        std::memcpy(buffer, bytes.constData(), copy);
    buffer[copy] = '\0';
}

//! Free a (char**, count) pair produced by the engine.
void freeStringArray(char **items, int count)
{
    if (items == nullptr)
        return;
    for (int i = 0; i < count; ++i)
        std::free(items[i]);
    std::free(items);
}

} // namespace

//---------------------------------------------------------------------------
Backend::Backend()
{
    setDefPath(installedDefPath());
    cur_defpath[0] = '\0';
    mits_137_sect_flg = 0;
}

//---------------------------------------------------------------------------
QString Backend::installedDefPath()
{
    return QDir(QCoreApplication::applicationDirPath())
               .filePath(QStringLiteral("diskdefs"));
}

//---------------------------------------------------------------------------
void Backend::setDefPath(const QString &path)
{
    copyToBuffer(defpath, sizeof(defpath), path);
}

QString Backend::defPath() const
{
    return QString::fromLocal8Bit(defpath);
}

//---------------------------------------------------------------------------
void Backend::setCurrentDefPath(const QString &path)
{
    copyToBuffer(cur_defpath, sizeof(cur_defpath), path);
}

QString Backend::currentDefPath() const
{
    return QString::fromLocal8Bit(cur_defpath);
}

//---------------------------------------------------------------------------
void Backend::useDiskdefsFrom(const QString &imagePath)
{
    const QFileInfo info(imagePath);
    setCurrentDefPath(info.absoluteDir().filePath(QStringLiteral("diskdefs")));
}

//---------------------------------------------------------------------------
void Backend::setFullSize(bool on) { FullSizeFlg = on ? 1 : 0; }
void Backend::setBootSkew(bool on) { BootSkewFlg = on ? 1 : 0; }
bool Backend::fullSize() const     { return FullSizeFlg != 0; }
bool Backend::bootSkew() const     { return BootSkewFlg != 0; }

//---------------------------------------------------------------------------
Result Backend::listFormats(QStringList &out) const
{
    out.clear();

    int    count = 0;
    char **names = nullptr;

    const char *err = cpm_fmt_test(&count, &names);
    if (err != nullptr) {
        freeStringArray(names, count);
        return Result(err);
    }

    for (int i = 0; i < count; ++i)
        out << QString::fromLocal8Bit(names[i]);

    freeStringArray(names, count);
    return Result();
}

//---------------------------------------------------------------------------
Result Backend::listDirectory(const QString &image, const QString &format,
                              const QString &pattern, QList<Entry> &out) const
{
    out.clear();

    QByteArray aImage   = image.toLocal8Bit();
    QByteArray aFormat  = format.toLocal8Bit();
    QByteArray aPattern = pattern.isEmpty() ? QByteArrayLiteral("*.*")
                                            : pattern.toLocal8Bit();

    int    count = 0;
    char **names = nullptr;

    const char *err = cpm_ls_test(aImage.data(), aFormat.data(),
                                  aPattern.data(), &count, &names);
    if (err != nullptr) {
        // On failure cpm_ls_test leaves the outputs untouched.
        return Result(err);
    }

    for (int i = 0; i < count; ++i) {
        const char *raw = names[i];
        if (raw == nullptr)
            continue;

        Entry entry;
        const int len = static_cast<int>(std::strlen(raw));
        if (len > 2) {
            bool numeric = false;
            const int user = QString::fromLatin1(raw, 2).toInt(&numeric);
            entry.user = numeric ? user : 0;
            entry.name = QString::fromLocal8Bit(raw + 2);
        } else {
            entry.name = QString::fromLocal8Bit(raw);
        }
        if (!entry.name.isEmpty())
            out.append(entry);
    }

    if (count > 0)
        cpmglobfree(names, count);

    return Result();
}

//---------------------------------------------------------------------------
Result Backend::extract(const QString &image, const QString &format,
                        const QString &entryName, const QString &destination) const
{
    QByteArray aImage  = image.toLocal8Bit();
    QByteArray aFormat = format.toLocal8Bit();
    QByteArray aSrc    = entryName.toLocal8Bit();
    QByteArray aDest   = destination.toLocal8Bit();

    return Result(cpm_to_win_test(aImage.data(), aFormat.data(),
                                  aSrc.data(), aDest.data()));
}

//---------------------------------------------------------------------------
Result Backend::insert(const QString &image, const QString &format,
                       const QString &sourcePath, const QString &entryName) const
{
    QByteArray aImage  = image.toLocal8Bit();
    QByteArray aFormat = format.toLocal8Bit();
    QByteArray aSrc    = sourcePath.toLocal8Bit();
    QByteArray aDest   = entryName.toLocal8Bit();

    return Result(win_to_cpm_test(aImage.data(), aFormat.data(),
                                  aSrc.data(), aDest.data()));
}

//---------------------------------------------------------------------------
Result Backend::remove(const QString &image, const QString &format,
                       const QString &entryName) const
{
    QByteArray aImage  = image.toLocal8Bit();
    QByteArray aFormat = format.toLocal8Bit();
    QByteArray aName   = entryName.toLocal8Bit();

    return Result(cpm_rm_test(aImage.data(), aFormat.data(), aName.data()));
}

//---------------------------------------------------------------------------
Result Backend::createImage(const QString &image, const QString &format,
                            const QStringList &bootFiles) const
{
    QByteArray aImage  = image.toLocal8Bit();
    QByteArray aFormat = format.toLocal8Bit();

    // Keep the buffers alive and build the NULL-terminated char*[5].
    QList<QByteArray> buffers;
    for (int i = 0; i < 4; ++i) {
        const QString file = (i < bootFiles.size()) ? bootFiles.at(i) : QString();
        buffers.append(file.isEmpty() ? QByteArray() : file.toLocal8Bit());
    }

    char *boot[5];
    for (int i = 0; i < 4; ++i)
        boot[i] = buffers.at(i).isEmpty() ? nullptr : buffers[i].data();
    boot[4] = nullptr;

    return Result(creat_cpm_test(aImage.data(), aFormat.data(), boot));
}

} // namespace cpm
