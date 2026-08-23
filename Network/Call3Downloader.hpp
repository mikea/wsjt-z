#ifndef WSJTX_CALL3_DOWNLOADER_HPP
#define WSJTX_CALL3_DOWNLOADER_HPP

#include <QObject>
#include <QDir>

#include "FileDownload.hpp"

class QNetworkAccessManager;

// Downloads and validates the callsign-to-locator database used by Lookup.
//
// FileDownload atomically replaces CALL3.TXT. This wrapper additionally keeps
// the prior valid database as CALL3_backup.TXT and restores it if downloaded
// content is not a CALL3 database.
class Call3Downloader final : public QObject
{
  Q_OBJECT

public:
  enum class Dataset
  {
    terrestrial,
    eme
  };

  // Binds the downloader to the application's writable data directory. The
  // network manager must outlive this object.
  explicit Call3Downloader (QNetworkAccessManager *, QDir data_directory,
                            QObject * parent = nullptr);

  // Starts an update unless one is already active. Startup failures are
  // reported through error(), just like asynchronous network failures.
  void start (Dataset);

  // Reads the version marker from the installed CALL3.TXT. An empty result
  // means the file is absent or has no recognizable marker.
  QString installed_version () const;

signals:
  // Reports a validated database replacement and its version string.
  void complete (QString const& version);

  // Reports a failed update after preserving or restoring the old database.
  void error (QString const& reason);

private:
  // Saves the current database through QSaveFile so an interrupted backup
  // write cannot destroy the previous backup.
  bool backup_installed_file (QString& reason);

  // Checks both the version marker and at least one callsign/grid record. This
  // rejects successful HTTP responses containing an error or HTML page.
  QString validate_installed_file () const;

  // Reinstates the backup after invalid downloaded content; if there was no
  // original database, it removes only the newly downloaded destination.
  bool restore_installed_file (QString& reason);

  // Copies a file using atomic replacement at the destination.
  static bool copy_atomically (QString const& source, QString const& destination,
                               QString& reason);

  // Completes validation and rollback after FileDownload commits its result.
  void download_complete ();

  // Clears in-flight state and forwards a network failure to the UI.
  void download_failed (QString const& reason);

  QDir data_directory_;
  FileDownload download_;
  QNetworkAccessManager * network_manager_;
  bool active_ {false};
  bool had_installed_file_ {false};
};

#endif
