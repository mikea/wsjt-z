#ifndef LOTW_CONFIRMATION_HPP_
#define LOTW_CONFIRMATION_HPP_

class QString;

namespace LotwConfirmation
{
  // Reports whether an ADIF record carries an affirmative received LoTW QSL.
  // The parser honors the ADIF field length and accepts field names and the Y
  // value without regard to case. Missing or malformed fields are unconfirmed.
  bool received (QString const& adif_record);
}

#endif
