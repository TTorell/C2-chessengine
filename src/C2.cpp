#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>
#include "game.hpp"
#include "position_reader.hpp"
#include "C2_unittest.hpp"
#include "chessfuncs.hpp"
#include "chesstypes.hpp"
#include "Config_param.hpp"

using namespace std;
using namespace std::chrono;

namespace C2_chess
{

// Method for the cmdline-interface
int write_menue_get_choice(ostream& os)
{
  os << endl;
  os << "----------------------------" << endl;
  os << "Welcome to C2 Chess Program." << endl;
  os << "" << endl;
  os << "Menue:" << endl;
  os << "1. Play a game against C2." << endl;
  os << "2. Load a position from a .pgn-file" << endl;
  os << "   and start playing from there." << endl;
  os << "" << endl;

  while (true)
  {
    os << "Pick a choice [1]: ";

    string answer;
    cin >> answer;
    if (answer.size() == 0)
      return 1;
    switch (answer[0])
    {
      case '1':
        return 1;
      case '2':
        return 2;
      default:
        os << "Sorry, try again." << endl;
        continue;
    }
  }
}

// Method for the cmdline-interface
int back_to_main_menu()
{
  string input;
  cout << endl << "Back to main menu? [y/n]:";
  cin >> input;
  if (input == "y")
    return 0;
  else
  {
    cout << "Exiting C2" << endl;
    return -1;
  }
}

// Method for the cmdline-interface
col white_or_black()
{
  char st[100];
  bool try_again = true;
  while (try_again)
  {
    cout << "Which color would you like to play ? [w/b]: ";
    cin >> st;
    if (st[0] == 'w')
    {
      return col::white;
      try_again = false;
    }
    else if (st[0] == 'b')
    {
      return col::black;
      try_again = false;
    }
    else
      cout << "Enter w or b" << endl;
  }
  return col::white;
}

// Method for the cmdline-interface
int play_on_cmd_line()
{
  while (true)
  {
    int choice = write_menue_get_choice(cout);
    switch (choice)
    {
      case 1:

      {
        col color = white_or_black();
        Game game(color);
        game.setup_pieces();
        game.init();
        game.start();
        if (back_to_main_menu() != 0)
          return -1;
        continue;
      }
      case 2:
      {
        string input;
       
        col human_color = white_or_black();
        Game game(human_color);
        FEN_reader fr(game);
        Position_reader& pr = fr;
        string filename = GetStdoutFromCommand("cmd java -classpath \".\" ChooseFile");
        int status = pr.read_position(filename);
        if (status != 0)
        {
            cout << "Sorry, Couldn't read position from " << filename << endl;
            continue;
        }
        game.init();
        game.start();
        if (back_to_main_menu() != 0)
          exit(0);
        continue;
      }
      default:
        require(false,
        __FILE__,
                __FUNCTION__,
                __LINE__);
    }
  }
  return 0;
}

// This Mthod e.g. splits up commands from the GUI into tokens (words)
vector<string> split(const string &s, char delim)
{
  vector<string> result;
  stringstream ss(s);
  string item;

  while (getline(ss, item, delim))
  {
    result.push_back(item);
  }

  return result;
}

// Set default values for the few config parameters
void init_config_params(map<string, Config_param>& config_params)
{
  config_params.insert(make_pair("use_pruning", *(new Config_param("use_pruning", "true", "check", "true"))));
  config_params.insert(make_pair("max_search_level", *(new Config_param("max_search_level", "7", "spin", "7", "1", "8"))));
}

// Global booleans which can be set to false to stop input and output threads
// and to tell if the engine succeeded to open the command_log.txt file, where
// it e.g. saves all input and output commands it receives and sends during a
// game or (in the future) also during an analysis task.
// If the chess-GUI has been set to start the C2-engine from a working
// directory which is write-protected for the user who starts it, then C2 will
// fail to open that logfile. I'm not sure if it matters, but I have tried to
// protect against writing to a file that never was opened, throughout the code.
// (The engine seemed to work fine even before I did this. -- easy to test if I
// wish to do so.)
std::atomic<bool> input_thread_running(true);
std::atomic<bool> output_thread_running(true);
std::atomic<bool> logfile_is_open(true);

// Method for parsing input-commands from a chess-GUI (some commands are taken care of already in ḿain().
int parse_command(const string& command, Circular_fifo& output_buffer, Shared_ostream& logfile, Game& game, map<string, Config_param>& config_params)
{
  // We know at this point that the command string isn't empty
  vector<string> tokens = split(command, ' ');
  if (tokens[0] == "uci")
  {
    output_buffer.put("id name C2 pre release");
    output_buffer.put("id author Torsten Torell");
    // Tell GUI which parameters are configurable.
    pair<const string, Config_param> pair;
    pair.second.get_command_for_gui();
    for (auto it : config_params)
    {
      output_buffer.put(it.second.get_command_for_gui());
    }
    output_buffer.put("uciok");
  }

  // Since we are inside parse_command(), the engine obviously has been started.
  if (tokens[0] == "isready")
    output_buffer.put("readyok");

  if (tokens[0] == "setoption")
  {
    // All my config-parameters have names which are only one token long.
    string name = "", value = "";
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
      auto search = config_params.find(name);
      if (search != config_params.end())
        search->second.set_value(value);
    }
  }
  if (tokens[0] == "position")
  {
    FEN_reader reader(game);
    // Discard the first 13 characters, "position fen ", from the command.
    string fen_string = command.substr(13, command.size() - 12);
    // Remove possible pieces and information from the main chessboard.
    game.clear_chessboard();
    // The information about the new requested position comes in
    // Forsyth–Edwards Notation.
    // Read the FEN-coded position from the GUI command and set up
    // the board accordingly.
    reader.parse_FEN_string(fen_string);
    // print the position to the logfile
    if (logfile_is_open)
    {
      game.write_diagram(logfile) << "\n";
    }
  }
  // To implement go I think I needed some variables not available here,
  // So I took care of it in main instead.
  if (tokens[0] == "go")
    return 1;

