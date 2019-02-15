#include <iostream>
#include "castling_state.hpp"

using namespace std;

namespace C2_chess
{

ostream& operator<<(ostream& os, const Castling_state& cm)
{
  if (cm._w_kingside_OK)
    os << 'K';
  if (cm._b_kingside_OK)
    os << 'k';
  if (cm._w_queenside_OK)
    os << 'Q';
  if (cm._b_queenside_OK)
    os << 'q';
  if (!cm._w_kingside_OK && !cm._b_kingside_OK && !cm._w_queenside_OK && !cm._b_queenside_OK)
    os << '-';
  return os;
}

istream& operator>>(istream& is, Castling_state& cm)
{
  string cs;
  is >> cs;
  cm._w_kingside_OK = false;
  cm._b_kingside_OK = false;
  cm._w_queenside_OK = false;
  cm._b_queenside_OK = false;
  for (unsigned i = 0; i < cs.size(); i++)
  {
    switch (cs[i])
    {
      case 'K':
        cm._w_kingside_OK = true;
        break;
      case 'k':
        cm._b_kingside_OK = true;
        break;
      case 'Q':
        cm._w_queenside_OK = true;
        break;
      case 'q':
        cm._b_queenside_OK = true;
    }
  }
  // We have no way of knowing the value of the following variables
  cm._w_has_castled = false; // these are only used for evaluating positions
  cm._b_has_castled = false;

  return is;
}
}
