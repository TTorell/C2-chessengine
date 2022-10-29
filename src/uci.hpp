/*
 * uci.hpp
 *
 *  Created on: Aug 19, 2022
 *      Author: torsten
 */

#ifndef SRC_UCI_HPP_
#define SRC_UCI_HPP_
#include <string>
#include "chesstypes.hpp"
#include "chessfuncs.hpp"
#include "circular_fifo.hpp"
#include "magic_enum.hpp"

namespace C2_chess
{

enum class UCI_cmd
{
  uci,
  setoption,
  isready,
  quit,
  ucinewgame,
  position,
  go,
  stop,
  cmd,
  unknown
};

struct Go_params
{
    bool infinite;
    double movetime;
    double wtime;
    double btime;
    double winc;
    double binc;

  public:
    Go_params() :
        infinite(false),
        movetime(0),
        wtime(0),
        btime(0),
        winc(0),
        binc(0)
    {
    }

    void clear()
    {
      std::memset(this, 0, sizeof(Go_params));
    }
};

struct Position_params
{
    bool moves;
    std::string FEN_string;
    std::vector<std::string> move_list;

  public:
    Position_params() :
        moves(false),
        FEN_string(""),
        move_list()
    {
    }

    void clear()
    {
      FEN_string = "";
      move_list.clear();
    }
};

class UCI
{
  private:
    std::vector<std::string> _command_tokens;
    Go_params _go_params;
    Position_params _position_params;

    // This method finds the token following var_name in the command.
    // Returns an empty string if not found.
    std::string parse_command_var(const std::string& var_name)
    {
      bool found = false;
      std::string token = "";
      for (const std::string& t : _command_tokens)
      {
        if (found)
        {
          token = t;
          break;
        }
        if (t == var_name)
          found = true;
      }
      return token;
    }

    void set_double_param(const std::string& name, double& param)
    {
      std::string s;
      s = parse_command_var(name);
      if (!s.empty())
        param = std::stod(s);
    }

    Go_params parse_go_params()
    {
      if (_command_tokens[0] == "go")
      {
        _go_params.clear();
        set_double_param("movetime", _go_params.movetime);
        set_double_param("wtime", _go_params.wtime);
        set_double_param("btime", _go_params.btime);
        set_double_param("winc", _go_params.winc);
        set_double_param("binc", _go_params.binc);
        _go_params.infinite = is_in_vector(_command_tokens, std::string("infinite"));
      }
      return _go_params;
    }

    Position_params parse_position_params(const std::string& command)
    {
      // The information about the new requested position comes
      // as a text-string in Forsyth–Edwards Notation,
      // or the string literal "startpos" which means the FEN-string
      // of the initial chessboard setup position.
      // Store the FEN-coded position from the GUI command.
      if (_command_tokens[0] == "position")
      {
        _position_params.clear();
        if (is_in_vector(_command_tokens, std::string("fen")))
        {
          _position_params.FEN_string = command.substr(13, command.size() - 1);
        }
        else if (is_in_vector(_command_tokens, std::string("startpos")))
        {
          _position_params.FEN_string = start_position_FEN;
        }

        if (is_in_vector(_command_tokens, std::string("moves")))
        {
          _position_params.moves = true;
          bool found_moves = false;
          for (const std::string& s : _command_tokens)
          {
            if (s == "moves")
            {
              found_moves = true;
              continue;
            }
            if (found_moves)
              _position_params.move_list.push_back(s);
          }
        }
      }
      return _position_params;
    }

  public:
    UCI():
      _command_tokens {},
      _go_params(),
      _position_params()
    {

    }
    Go_params get_go_params() const
    {
      return _go_params;
    }

    Position_params get_position_params() const
    {
      return _position_params;
    }

    // function for parsing UCI-input-commands from a chess-GUI
    // (some commands are taken care of already in the ḿain()-function.
    UCI_cmd parse_command(const std::string& command,
                          Circular_fifo& output_buffer,
                          Config_params& config_params)
    {
      // We know at this point that the command string isn't empty
      _command_tokens = split(command, ' ');

      if (_command_tokens[0] == "go")
      {
        parse_go_params();
        return UCI_cmd::go;
      }

      if (_command_tokens[0] == "position")
      {
        parse_position_params(command);
        return UCI_cmd::position;
      }

      if (_command_tokens[0] == "stop")
      {
        return UCI_cmd::stop;
      }

      // Since we are inside parse_command(), the engine thread
      // is obviously ready for new commands.
      if (_command_tokens[0] == "isready")
      {
        output_buffer.put("readyok");
        return UCI_cmd::isready;
      }

      if (_command_tokens[0] == "quit")
      {
        return UCI_cmd::quit;
      }

      if (_command_tokens[0] == "ucinewgame")
      {
        return UCI_cmd::ucinewgame;
      }

      if (_command_tokens[0] == "cmd")
      {
        return UCI_cmd::cmd;
      }

      if (_command_tokens[0] == "uci")
      {
        output_buffer.put("id name C2 experimental");
        output_buffer.put("id author Torsten Torell");
        // Tell GUI which parameters are configurable.
        for (auto it : config_params.get_map())
        {
          output_buffer.put(it.second.get_UCI_string_for_gui());
        }
        output_buffer.put("uciok");
        return UCI_cmd::uci;
      }

      if (_command_tokens[0] == "setoption")
      {
        // All my config-parameters have names which are only one token long.
        std::string name = "";
        std::string value = "";
        bool read_name = false;
        bool read_value = false;
        for (const std::string& token : _command_tokens)
        {
          if (read_name)
          {
            name = token;
            read_name = false;
          }
          if (read_value)
          {
            value = token;
            read_value = false;
          }
          if (token == "name")
            read_name = true;
          if (token == "value")
            read_value = true;
        }
        if (!(name.empty() || value.empty()))
        {
          config_params.set_config_param(name, value);
        }
        return UCI_cmd::setoption;
      }

      // Unknown command, or not implemented yet.
      return UCI_cmd::unknown;
    }

};
} // namespace C2_chess

#endif /* SRC_UCI_HPP_ */
