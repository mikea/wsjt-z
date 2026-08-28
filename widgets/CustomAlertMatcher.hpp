#ifndef CUSTOM_ALERT_MATCHER_HPP_
#define CUSTOM_ALERT_MATCHER_HPP_

class QString;
class QStringList;

namespace CustomAlertMatcher
{
  // Reports whether a callsign satisfies any custom-alert item. Items beginning
  // with '^' match only at the start of the callsign; all other items retain
  // the historical substring behavior. Comparisons are case-insensitive.
  bool matches (QString const& callsign, QStringList const& items);
}

#endif
