#include "Call3Database.hpp"

#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>

namespace
{
QString const database_name {"CALL3.TXT"};
QRegularExpression const grid_pattern {
  QStringLiteral ("^(?!RR73$)[A-R]{2}[0-9]{2}(?:[A-X]{2})?$")};
}

Call3Database::Call3Database (QDir data_directory)
  : data_directory_ {data_directory}
{
}

QString Call3Database::grid_for_call (QString const& call)
{
  refresh_if_changed ();
  return grids_.value (call.trimmed ().toUpper ());
}

void Call3Database::refresh_if_changed ()
{
  QFileInfo const info {data_directory_.absoluteFilePath (database_name)};
  auto const current_modified = info.exists () ? info.lastModified () : QDateTime {};
  auto const current_size = info.exists () ? info.size () : qint64 {-1};
  if (current_modified == modified_ && current_size == size_)
    {
      return;
    }

  modified_ = current_modified;
  size_ = current_size;
  grids_.clear ();

  QFile file {info.absoluteFilePath ()};
  if (!file.open (QIODevice::ReadOnly | QIODevice::Text))
    {
      return;
    }

  QTextStream input {&file};
  while (!input.atEnd ())
    {
      auto const fields = input.readLine ().split (',');
      if (fields.size () < 2)
        {
          continue;
        }

      auto const call = fields.at (0).trimmed ().toUpper ();
      auto const grid = fields.at (1).trimmed ().toUpper ();
      if (!call.startsWith ("//") && grid_pattern.match (grid).hasMatch ())
        {
          grids_.insert (call, grid);
        }
    }
}
