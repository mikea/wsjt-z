#include "Call3Downloader.hpp"

#include <QFile>
#include <QRegularExpression>
#include <QSaveFile>
#include <QTextStream>

namespace
{
QString const terrestrial_url {"https://wsjt-x-improved.sourceforge.io/CALL3.TXT"};
QString const eme_url {"https://wsjt-x-improved.sourceforge.io/CALL3_EME.TXT"};
QString const database_name {"CALL3.TXT"};
QString const backup_name {"CALL3_backup.TXT"};

// Extracts a trimmed version marker while leaving structural validation to the
// caller, which also checks for a database record.
QString version_from_line (QString const& line)
{
  QString const prefix {"// Version:"};
  return line.startsWith (prefix) ? line.mid (prefix.size ()).trimmed () : QString {};
}
}

Call3Downloader::Call3Downloader (QNetworkAccessManager * network_manager,
                                  QDir data_directory, QObject * parent)
  : QObject {parent}
  , data_directory_ {data_directory}
  , network_manager_ {network_manager}
{
  connect (&download_, &FileDownload::complete, this,
           [this] (QString const&) { download_complete (); });
  connect (&download_, &FileDownload::error, this, &Call3Downloader::download_failed);
}

void Call3Downloader::start (Dataset dataset)
{
  if (active_)
    {
      return;
    }

  QString reason;
  if (!data_directory_.mkpath (QString {"."}) || !backup_installed_file (reason))
    {
      emit error (reason.isEmpty () ? tr ("Unable to create the CALL3 data directory.") : reason);
      return;
    }

  active_ = true;
  download_.configure (network_manager_,
                       Dataset::terrestrial == dataset ? terrestrial_url : eme_url,
                       data_directory_.absoluteFilePath (database_name),
                       "WSJT-Z CALL3 Downloader");
  download_.start_download ();
}

QString Call3Downloader::installed_version () const
{
  QFile file {data_directory_.absoluteFilePath (database_name)};
  if (!file.open (QIODevice::ReadOnly | QIODevice::Text))
    {
      return {};
    }

  QTextStream input {&file};
  while (!input.atEnd ())
    {
      auto const version = version_from_line (input.readLine ());
      if (!version.isEmpty ())
        {
          return version;
        }
    }
  return {};
}

bool Call3Downloader::backup_installed_file (QString& reason)
{
  auto const installed_path = data_directory_.absoluteFilePath (database_name);
  had_installed_file_ = QFile::exists (installed_path);
  if (!had_installed_file_)
    {
      return true;
    }
  return copy_atomically (installed_path, data_directory_.absoluteFilePath (backup_name), reason);
}

QString Call3Downloader::validate_installed_file () const
{
  QFile file {data_directory_.absoluteFilePath (database_name)};
  if (!file.open (QIODevice::ReadOnly | QIODevice::Text))
    {
      return {};
    }

  static QRegularExpression const grid_pattern {
    QStringLiteral ("^[A-R]{2}[0-9]{2}(?:[A-X]{2})?$")};
  QTextStream input {&file};
  QString version;
  bool found_record {false};
  while (!input.atEnd () && (version.isEmpty () || !found_record))
    {
      auto const line = input.readLine ();
      if (version.isEmpty ())
        {
          version = version_from_line (line);
        }
      auto const fields = line.split (',');
      found_record = found_record
        || (fields.size () >= 2 && !fields.at (0).trimmed ().isEmpty ()
            && grid_pattern.match (fields.at (1).trimmed ().toUpper ()).hasMatch ());
    }
  return found_record ? version : QString {};
}

bool Call3Downloader::restore_installed_file (QString& reason)
{
  auto const installed_path = data_directory_.absoluteFilePath (database_name);
  if (!had_installed_file_)
    {
      if (!QFile::exists (installed_path) || QFile::remove (installed_path))
        {
          return true;
        }
      reason = tr ("The invalid download could not be removed: %1").arg (installed_path);
      return false;
    }
  return copy_atomically (data_directory_.absoluteFilePath (backup_name), installed_path, reason);
}

bool Call3Downloader::copy_atomically (QString const& source, QString const& destination,
                                       QString& reason)
{
  QFile input {source};
  if (!input.open (QIODevice::ReadOnly))
    {
      reason = tr ("Unable to read %1: %2").arg (source, input.errorString ());
      return false;
    }

  QSaveFile output {destination};
  auto const contents = input.readAll ();
  if (!output.open (QIODevice::WriteOnly) || output.write (contents) != contents.size ()
      || !output.commit ())
    {
      reason = tr ("Unable to write %1: %2").arg (destination, output.errorString ());
      return false;
    }
  return true;
}

void Call3Downloader::download_complete ()
{
  active_ = false;
  auto const version = validate_installed_file ();
  if (!version.isEmpty ())
    {
      emit complete (version);
      return;
    }

  QString restore_error;
  auto const restored = restore_installed_file (restore_error);
  QString reason {tr ("The downloaded file is not a valid CALL3 database.")};
  if (!restored)
    {
      reason += tr (" The previous database could not be restored: %1").arg (restore_error);
    }
  emit error (reason);
}

void Call3Downloader::download_failed (QString const& reason)
{
  if (!active_)
    {
      return;
    }
  active_ = false;
  emit error (reason);
}
