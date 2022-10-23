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

    // For test purpose
    Position_element& operator =(const Position_element& pe)
    {
      position_key = pe.position_key;
      return *this;
    }

    // For test purpose
    bool operator ==(const Position_element& pe) const
    {
      if (position_key != pe.position_key)
        return false;
      return true;
    }
};

struct History_state
{
    size_t _n_plies;
    size_t _n_repeated_positions;
    bool _is_threefold_repetiotion;

    History_state() :
        _n_plies(0), _n_repeated_positions(0), _is_threefold_repetiotion(false)
    {
    }

    History_state(uint32_t n_plies, uint32_t n_repeated_positions, bool is_threefold_repetiotion) :
        _n_plies(n_plies), _n_repeated_positions(n_repeated_positions), _is_threefold_repetiotion(is_threefold_repetiotion)
    {
    }

    void clear()
    {
      memset(this, 0, sizeof(History_state));
    }

    // For test purpose
    bool operator ==(const History_state& hs) const
    {
      if (_n_plies != hs._n_plies || _n_repeated_positions != hs._n_repeated_positions || _is_threefold_repetiotion != hs._is_threefold_repetiotion)
        return false;
      return true;
    }

    friend std::ostream& operator <<(std::ostream& os, History_state);
};

class Game_history
{
  private:
    History_state _state;
    Position_element _moves_played[MAX_HISTORY_PLIES];
    uint64_t _repeated_positions[MAX_REPEATED_POSITIONS];

    void remove_repeated_position(size_t idx)
    {
      assert(idx < _state._n_repeated_positions);
      for (auto i = idx; i < _state._n_repeated_positions - 1; i++)
        _repeated_positions[i] = _repeated_positions[i + 1];
      _state._n_repeated_positions--;
    }

  public:
    Game_history() :
        _state(), _moves_played {0}, _repeated_positions {0}
    {
    }

    // For test purpose
    Game_history(const Game_history& gh) :
        _state(gh._state), _moves_played {0}, _repeated_positions {0}
    {
      for (size_t i = 0; i < _state._n_plies; i++)
        _moves_played[i] = gh._moves_played[i];
      for (size_t i = 0; i < _state._n_repeated_positions; i++)
        _repeated_positions[i] = gh._repeated_positions[i];
    }

    // For test purpose
    bool operator ==(const Game_history& gh) const
    {
      if (_state != gh._state)
        return false;
      for (size_t i = 0; i < _state._n_repeated_positions; i++)
      {
        if (_repeated_positions[i] != gh._repeated_positions[i])
          return false;
      }
      for (size_t i = 0; i < _state._n_plies; i++)
      {
        if (_moves_played[i] != gh._moves_played[i])
          return false;
      }
      return true;
    }

    void clear()
    {
      _state.clear();
    }

    void takeback_moves(const History_state& state)
    {
      assert(state._n_plies <= _state._n_plies && state._n_repeated_positions <= _state._n_repeated_positions);
      _state = state;
    }

    void takeback_latest_move()
    {
      assert(_state._n_plies > 0);
      auto key_to_remove = _moves_played[_state._n_plies - 1].position_key;
      for (auto idx = static_cast<int>(_state._n_repeated_positions) - 1; idx >= 0; idx--)
      {
        if (_repeated_positions[idx] == key_to_remove)
        {
          remove_repeated_position(idx);
          break;
        }
      }
      _state._n_plies--;
    }

    void add_position(uint64_t position_key)
    {
      if (_state._n_plies < MAX_HISTORY_PLIES)
      {
        _moves_played[_state._n_plies].position_key = position_key;
        for (int idx = static_cast<int>(_state._n_repeated_positions) - 1; idx >= 0; idx--)
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
              _repeated_positions[_state._n_repeated_positions++ ] = position_key;
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
