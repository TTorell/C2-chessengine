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
#include "uci.hpp"
#include "magic_enum.hpp"

namespace C2_chess
{

void read_input(Circular_fifo* input_buffer, Game* game);

void write_output(Circular_fifo* output_buffer);

void close_threads(std::thread& input_thread, std::thread& output_thread);

void print_help_txt();

std::string uci_style(const Bitmove& move);

// Global booleans which can be set to false to stop input and output threads
// and to tell if the engine succeeded to open the command_log.txt file, where
// it e.g. logs all input and output commands it receives and sends during a
// game or (in the future) also during an analysis task.
// If the chess-GUI has been set to start the C2-engine from a working
// directory which is write-protected for the user who starts it, then C2 will
// fail to open that logfile.

std::atomic<bool> input_thread_running(true);
std::atomic<bool> output_thread_running(true);

// This method will run in the input_thread.
void read_input(Circular_fifo* input_buffer, Game* game)
{
  Shared_ostream* logfile = Shared_ostream::get_instance();
  while (input_thread_running)
  {
    std::string command;
    getline(std::cin, command);
    if (command.empty())
    {
      continue;
    }
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


} // End of namespace C2_chess

using namespace C2_chess;

int main(int argc, char* argv[])
{
// The engine can log all communication with GUI, and
// other stuff to a logfile.
// This requires that the engine has been started by
// the GUI from a directory where it has write-permisson.
// And that's of course where you can find the logfile.
  std::ofstream ofs(get_logfile_name());
// TODO: check if open failed
  Shared_ostream& logfile = *(Shared_ostream::get_instance(ofs, ofs.is_open()));

// Open a shared_ostream for the cmdline-interface:
  Shared_ostream& cmdline = *(Shared_ostream::get_cout_instance());

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
  game.init();

  // Thread which receives input commands and puts them in the
  // ipput_buffer, While the main engine process is "thinking"
  // about other things.
  std::thread input_thread(read_input, &input_buffer, &game);

// Thread which buffers output commands from the engine.
  std::thread output_thread(write_output, &output_buffer);

  UCI Uci;

  std::string command;

  logfile << "Engine started" << "\n";
  while (true)
  {
    // Processing the GUI commands
    command = input_buffer.get();
    if (command.empty())
    {
      std::this_thread::sleep_for(std::chrono::microseconds(50));
      continue;
    }

    auto uci_command = Uci.parse_command(command, output_buffer, config_params);

    if (uci_command == UCI_cmd::go)
    {
      // (try to) Find the best move in the position
      Bitmove bestmove = game.engine_go(config_params, Uci.get_go_params());
      output_buffer.put("bestmove " + uci_move(bestmove));
      continue;
    }

    // The stop and quit commands are first taken care of in the
    // input thread to stop the search-thread if it's searching
    // (or pondering, which isn't implemented yet).
    // but the commands are also put in the input-buffer for
    // this main-thread to read.
    if (uci_command == UCI_cmd::quit)
    {
      break;
    }
    if (uci_command == UCI_cmd::stop)
    {
      // We have already stopped searching when we reach this code,
      // because the searching also runs in this thread.
      continue;
    }

    if (uci_command == UCI_cmd::position)
    {
      game.read_position(Uci.get_position_params());
    }

    if (uci_command == UCI_cmd::ucinewgame)
    {
      // We have nothing to clean up before starting a new game.
      // It will be taken care of when the first position is sent
      // from the GUI.
      // Maybe we could close the logfile and start a new one, but
      // for now everything is logged until the GUI closes the engine
      // down by a quit command.
      continue;
    }

    // this is a homemade command specific for this engine.
    if (uci_command == UCI_cmd::cmd)
    {
      close_threads(input_thread, output_thread);
      // Tell the cmdline instance to be ready for
      // writing to cout.
      cmdline.open();
      // Starting command-line play
      play_on_cmd_line(config_params);
      break;
    }
    if (uci_command == UCI_cmd::unknown)
    {
      logfile << "Unknown UCI-command:\n" << command << "\n";
      continue;
    }
  }
  close_threads(input_thread, output_thread);
  ofs.close();
  return 0;
}

