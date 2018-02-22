/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwebenginedownloaditem.h"
#include "qwebenginedownloaditem_p.h"

#include "browser_context_adapter.h"
#include "qwebengineprofile_p.h"


QT_BEGIN_NAMESPACE

using QtWebEngineCore::BrowserContextAdapterClient;

ASSERT_ENUMS_MATCH(BrowserContextAdapterClient::NoReason, QWebEngineDownloadItem::NoReason)
ASSERT_ENUMS_MATCH(BrowserContextAdapterClient::FileFailed, QWebEngineDownloadItem::FileFailed)
ASSERT_ENUMS_MATCH(BrowserContextAdapterClient::FileAccessDenied, QWebEngineDownloadItem::FileAccessDenied)
ASSERT_ENUMS_MATCH(BrowserContextAdapterClient::FileNoSpace, QWebEngineDownloadItem::FileNoSpace)
ASSERT_ENUMS_MATCH(BrowserContextAdapterClient::FileNameTooLong, QWebEngineDownloadItem::FileNameTooLong)
ASSERT_ENUMS_MATCH(BrowserContextAdapterClient::FileTooLarge, QWebEngineDownloadItem::FileTooLarge)
ASSERT_ENUMS_MATCH(BrowserContextAdapterClient::FileVirusInfected, QWebEngineDownloadItem::FileVirusInfected)
ASSERT_ENUMS_MATCH(BrowserContextAdapterClient::FileTransientError, QWebEngineDownloadItem::FileTransientError)
ASSERT_ENUMS_MATCH(BrowserContextAdapterClient::FileBlocked, QWebEngineDownloadItem::FileBlocked)
ASSERT_ENUMS_MATCH(BrowserContextAdapterClient::FileSecurityCheckFailed, QWebEngineDownloadItem::FileSecurityCheckFailed)
ASSERT_ENUMS_MATCH(BrowserContextAdapterClient::FileTooShort, QWebEngineDownloadItem::FileTooShort)
ASSERT_ENUMS_MATCH(BrowserContextAdapterClient::FileHashMismatch, QWebEngineDownloadItem::FileHashMismatch)
ASSERT_ENUMS_MATCH(BrowserContextAdapterClient::NetworkFailed, QWebEngineDownloadItem::NetworkFailed)
ASSERT_ENUMS_MATCH(BrowserContextAdapterClient::NetworkTimeout, QWebEngineDownloadItem::NetworkTimeout)
ASSERT_ENUMS_MATCH(BrowserContextAdapterClient::NetworkDisconnected, QWebEngineDownloadItem::NetworkDisconnected)
ASSERT_ENUMS_MATCH(BrowserContextAdapterClient::NetworkServerDown, QWebEngineDownloadItem::NetworkServerDown)
ASSERT_ENUMS_MATCH(BrowserContextAdapterClient::NetworkInvalidRequest, QWebEngineDownloadItem::NetworkInvalidRequest)
ASSERT_ENUMS_MATCH(BrowserContextAdapterClient::ServerFailed, QWebEngineDownloadItem::ServerFailed)
//ASSERT_ENUMS_MATCH(BrowserContextAdapterClient::ServerNoRange, QWebEngineDownloadItem::ServerNoRange)
ASSERT_ENUMS_MATCH(BrowserContextAdapterClient::ServerBadContent, QWebEngineDownloadItem::ServerBadContent)
ASSERT_ENUMS_MATCH(BrowserContextAdapterClient::ServerUnauthorized, QWebEngineDownloadItem::ServerUnauthorized)
ASSERT_ENUMS_MATCH(BrowserContextAdapterClient::ServerCertProblem, QWebEngineDownloadItem::ServerCertProblem)
ASSERT_ENUMS_MATCH(BrowserContextAdapterClient::ServerForbidden, QWebEngineDownloadItem::ServerForbidden)
ASSERT_ENUMS_MATCH(BrowserContextAdapterClient::ServerUnreachable, QWebEngineDownloadItem::ServerUnreachable)
ASSERT_ENUMS_MATCH(BrowserContextAdapterClient::UserCanceled, QWebEngineDownloadItem::UserCanceled)
//ASSERT_ENUMS_MATCH(BrowserContextAdapterClient::UserShutdown, QWebEngineDownloadItem::UserShutdown)
//ASSERT_ENUMS_MATCH(BrowserContextAdapterClient::Crash, QWebEngineDownloadItem::Crash)

