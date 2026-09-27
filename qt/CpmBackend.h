//---------------------------------------------------------------------------
// CpmBackend - thin, Qt-friendly wrapper around the portable C engine
//   cpm_ls_test / cpm_fmt_test / cpm_to_win_test / win_to_cpm_test /
//   cpm_rm_test / creat_cpm_test
// The engine reports errors by returning a const char* (NULL == success).//
// Copyright (C) 2026 Joseph Kwok (@MustardBun)
// SPDX-License-Identifier: GPL-3.0-or-later//---------------------------------------------------------------------------
#ifndef CPMBACKEND_H
#define CPMBACKEND_H

#include <QString>
#include <QStringList>

// --- core engine, C linkage -------------------------------------------------
extern "C" {
#include "cpm_test.h"

// Globals defined by the engine (cpmfs.c / mkfs.cpm.c). They must be
// referenced with C linkage or the linker will look for mangled names.
extern char defpath[261];
extern char cur_defpath[261];
extern char mits_137_sect_flg;
extern char FullSizeFlg;
extern char BootSkewFlg;
}

namespace cpm {

//! Simple ok/error result; the engine returns NULL on success.
class Result
{
public:
    Result() : m_ok(true) {}
    explicit Result(const QString &error) : m_ok(false), m_error(error) {}
    //! Build from the engine's return value.
    explicit Result(const char *engineError)
        : m_ok(engineError == nullptr)
        , m_error(engineError ? QString::fromLocal8Bit(engineError) : QString()) {}

    bool ok() const { return m_ok; }
    bool failed() const { return !m_ok; }
    QString error() const { return m_error; }
    explicit operator bool() const { return m_ok; }

private:
    bool    m_ok = true;
    QString m_error;
};

//! One file inside a CP/M image.
struct Entry
{
    int     user = 0;   //!< CP/M user number (0-15)
    QString name;       //!< e.g. "FOO.COM", without the "00" user prefix
};

class Backend
{
public:
    Backend();

    //! Directory definitions ("diskdefs") resolved next to the executable.
    static QString installedDefPath();

    //! Location the engine reads the format list from (defpath / cur_defpath).
    void    setDefPath(const QString &path);
    QString defPath() const;

    //! Per-image override; the engine tries cur_defpath before defpath.
    void    setCurrentDefPath(const QString &path);
    QString currentDefPath() const;

    //! Point the engine at a diskdefs sitting beside the given image file.
    void useDiskdefsFrom(const QString &imagePath);

    //! mkfs options backed by the engine's FullSizeFlg / BootSkewFlg.
    void setFullSize(bool on);
    void setBootSkew(bool on);
    bool fullSize() const;
    bool bootSkew() const;

    //! Format names found in diskdefs.
    Result listFormats(QStringList &out) const;

    //! Directory listing of a CP/M image, filtered by a CP/M wildcard pattern.
    Result listDirectory(const QString &image, const QString &format,
                         const QString &pattern, QList<Entry> &out) const;

    //! image -> host file
    Result extract(const QString &image, const QString &format,
                   const QString &entryName, const QString &destination) const;

    //! host file -> image
    Result insert(const QString &image, const QString &format,
                  const QString &sourcePath, const QString &entryName) const;

    //! Delete a file from the image.
    Result remove(const QString &image, const QString &format,
                  const QString &entryName) const;

    //! Format a new image. \a bootFiles holds up to four boot-block images.
    Result createImage(const QString &image, const QString &format,
                       const QStringList &bootFiles) const;
};

} // namespace cpm

#endif // CPMBACKEND_H
