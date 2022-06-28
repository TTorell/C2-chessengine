/*
 * game_history.hpp
 *
 *  Created on: Jun 10, 2022
 *      Author: torsten
 */

#ifndef SRC_GAME_HISTORY_HPP_
#define SRC_GAME_HISTORY_HPP_

#include <cstdint>

namespace
{
  const uint64_t MAX_HISTORY_PLIES = 1024;
  const uint64_t MAX_REPEATED_POSITIONS = 50;
}

namespace C2_chess
{

struct Position_element
{
    uint64_t position_key;
};

struct History_state
{
    uint32_t _n_plies;
    uint32_t _n_repeated_positions;
    bool _is_threefold_repetiotion;

    History_state():
      _n_plies(0),
      _n_repeated_positions(0),
      _is_threefold_repetiotion(false)
    {
    }

    History_state(uint32_t n_plies, uint32_t n_repeated_positions, bool is_threefold_repetiotion):
      _n_plies(n_plies),
      _n_repeated_positions(n_repeated_positions),
      _is_threefold_repetiotion(is_threefold_repetiotion)
    {
    }

    void clear()
    {
      memset(this, 0, sizeof(History_state));
    }

    bool operator ==(const History_state& hs) const
    {
      if (_n_plies != hs._n_plies ||
          _n_repeated_positions != hs._n_repeated_positions ||
          _is_threefold_repetiotion != hs._is_threefold_repetiotion)
        return false;
      return true;
    }

    friend std::ostream& operator << (std::ostream& os, History_state);
};

class Game_history
{
  private:
    History_state _state;
    Position_element _moves_played[MAX_HISTORY_PLIES];
    uint64_t _repeated_positions[MAX_REPEATED_POSITIONS];

  public:
    Game_history() :
        _state(),
        _moves_played{0},
        _repeated_positions{0}
    {
    }

    void clear()
    {
      _state.clear();
    }

    void takeback_moves(const History_state& state)
    {
      assert(state._n_plies <= _state._n_plies&& state._n_repeated_positions <= _state._n_repeated_positions);
      _state = state;
    }

    void add_position(uint64_t position_key)
    {
      if (_state._n_plies < MAX_HISTORY_PLIES)
      {
        _moves_played[_state._n_plies].position_key = position_key;
        for (int idx = static_cast<int>(_state._n_repeated_positions) -1; idx >= 0; idx--)
        {
          if (_repeated_positions[idx] == position_key)
          {
            _state._is_threefold_repetiotion = true;
            _state._n_plies++;
            return;
          }
        }
        for (int idx = static_cast<int>(_state._n_plies) - 1; idx >= 0; idx--)
        {
          if (_moves_played[idx].position_key == position_key)
          {
            if (_state._n_repeated_positions < MAX_REPEATED_POSITIONS)
              _repeated_positions[_state._n_repeated_positions++] = position_key;
            break;
          }
        }
        _state._n_plies++;
      }
    }

    bool is_threefold_repetition() const
    {
      return _state._is_threefold_repetiotion;
    }

    History_state get_state() const
    {
      return _state;
    }

};

} // namespace C2_chess

#endif /* SRC_GAME_HISTORY_HPP_ */