static inline QWebEngineDownloadItem::DownloadState toDownloadState(int state)
{
    switch (state) {
    case BrowserContextAdapterClient::DownloadInProgress:
        return QWebEngineDownloadItem::DownloadInProgress;
    case BrowserContextAdapterClient::DownloadCompleted:
        return QWebEngineDownloadItem::DownloadCompleted;
    case BrowserContextAdapterClient::DownloadCancelled:
        return QWebEngineDownloadItem::DownloadCancelled;
    case BrowserContextAdapterClient::DownloadInterrupted:
        return QWebEngineDownloadItem::DownloadInterrupted;
    default:
        Q_UNREACHABLE();
        return QWebEngineDownloadItem::DownloadCancelled;
    }
}

static inline QWebEngineDownloadItem::DownloadInterruptReason toDownloadInterruptReason(int reason)
{
    return static_cast<QWebEngineDownloadItem::DownloadInterruptReason>(reason);
}

/*!
    \class QWebEngineDownloadItem
    \brief The QWebEngineDownloadItem class provides information about a download.

    \since 5.5

    \inmodule QtWebEngineWidgets

    QWebEngineDownloadItem stores the state of a download to be used to manage requested downloads.
*/

QWebEngineDownloadItemPrivate::QWebEngineDownloadItemPrivate(QWebEngineProfilePrivate *p, const QUrl &url)
    : profile(p)
    , downloadFinished(false)
    , downloadId(-1)
    , downloadState(QWebEngineDownloadItem::DownloadCancelled)
    , savePageFormat(QWebEngineDownloadItem::MimeHtmlSaveFormat)
    , type(QWebEngineDownloadItem::Attachment)
    , interruptReason(QWebEngineDownloadItem::NoReason)
    , downloadUrl(url)
    , downloadPaused(false)
    , totalBytes(-1)
    , receivedBytes(0)
{
}

QWebEngineDownloadItemPrivate::~QWebEngineDownloadItemPrivate()
{
}

void QWebEngineDownloadItemPrivate::update(const BrowserContextAdapterClient::DownloadItemInfo &info)
{
    Q_Q(QWebEngineDownloadItem);

    Q_ASSERT(downloadState != QWebEngineDownloadItem::DownloadRequested);

    if (toDownloadInterruptReason(info.downloadInterruptReason) != interruptReason)
        interruptReason = toDownloadInterruptReason(info.downloadInterruptReason);

    if (toDownloadState(info.state) != downloadState) {
        downloadState = toDownloadState(info.state);
        Q_EMIT q->stateChanged(downloadState);
    }

    if (info.receivedBytes != receivedBytes || info.totalBytes != totalBytes) {
        receivedBytes = info.receivedBytes;
        totalBytes = info.totalBytes;
        Q_EMIT q->downloadProgress(receivedBytes, totalBytes);
    }

    if (info.done != downloadFinished) {
        downloadFinished = info.done;
        if (downloadFinished)
            Q_EMIT q->finished();
    }

    if (downloadPaused != info.paused) {
        downloadPaused = info.paused;
        Q_EMIT q->isPausedChanged(downloadPaused);
    }
}

/*!
    Accepts the current download request, which will start the download.

    \sa finished(), stateChanged()
*/

void QWebEngineDownloadItem::accept()
{
    Q_D(QWebEngineDownloadItem);

    if (d->downloadState != QWebEngineDownloadItem::DownloadRequested)
        return;

    d->downloadState = QWebEngineDownloadItem::DownloadInProgress;
    Q_EMIT stateChanged(d->downloadState);
}

/*!
    Cancels the current download.

    \sa finished(), stateChanged()
*/

void QWebEngineDownloadItem::cancel()
{
    Q_D(QWebEngineDownloadItem);

    QWebEngineDownloadItem::DownloadState state = d->downloadState;

    if (state == QWebEngineDownloadItem::DownloadCompleted
            || state == QWebEngineDownloadItem::DownloadCancelled)
        return;

    // We directly cancel the download request if the user cancels
    // before it even started, so no need to notify the profile here.
    if (state == QWebEngineDownloadItem::DownloadInProgress) {
        if (auto browserContext = d->profile->browserContext())
            browserContext->cancelDownload(d->downloadId);
    } else {
        d->downloadState = QWebEngineDownloadItem::DownloadCancelled;
        Q_EMIT stateChanged(d->downloadState);
    }
}

/*!
    \since 5.10
    Pauses the current download. Has no effect if the state is not \c DownloadInProgress.

    \sa resume()
*/

void QWebEngineDownloadItem::pause()
{
    Q_D(QWebEngineDownloadItem);

    QWebEngineDownloadItem::DownloadState state = d->downloadState;

    if (state != QWebEngineDownloadItem::DownloadInProgress)
        return;

    d->profile->browserContext()->pauseDownload(d->downloadId);
}

