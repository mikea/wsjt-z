#include <QtTest/QtTest>

#include "widgets/CustomAlertMatcher.hpp"

class TestCustomAlertMatcher final : public QObject
{
  Q_OBJECT

private Q_SLOTS:
  // Confirms that ordinary items continue to match anywhere in a callsign and
  // that comparisons preserve the existing case-insensitive input behavior.
  void matches_unanchored_items ();

  // Verifies that a leading caret constrains the remainder of an item to the
  // beginning of the callsign instead of treating it as a substring.
  void matches_anchored_items_only_at_start ();

  // Ensures an anchor without a prefix cannot turn every callsign into a match.
  void rejects_empty_anchored_item ();
};

void TestCustomAlertMatcher::matches_unanchored_items ()
{
  QStringList const alerts {"IOTA", "hf0pas"};

  QVERIFY (CustomAlertMatcher::matches ("AAIOTA", alerts));
  QVERIFY (CustomAlertMatcher::matches ("HF0PAS", alerts));
  QVERIFY (!CustomAlertMatcher::matches ("3D2ABC", alerts));
}

void TestCustomAlertMatcher::matches_anchored_items_only_at_start ()
{
  QStringList const alerts {"IOTA", "^3", "HF0PAS"};

  QVERIFY (CustomAlertMatcher::matches ("3D2ABC", alerts));
  QVERIFY (!CustomAlertMatcher::matches ("AA3ABC", alerts));
}

void TestCustomAlertMatcher::rejects_empty_anchored_item ()
{
  QVERIFY (!CustomAlertMatcher::matches ("SQ9FVE", QStringList {"^"}));
}

QTEST_APPLESS_MAIN (TestCustomAlertMatcher)

#include "test_custom_alert_matcher.moc"
