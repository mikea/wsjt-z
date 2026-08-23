#include "GridLocatorCache.hpp"

#include <QRegularExpression>

namespace
{
QRegularExpression const grid_pattern {
  QStringLiteral ("^(?!RR73$)[A-R]{2}[0-9]{2}(?:[A-X]{2})?$")};

// Normalizes valid Maidenhead locators for reliable prefix comparison.
QString normalized_grid (QString const& grid)
{
  auto const normalized = grid.trimmed ().toUpper ();
  return grid_pattern.match (normalized).hasMatch () ? normalized : QString {};
}
}

void GridLocatorCache::remember (QString const& call, QString const& grid)
{
  auto const normalized = normalized_grid (grid);
  if (!normalized.isEmpty ())
    {
      grids_.insert (call.trimmed ().toUpper (), normalized);
    }
}

QString GridLocatorCache::resolve (QString const& call, QString const& other_grid) const
{
  auto const cached = grids_.value (call.trimmed ().toUpper ());
  auto const other = normalized_grid (other_grid);
  if (cached.isEmpty ()) return other;
  if (other.isEmpty () || cached.left (4) != other.left (4)) return cached;
  return other.size () > cached.size () ? other : cached;
}

void GridLocatorCache::clear ()
{
  grids_.clear ();
}
