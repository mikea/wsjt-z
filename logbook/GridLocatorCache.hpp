#ifndef WSJTX_GRID_LOCATOR_CACHE_HPP
#define WSJTX_GRID_LOCATOR_CACHE_HPP

#include <QHash>
#include <QString>

// Retains the most recently decoded CQ locator for each station on one band.
class GridLocatorCache final
{
public:
  // Records a valid four- or six-character Maidenhead locator. Invalid input
  // is ignored so a report token cannot replace a locator learned from CQ.
  void remember (QString const& call, QString const& grid);

  // Reconciles another lookup source with the CQ observation. A conflicting
  // square is rejected because the station may have operated mobile since the
  // external database was updated; matching sources contribute the longer grid.
  QString resolve (QString const& call, QString const& other_grid) const;

  // Drops all observations when reception moves to another band.
  void clear ();

private:
  QHash<QString, QString> grids_;
};

#endif
