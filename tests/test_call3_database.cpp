#include <QtTest>

#include <QFile>
#include <QTemporaryDir>

#include "logbook/Call3Database.hpp"
#include "logbook/GridLocatorCache.hpp"

class TestCall3Database final : public QObject
{
  Q_OBJECT

private:
  // Exercises normalization and rejection using representative terrestrial
  // and EME-style records, including comments and malformed locators.
  Q_SLOT void resolves_only_valid_grid_records ();

  // Verifies that replacing CALL3.TXT during a session invalidates the cache
  // used by subsequent auto-call decisions.
  Q_SLOT void reloads_replaced_database ();

  // Ensures a CQ observation rejects stale mobile-station data from another
  // square while accepting a more precise locator with the same prefix.
  Q_SLOT void cq_grid_constrains_other_sources ();

  // Ensures changing bands removes CQ observations and permits an external
  // locator to become authoritative again.
  Q_SLOT void clearing_cq_grids_removes_constraint ();
};

void TestCall3Database::resolves_only_valid_grid_records ()
{
  QTemporaryDir directory;
  QVERIFY (directory.isValid ());
  QFile database {directory.filePath ("CALL3.TXT")};
  QVERIFY (database.open (QIODevice::WriteOnly | QIODevice::Text));
  database.write ("// Version: test\n"
                  "K1ABC,fn20ab,,,\n"
                  "W2XYZ,EM12,,,\n"
                  "BAD1,ZZ99,,,\n"
                  "BAD2,RR73,,,\n");
  database.close ();

  Call3Database call3 {QDir {directory.path ()}};
  QCOMPARE (call3.grid_for_call (" k1abc "), QString {"FN20AB"});
  QCOMPARE (call3.grid_for_call ("W2XYZ"), QString {"EM12"});
  QVERIFY (call3.grid_for_call ("BAD1").isEmpty ());
  QVERIFY (call3.grid_for_call ("BAD2").isEmpty ());
  QVERIFY (call3.grid_for_call ("N0CALL").isEmpty ());
}

void TestCall3Database::reloads_replaced_database ()
{
  QTemporaryDir directory;
  QVERIFY (directory.isValid ());
  auto const path = directory.filePath ("CALL3.TXT");
  QFile database {path};
  QVERIFY (database.open (QIODevice::WriteOnly | QIODevice::Text));
  database.write ("K1ABC,FN20,,,\n");
  database.close ();

  Call3Database call3 {QDir {directory.path ()}};
  QCOMPARE (call3.grid_for_call ("K1ABC"), QString {"FN20"});

  QVERIFY (database.open (QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text));
  database.write ("K1ABC,EM12AB,,,,\n");
  database.close ();
  QCOMPARE (call3.grid_for_call ("K1ABC"), QString {"EM12AB"});
}

void TestCall3Database::cq_grid_constrains_other_sources ()
{
  GridLocatorCache cache;
  cache.remember ("K1ABC", "FN20");
  cache.remember ("K1ABC", "RR73");

  QCOMPARE (cache.resolve ("K1ABC", "FN20AB"), QString {"FN20AB"});
  QCOMPARE (cache.resolve ("K1ABC", "EM12AB"), QString {"FN20"});
  QCOMPARE (cache.resolve ("K1ABC", "RR73"), QString {"FN20"});

  cache.remember ("K1ABC", "EM12CD");
  QCOMPARE (cache.resolve ("K1ABC", "FN20AB"), QString {"EM12CD"});
}

void TestCall3Database::clearing_cq_grids_removes_constraint ()
{
  GridLocatorCache cache;
  cache.remember ("K1ABC", "FN20");
  cache.clear ();

  QCOMPARE (cache.resolve ("K1ABC", "EM12AB"), QString {"EM12AB"});
}

QTEST_GUILESS_MAIN (TestCall3Database)

#include "test_call3_database.moc"