/*!
    \since 5.10
    Resumes the current download if it was paused or interrupted.

    \sa pause(), isPaused(), state()
*/
void QWebEngineDownloadItem::resume()
{
    Q_D(QWebEngineDownloadItem);

    QWebEngineDownloadItem::DownloadState state = d->downloadState;

    if (d->downloadFinished || (state != QWebEngineDownloadItem::DownloadInProgress && state != QWebEngineDownloadItem::DownloadInterrupted))
        return;
    d->profile->browserContext()->resumeDownload(d->downloadId);
}

/*!
    Returns the download item's ID.
*/

quint32 QWebEngineDownloadItem::id() const
{
    Q_D(const QWebEngineDownloadItem);
    return d->downloadId;
}

/*!
    \fn QWebEngineDownloadItem::finished()

    This signal is emitted when the download finishes.

    \sa state(), isFinished()
*/

/*!
    \fn QWebEngineDownloadItem::isPausedChanged(bool isPaused)
    \since 5.10

    This signal is emitted whenever \a isPaused changes.

    \sa pause(), isPaused()
*/

/*!
    \fn QWebEngineDownloadItem::stateChanged(DownloadState state)

    This signal is emitted whenever the download's \a state changes.

    \sa state(), DownloadState
*/

/*!
    \fn QWebEngineDownloadItem::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)

    This signal is emitted to indicate the progress of the download request.

    The \a bytesReceived parameter indicates the number of bytes received, while
    \a bytesTotal indicates the total number of bytes expected to be downloaded.
    If the size of the file to be downloaded is not known, \c bytesTotal will be
    0.

    \sa totalBytes(), receivedBytes()
*/

/*!
    \enum QWebEngineDownloadItem::DownloadState

    This enum describes the state of the download:

    \value DownloadRequested Download has been requested, but has not been accepted yet.
    \value DownloadInProgress Download is in progress.
    \value DownloadCompleted Download completed successfully.
    \value DownloadCancelled Download has been cancelled.
    \value DownloadInterrupted Download has been interrupted (by the server or because of lost
            connectivity).
*/

/*!
    \enum QWebEngineDownloadItem::SavePageFormat
    \since 5.7

    This enum describes the format that is used to save a web page.

    \value UnknownSaveFormat This is not a request for downloading a complete web page.
    \value SingleHtmlSaveFormat The page is saved as a single HTML page. Resources such as images
           are not saved.
    \value CompleteHtmlSaveFormat The page is saved as a complete HTML page, for example a directory
            containing the single HTML page and the resources.
    \value MimeHtmlSaveFormat The page is saved as a complete web page in the MIME HTML format.
*/

/*!
    \enum QWebEngineDownloadItem::DownloadType
    \since 5.8

    Describes the requested download's type.

    \value Attachment The web server's response includes a
           \c Content-Disposition header with the \c attachment directive. If \c Content-Disposition
           is present in the reply, the web server is indicating that the client should prompt the
           user to save the content regardless of the content type.
           See \l {RFC 2616 section 19.5.1} for details.
    \value DownloadAttribute The user clicked a link with the \c download
           attribute. See \l {HTML download attribute} for details.
    \value UserRequested The user initiated the download, for example by
           selecting a web action.
    \value SavePage Saving of the current page was requested (for example by
           the \l{QWebEnginePage::WebAction}{QWebEnginePage::SavePage} web action).
*/

/*!
    \enum QWebEngineDownloadItem::DownloadInterruptReason
    \since 5.9

    Describes the reason why a download was interrupted:

    \value NoReason Unknown reason or not interrupted.
    \value FileFailed General file operation failure.
    \value FileAccessDenied The file cannot be written locally, due to access restrictions.
    \value FileNoSpace Insufficient space on the target drive.
    \value FileNameTooLong The directory or file name is too long.
    \value FileTooLarge The file size exceeds the file system limitation.
    \value FileVirusInfected The file is infected with a virus.
    \value FileTransientError Temporary problem (for example the file is in use,
           out of memory, or too many files are opened at once).
    \value FileBlocked The file was blocked due to local policy.
    \value FileSecurityCheckFailed An attempt to check the safety of the download
           failed due to unexpected reasons.
    \value FileTooShort An attempt was made to seek past the end of a file when
           opening a file (as part of resuming a previously interrupted download).
    \value FileHashMismatch The partial file did not match the expected hash.

    \value NetworkFailed General network failure.
    \value NetworkTimeout The network operation has timed out.
    \value NetworkDisconnected The network connection has been terminated.
    \value NetworkServerDown The server has gone down.
    \value NetworkInvalidRequest The network request was invalid (for example, the
           original or redirected URL is invalid, has an unsupported scheme, or is disallowed by policy).

    \value ServerFailed General server failure.
    \value ServerBadContent The server does not have the requested data.
    \value ServerUnauthorized The server did not authorize access to the resource.
    \value ServerCertProblem A problem with the server certificate occurred.
    \value ServerForbidden Access forbidden by the server.
    \value ServerUnreachable Unexpected server response (might indicate that
           the responding server may not be the intended server).
    \value UserCanceled The user canceled the download.
*/

