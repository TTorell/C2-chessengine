#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>
#include <cstring>
#include <cstdio>

#include "bitboard_with_utils.hpp"
#include "chessfuncs.hpp"
#include "chesstypes.hpp"
#include "config_param.hpp"
#include "game.hpp"
#include "circular_fifo.hpp"
#include "current_time.hpp"

namespace C2_chess
{

// This method finds the token in the go-command following var_name.
// Returns an empty string if not found.
std::string parse_go_command(const std::vector<std::string>& command_tokens, const std::string& var_name)
{
  bool found = false;
  std::string token = "";
  for (const std::string& t : command_tokens)
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

// Global booleans which can be set to false to stop input and output threads
// and to tell if the engine succeeded to open the command_log.txt file, where
// it e.g. saves all input and output commands it receives and sends during a
// game or (in the future) also during an analysis task.
// If the chess-GUI has been set to start the C2-engine from a working
// directory which is write-protected for the user who starts it, then C2 will
// fail to open that logfile.

std::atomic<bool> input_thread_running(true);
std::atomic<bool> output_thread_running(true);

// Method for parsing input-commands from a chess-GUI (some commands are taken care of already in ḿain().
int parse_command(const std::string& command,
                  Circular_fifo& output_buffer,
                  Game& game,
                  Config_params& config_params,
                  std::vector<std::string>& returned_tokens)
{
  //Shared_ostream& logfile = *(Shared_ostream::get_instance());

  // We know at this point that the command string isn't empty
  std::vector<std::string> tokens = split(command, ' ');
  if (tokens[0] == "uci")
  {
    output_buffer.put("id name C2 experimental");
    output_buffer.put("id author Torsten Torell");
    // Tell GUI which parameters are configurable.
    for (auto it : config_params.get_map())
    {
      output_buffer.put(it.second.get_UCI_string_for_gui());
    }
    output_buffer.put("uciok");
  }

  // Since we are inside parse_command(), the engine is obviously ready.
  if (tokens[0] == "isready")
    output_buffer.put("readyok");

  if (tokens[0] == "setoption")
  {
    // All my config-parameters have names which are only one token long.
    std::string name = "", value = "";
    bool read_name = false;
    bool read_value = false;
    for (auto token : tokens)
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
  }
  if (tokens[0] == "position")
  {
    // Discard the first 13 characters, "position fen ", from the command.
    std::string fen_string = command.substr(13, command.size() - 12);
    // The information about the new requested position comes in
    // Forsyth–Edwards Notation.
    // Read the FEN-coded position from the GUI command and set up
    // the board accordingly.
    game.read_position_FEN(fen_string);
    // print the position to the logfile
    // game.write_diagram(logfile) << "\n";
  }
  // To implement go I think I needed some variables not available here,
  // So I took care of it in main instead.
  if (tokens[0] == "go")
  {
    returned_tokens = tokens;
    return 1;
  }
  // Otherwise just ignore the command.
  returned_tokens = tokens;
  return 0;
}

// This method will run in the input_thread.
void read_input(Circular_fifo* input_buffer, Game* game)
{
  Shared_ostream* logfile = Shared_ostream::get_instance();
  while (input_thread_running)
  {
    std::string command;
    getline(std::cin, command);
    input_buffer->put(command);
    (*logfile) << "input: " << command << "\n";
    if (command == "stop")
    {
      // Interrupt the probably searching main thread.
      game->set_time_left(false);
    }
    if (command == "quit")
    {
      // Interrupt the possibly searching main thread.
      // Note that the quit command was also put in the
      // input buffer, for the main thread to read when
      // it has finished searching.
      game->set_time_left(false);
      break;
    }
  }
  std::this_thread::yield();
}

// This method will run in the output_thread.
void write_output(Circular_fifo* output_buffer)
{
  Shared_ostream* logfile = Shared_ostream::get_instance();
  std::string command;
  while (output_thread_running)
  {
    command = (*output_buffer).get();
    if (!command.empty())
    {
      if (command == "quit_thread")
        break;
      (*logfile) << "output: " << command << "\n";
      std::cout << command << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
  }
  std::this_thread::yield();
}

void close_threads(std::thread& input_thread, std::thread& output_thread)
{
  //  Stop threads and wait for them to finish.
  input_thread_running = false;
  input_thread.join();
  output_thread_running = false;
  output_thread.join();
}

void print_help_txt()
{
  Shared_ostream& cmdline = *(Shared_ostream::get_cout_instance());

  cmdline << "\n" <<
          "Usage: C2 [-cmd | -help]" << "\n" << "\n" <<
          "Options:" << "\n" <<
          "--------" << "\n" <<
          "-help     : Prints this text." << "\n" <<
          "-cmd      : Starts a game in primitive cmd-line mode." << "\n" <<
          ""
          "C2 Without arguments will start the chess-engine" << "\n" << "\n";
}


std::string bestmove_engine_style(const BitMove& move)
{
  std::stringstream ss;
  ss << "bestmove " << 'a' + file_idx(move.from()) << rank_idx(move.from()) <<
     'a' + file_idx(move.to()) << rank_idx(move.to());
  if (move.properties() & move_props_promotion)
  {
    switch (move.promotion_piece_type())
    {
      case piecetype::Queen:
        ss << "q";
        break;
      case piecetype::Rook:
        ss << "r";
        break;
      case piecetype::Knight:
        ss << "n";
        break;
      case piecetype::Bishop:
        ss << "b" << std::endl;
        break;
      default:
        assert(false);
    }
  }
  return ss.str();
}

} // End of namespace C2_chess

using namespace C2_chess;

int main(int argc, char* argv[])
{
  //long long unsigned int input = 1;
//  uint64_t input = 1;
//  while (input)
//  {
//    std::cout << to_binary(input) << std::endl;
//    printf("log(%llu) = %u\n", static_cast<long long unsigned int>(input), LOG2(input));
//    input <<= 1;
//  }
//
//  for (int i = 0; i < argc; ++i)
//    std::cout << argv[i] << "\n";

// The engine can log all communication with GUI, and
// other stuff to the command_log.txt file.
// This requires that the engine has been started by
// the GUI from a directory where it has write-permisson.
// And that's of course where you can find the logfile.
  std::ofstream ofs(get_logfile_name());
// TODO: check if open failed
  Shared_ostream& logfile = *(Shared_ostream::get_instance(ofs, ofs.is_open()));

// Open a shared_ostream for the cmdline-interface:
  Shared_ostream& cmdline = *(Shared_ostream::get_cout_instance());

  if (argc > 1 && strcmp(argv[1], "-test") == 0)
  {
    Bitboard_with_utils chessboard;
    const char* const preferred_test_exec_dir = "/home/torsten/eclipse-workspace/C2-chessengine";
    if (!check_execution_dir(preferred_test_exec_dir))
      return false;
    std::string arg = "";
    if (argc > 2)
      arg = argv[2];
    bool result = chessboard.bitboard_tests(arg);
    if (!result)
      std::cout << "TESTS FAILED!" << std::endl;
//    if (old_mg_tests(0) != 0)
//      std::cout << "OLD TESTS FAILED!" << std::endl;
    return 0;
  }

  std::cout << "C2 experimental chess-engine" << std::endl;

// Configuration parameters for the engine, which the GUI can manipulate
// are stored in this class. Here they'll receive default values.
  Config_params config_params;

// If you start the engine with the -cmd argument from
// the bash terminal (or just start it without any commands
// and enter "cmd" from the keyboard to the running engine)
// you can play against it in text-mode. (A very primitive
// and rudimentary chess-board will be shown for each move,
// but at least we can check if it seems to work without
// having a chess GUI)
  if (argc > 1 && strcmp(argv[1], "-cmd") == 0)
  {
    play_on_cmd_line(config_params);
    return 0;
  }
// Apparently it's not a cmd-line game the user wants,
// to begin with at least. There's another way of entering
// command-line mode by sending "cmd" to the running chess-
// engine.
  cmdline.close(); // Will only set an _is_open bool to false.

// Allow --help as well as -help
  if (argc > 1 && regexp_match(argv[1], "-{1,2}help"))
  {
    print_help_txt();
    return 0;
  }

// The engine needs to store commands from the GUI until it's
// ready to read them.
  Circular_fifo input_buffer;

// It's not important to buffer output commands, but I do it anyway.
// ( symmetry above all)
  Circular_fifo output_buffer;

// Create an instance of the Game class.
  Game game(config_params);

// Thread which receives input commands and puts them in the
// ipput_buffer, While the main engine process is "thinking"
// about other things.
  std::thread input_thread(read_input, &input_buffer, &game);

// Thread which buffers output commands from the engine.
  std::thread output_thread(write_output, &output_buffer);

  std::string command;

  logfile << "Engine started" << "\n";
  while (true)
  {
    // Processing the GUI commands
    command = input_buffer.get();
    if (!command.empty())
    {
      if (command == "quit")
      {
        break;
      }
      if (command == "cmd")
      {
        close_threads(input_thread, output_thread);
        // Tell the cmdline instance to be ready for
        // writing to cout.
        cmdline.open();
        play_on_cmd_line(config_params);
        break;
      }
      if (command == "ucinewgame")
      {
        // We have nothing, but the move log in the Game class,
        // to clean up before starting a new game. Maybe we could
        // close the logfile and start a new one, but for now everything
        // is logged until the GUI closes the engine down by a quit command.
//        game.clear_move_log();
//        logfile << "New game started\n";
//        logfile.write_config_params(config_params);
        continue;
      }

      // Other commands, "uci", "position" etc, are sent to the parse_command() function and most
      // of them are also taken care of there
      std::vector<std::string> command_tokens; // will contain tokens from the following parsing.
      int rc = parse_command(command, output_buffer, game, config_params, command_tokens);
      if (rc == 1) // This means go ...
      {
        // Init the main chess board with preserved position
        // and calculate all possible moves.
//        game.init();

        // logfile << "go command_tokens:" << "\n";
        // for (std::string token:command_tokens)
        //   logfile << token << "\n";

        // Get the time-limit from the go-command, if there is one.
        std::string max_search_time = parse_go_command(command_tokens, "movetime");
        if (!max_search_time.empty())
        {
          logfile << "max_search_time = " << max_search_time << " milliseconds\n";
        }

        // Find the best move
        BitMove bestmove = game.engine_go(config_params, max_search_time);
        output_buffer.put(bestmove_engine_style(bestmove));
      }
    }
    //this_thread::sleep_for(milliseconds(3));
  }
  close_threads(input_thread, output_thread);
  ofs.close();
  return 0;
}

