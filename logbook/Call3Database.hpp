#ifndef WSJTX_CALL3_DATABASE_HPP
#define WSJTX_CALL3_DATABASE_HPP

#include <QDateTime>
#include <QDir>
#include <QHash>
#include <QString>

// Provides cached callsign-to-locator lookups from the installed CALL3.TXT.
//
// The cache is refreshed when the file changes, allowing a database downloaded
// while WSJT-Z is running to become visible without restarting the application.
class Call3Database final
{
public:
  // Binds lookups to the application data directory containing CALL3.TXT.
  explicit Call3Database (QDir data_directory);

  // Returns a normalized four- or six-character Maidenhead locator. Missing
  // calls, malformed records, and an unavailable database produce an empty
  // string.
  QString grid_for_call (QString const& call);

private:
  // Rebuilds the in-memory index only when the file identity has changed. Size
  // supplements modification time because some replacements preserve mtimes.
  void refresh_if_changed ();

  QDir data_directory_;
  QHash<QString, QString> grids_;
  QDateTime modified_;
  qint64 size_ {-1};
};

#endif
