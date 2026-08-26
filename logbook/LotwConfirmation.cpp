#include "LotwConfirmation.hpp"

#include <QString>

namespace LotwConfirmation
{
  bool received (QString const& adif_record)
  {
    auto const field_start = adif_record.indexOf ("<LOTW_QSL_RCVD:", 0, Qt::CaseInsensitive);
    if (field_start < 0)
      {
        return false;
      }

    auto const value_start = adif_record.indexOf ('>', field_start);
    if (value_start < 0)
      {
        return false;
      }

    auto const length_start = adif_record.indexOf (':', field_start) + 1;
    auto const type_separator = adif_record.indexOf (':', length_start);
    auto const length_end = type_separator >= 0 && type_separator < value_start
      ? type_separator : value_start;
    bool valid_length {false};
    auto const field_length = adif_record.mid (length_start, length_end - length_start)
      .toInt (&valid_length);
    if (!valid_length || field_length != 1)
      {
        return false;
      }

    return "Y" == adif_record.mid (value_start + 1, field_length).toUpper ();
  }
}
