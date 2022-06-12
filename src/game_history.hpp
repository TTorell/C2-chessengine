/*
 * game_history.hpp
 *
 *  Created on: Jun 10, 2022
 *      Author: torsten
 */

#ifndef SRC_GAME_HISTORY_HPP_
#define SRC_GAME_HISTORY_HPP_

#include <cstdint>
//#include "bitboard.hpp"

namespace
{
  const uint32_t MAX_PLIES = 1024;
  const uint32_t MAX_REPEATED_POSITIONS = 50;
}

namespace C2_chess
{

struct Position_element
{
    uint64_t position_key;
};

class Game_history
{
  private:
    uint32_t _n_plies;
    uint32_t _n_repeated_positions;
    Position_element _moves_played[MAX_PLIES];
    uint64_t _repeated_positions[MAX_REPEATED_POSITIONS];
    bool _is_threefold_repetiotion;

  public:
    Game_history() :
        _n_plies(0),
        _n_repeated_positions(0),
        _moves_played {0},
        _repeated_positions {0},
        _is_threefold_repetiotion(false)
    {
    }

    void clear()
    {
      _n_plies = 0;
      _n_repeated_positions = 0;
      _is_threefold_repetiotion = false;
    }

    void takeback_moves(uint8_t old_n_plies, uint8_t old_n_repeated_positions)
    {
      assert(old_n_plies <= _n_plies && old_n_repeated_positions <= _n_repeated_positions);
      _n_plies = old_n_plies;
      _n_repeated_positions = old_n_repeated_positions;
      _is_threefold_repetiotion = false;
    }

    void add_position(uint64_t position_key)
    {
      if (_n_plies < MAX_PLIES)
      {
        _moves_played[_n_plies].position_key = position_key;
        for (int idx = _n_repeated_positions -1; idx >= 0; idx--)
        {
          if (_repeated_positions[idx] == position_key)
          {
            _is_threefold_repetiotion = true;
            _n_plies++;
            return;
          }
        }
        for (int idx = _n_plies - 1; idx >= 0; idx--)
        {
          if (_moves_played[idx].position_key == position_key)
          {
            if (_n_repeated_positions < MAX_REPEATED_POSITIONS)
              _repeated_positions[_n_repeated_positions++] = position_key;
            break;
          }
        }
        _n_plies++;
      }
    }

    bool is_threefold_repetition() const
    {
      return _is_threefold_repetiotion;
    }

    void get_sizes(uint32_t& n_plies, uint32_t& n_repeated_positions) const
    {
      n_plies = _n_plies;
      n_repeated_positions = _n_repeated_positions;
    }

};

} // namespace C2_chess

#endif /* SRC_GAME_HISTORY_HPP_ */