/*!
    Returns the download item's current state.

    \sa DownloadState
*/

QWebEngineDownloadItem::DownloadState QWebEngineDownloadItem::state() const
{
    Q_D(const QWebEngineDownloadItem);
    return d->downloadState;
}

/*!
    Returns the the total amount of data to download in bytes.

    \c -1 means the size is unknown.
*/

qint64 QWebEngineDownloadItem::totalBytes() const
{
    Q_D(const QWebEngineDownloadItem);
    return d->totalBytes;
}

/*!
    Returns the amount of data in bytes that has been downloaded so far.

    \c -1 means the size is unknown.
*/

qint64 QWebEngineDownloadItem::receivedBytes() const
{
    Q_D(const QWebEngineDownloadItem);
    return d->receivedBytes;
}

/*!
    Returns the download's origin URL.
*/

QUrl QWebEngineDownloadItem::url() const
{
    Q_D(const QWebEngineDownloadItem);
    return d->downloadUrl;
}

/*!
    \since 5.6

    Returns the MIME type of the download.
*/

QString QWebEngineDownloadItem::mimeType() const
{
    Q_D(const QWebEngineDownloadItem);
    return d->mimeType;
}

/*!
    Returns the full target path where data is being downloaded to.

    The path includes the file name. The default suggested path is the standard download location
    and file name is deduced not to overwrite already existing files.
*/

QString QWebEngineDownloadItem::path() const
{
    Q_D(const QWebEngineDownloadItem);
    return d->downloadPath;
}

/*!
    Sets the full target path to download the file to.

    The \a path should also include the file name. The download path can only be set in response
    to the QWebEngineProfile::downloadRequested() signal before the download is accepted.
    Past that point, this function has no effect on the download item's state.
*/
void QWebEngineDownloadItem::setPath(QString path)
{
    Q_D(QWebEngineDownloadItem);
    if (d->downloadState != QWebEngineDownloadItem::DownloadRequested) {
        qWarning("Setting the download path is not allowed after the download has been accepted.");
        return;
    }

    d->downloadPath = path;
}

/*!
    Returns whether this download is finished (completed, cancelled, or non-resumable interrupted state).

    \sa finished(), state(),
*/

bool QWebEngineDownloadItem::isFinished() const
{
    Q_D(const QWebEngineDownloadItem);
    return d->downloadFinished;
}

/*!
    Returns whether this download is paused.

    \sa pause()
*/

bool QWebEngineDownloadItem::isPaused() const
{
    Q_D(const QWebEngineDownloadItem);
    return d->downloadPaused;
}

/*!
    Returns the format the web page will be saved in if this is a download request for a web page.
    \since 5.7

    \sa setSavePageFormat()
*/
QWebEngineDownloadItem::SavePageFormat QWebEngineDownloadItem::savePageFormat() const
{
    Q_D(const QWebEngineDownloadItem);
    return d->savePageFormat;
}

/*!
    Sets the \a format the web page will be saved in if this is a download request for a web page.
    \since 5.7

    \sa savePageFormat()
*/
void QWebEngineDownloadItem::setSavePageFormat(QWebEngineDownloadItem::SavePageFormat format)
{
    Q_D(QWebEngineDownloadItem);
    d->savePageFormat = format;
}

/*!
    Returns the requested download's type.
    \since 5.8

 */

QWebEngineDownloadItem::DownloadType QWebEngineDownloadItem::type() const
{
    Q_D(const QWebEngineDownloadItem);
    return d->type;
}

/*!
    Returns the reason why the download was interrupted.
    \since 5.9

    \sa interruptReasonString()
*/

QWebEngineDownloadItem::DownloadInterruptReason QWebEngineDownloadItem::interruptReason() const
{
    Q_D(const QWebEngineDownloadItem);
    return d->interruptReason;
}

/*!
    Returns a human-readable description of the reason for interrupting the download.
    \since 5.9

    \sa interruptReason()
*/

QString QWebEngineDownloadItem::interruptReasonString() const
{
    return BrowserContextAdapterClient::downloadInterruptReasonToString(
              static_cast<BrowserContextAdapterClient::DownloadInterruptReason>(interruptReason()));
}

QWebEngineDownloadItem::QWebEngineDownloadItem(QWebEngineDownloadItemPrivate *p, QObject *parent)
    : QObject(parent)
    , d_ptr(p)
{
    p->q_ptr = this;
}

/*! \internal
*/
QWebEngineDownloadItem::~QWebEngineDownloadItem()
{
}

QT_END_NAMESPACE
