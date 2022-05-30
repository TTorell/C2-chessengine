#include <iomanip>
#include <deque>
#include <iterator>
#include <sstream>
#include "movelist.hpp"
#include "chessfuncs.hpp"
#include "shared_ostream.hpp"
#include "bitboard_with_utils.hpp"
namespace C2_chess
{

std::ostream& Movelog::write(std::ostream& os) const
{
  int moveno = _first_moveno;
  int increment = 0;
  for (int i = 0; i < static_cast<int>(_list.size()); i++)
  {
    std::ostringstream move;
    move << _list[i];
    if (i == 0 && _col_to_start == col::black)
    {
      os << moveno++ << "." << std::left << std::setw(9) << "  ...  " << move.str() << std::endl;
      increment = 1;
      continue;
    }

    if ((i + increment) % 2 == 0)
    {
      os << moveno++ << "." << std::left << std::setw(9) << move.str();
      if (i == static_cast<int>(_list.size() - 1))      // last move
        os << std::endl;
    }
    else
    {
      os << move.str() << std::endl;
    }
  }
  return os;
}
} // namespace C2_chess
