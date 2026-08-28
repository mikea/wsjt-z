#include "CustomAlertMatcher.hpp"

#include <QString>
#include <QStringList>

namespace CustomAlertMatcher
{
  bool matches (QString const& callsign, QStringList const& items)
  {
    for (auto const& item : items)
      {
        if (item.startsWith ('^'))
          {
            auto const prefix = item.mid (1);
            if (!prefix.isEmpty () && callsign.startsWith (prefix, Qt::CaseInsensitive))
              {
                return true;
              }
          }
        else if (!item.isEmpty () && callsign.contains (item, Qt::CaseInsensitive))
          {
            return true;
          }
      }

    return false;
  }
}