  return 0;
}

void read_input(Circular_fifo* input_buffer, Shared_ostream* logfile)
{
  while (input_thread_running)
  {
    string command;
    getline(cin, command);
    (*input_buffer).put(command);
    if (logfile_is_open)
      (*logfile) << "input: " << command << "\n";
    if (command == "quit")
      break;
    this_thread::sleep_for(milliseconds(20));
  }
  //cout << "input_thread: quit" << endl;
  this_thread::yield();
}

void write_output(Circular_fifo* output_buffer, Shared_ostream* logfile)
{
  string command;
  while (output_thread_running)
  {
    command = (*output_buffer).get();
    if (!command.empty())
    {
      if (logfile_is_open)
        (*logfile) << "output: " << command << "\n";
      if (command == "quit")
        break;
      cout << command << endl;
    }
    this_thread::sleep_for(milliseconds(20));
  }
  this_thread::yield();
}

} // End of namespace C2_chess

using namespace C2_chess;

int main()
{

  //ofstream testlog("testlog.txt");
  //C2_unit_test test;
  //test.main_test(cout);
  //test.main_test(testlog);
  //testlog.close();
  //string diff = GetStdoutFromCommand("diff testlog.txt testref.txt");
  //if (!diff.empty())
  // cout << "### UNIT TEST FAILED! :" << endl << diff << endl;

  ofstream ofs("command_log.txt");
  if (!ofs.is_open())
    logfile_is_open = false;
  Shared_ostream logfile(ofs);
  Game game;
  string command;
  Circular_fifo input_buffer;
  Circular_fifo output_buffer;
  thread output_thread(write_output, &output_buffer, &logfile);
  thread input_thread(read_input, &input_buffer, &logfile);

  map<string, Config_param> config_params;
  init_config_params(config_params);
  logfile << "Engine started" << "\n";
  while (true)
  {
    command = input_buffer.get();
    if (!command.empty())
    {
      if (command == "quit")
      {
        output_buffer.put("quit"); // closes output_thread
        break;
      }
      if (command == "cmd")
      {
        input_thread_running = false;
        output_thread_running = false;
        play_on_cmd_line();
        break;
      }
      int rc = parse_command(command, output_buffer, logfile, game, config_params);
      if (rc == 1) // = go
      {
        // Init the main chess board with preserved position
        // and calculate all possible moves.
        game.init();
        // Find best move
        Move bestmove = game.engine_go(logfile, logfile_is_open, config_params);
        output_buffer.put(bestmove.bestmove_engine_style());
      }
      if (command == "quit")
        break;
    }
    this_thread::sleep_for(milliseconds(20));
  }
  input_thread_running = false;
  output_thread_running = false;
  this_thread::sleep_for(milliseconds(40));
  ofs.close();
  return 0;
}

