#include <QtTest/QtTest>

#include "logbook/LotwConfirmation.hpp"

class TestLotwConfirmation final : public QObject
{
  Q_OBJECT

private Q_SLOTS:
  // Verifies affirmative fields across the case and typed-field forms accepted
  // by ADIF exporters.
  void recognizes_received_confirmation ();

  // Ensures incomplete, negative, and malformed metadata never suppresses a
  // station that still needs a confirmed contact.
  void rejects_unconfirmed_records ();
};

void TestLotwConfirmation::recognizes_received_confirmation ()
{
  QVERIFY (LotwConfirmation::received ("<CALL:5>K1ABC <LOTW_QSL_RCVD:1>Y <EOR>"));
  QVERIFY (LotwConfirmation::received ("<lotw_qsl_rcvd:1>y"));
  QVERIFY (LotwConfirmation::received ("<LOTW_QSL_RCVD:1:S>Y"));
}

void TestLotwConfirmation::rejects_unconfirmed_records ()
{
  QVERIFY (!LotwConfirmation::received ("<CALL:5>K1ABC <EOR>"));
  QVERIFY (!LotwConfirmation::received ("<LOTW_QSL_RCVD:1>N"));
  QVERIFY (!LotwConfirmation::received ("<LOTW_QSL_RCVD:0>"));
  QVERIFY (!LotwConfirmation::received ("<LOTW_QSL_RCVD:2>YY"));
  QVERIFY (!LotwConfirmation::received ("<LOTW_QSL_RCVD:1Y"));
}

QTEST_APPLESS_MAIN (TestLotwConfirmation)

#include "test_lotw_confirmation.moc"
