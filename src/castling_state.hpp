#ifndef _CASTLING_STATE
#define _CASTLING_STATE

#include "chesstypes.hpp"

namespace C2_chess
{

using std::ostream;
using std::istream;
class Castling_state {
  protected:
    bool _w_kingside_OK;
    bool _b_kingside_OK;
    bool _w_queenside_OK;
    bool _b_queenside_OK;
    bool _w_has_castled;
    bool _b_has_castled;

  public:
    Castling_state() :
        _w_kingside_OK(true), _b_kingside_OK(true), _w_queenside_OK(true), _b_queenside_OK(true), _w_has_castled(false), _b_has_castled(false)
    {
    }

    Castling_state(const Castling_state& cm) :
        _w_kingside_OK(cm._w_kingside_OK), _b_kingside_OK(cm._b_kingside_OK), _w_queenside_OK(cm._w_queenside_OK), _b_queenside_OK(cm._b_queenside_OK),
        _w_has_castled(cm._w_has_castled), _b_has_castled(cm._b_has_castled)
    {
    }

    ~Castling_state()
    {
    }

    Castling_state& operator=(const Castling_state& cm)
    {
      _w_kingside_OK = cm._w_kingside_OK;
      _w_queenside_OK = cm._w_queenside_OK;
      _b_kingside_OK = cm._b_kingside_OK;
      _b_queenside_OK = cm._b_queenside_OK;
      _w_has_castled = cm._w_has_castled;
      _b_has_castled = cm._b_has_castled;
      return *this;
    }

    bool is_castling_OK(col color) const
    {
      return (color == col::white ? _w_kingside_OK || _w_queenside_OK : _b_kingside_OK || _b_queenside_OK);
    }

    bool is_kingside_castling_OK(col color) const
    {
      return (color == col::white ? _w_kingside_OK : _b_kingside_OK);
    }

    bool is_queenside_castling_OK(col color) const
    {
      return (color == col::white ? _w_queenside_OK : _b_queenside_OK);
    }

    void king_rook_moved(col color)
    {
      color == col::white ? _w_kingside_OK = false : _b_kingside_OK = false;
    }

    void queen_rook_moved(col color)
    {
      color == col::white ? _w_queenside_OK = false : _b_queenside_OK = false;
    }

    void king_moved(col color)
    {
      color == col::white ? _w_kingside_OK = false, _w_queenside_OK = false : _b_kingside_OK = false, _b_queenside_OK = false;
    }

    bool has_castled(col color) const
    {
      if (color == col::white)
        return _w_has_castled;
      return _b_has_castled;
    }

    void set_has_castled(col color)
    {
      color == col::white ? _w_has_castled = true : _b_has_castled = true;
    }

    friend ostream& operator<<(ostream& os, const Castling_state& p);
    friend istream& operator>>(istream& is, Castling_state& cm);
};
}
#endif

