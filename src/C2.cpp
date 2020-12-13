#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <vector>
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

int write_menue_get_choice(ostream& os)
{

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
      return white;
      try_again = false;
    }
    else if (st[0] == 'b')
    {
      return black;
      try_again = false;
    }
    else
      cout << "Enter w or b" << endl;
  }
  return white;
}

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


void init_config_params(map<string, Config_param>& config_params)
{
  cout << "init_config_params" << endl;
  config_params.insert(make_pair("use_pruning", *(new Config_param("use_pruning", "true", "check", "true"))));
  config_params.insert(make_pair("max_search_level", *(new Config_param("max_search_level", "7", "spin", "7", "1", "8"))));
}

int parse_command(const string& command, Circular_fifo& output_buffer, Shared_ostream& logfile, Game& game, map<string, Config_param>& config_params)
{
  vector<string> tokens = split(command, ' ');
//  for (auto token : tokens)
//    cout << token << endl;
  if (tokens[0] == "uci")
  {
    output_buffer.put("id name C2 pre release");
    output_buffer.put("id author Torsten Torell");
    // Tell GUI which parameters are configurable.
//    map<string, Config_parameter>::iterator it = config_params.begin();
//    while (it != config_params.end())
    pair<const string,Config_param> pair;
    pair.second.get_command_for_gui();
    for (auto it : config_params)
    {
      output_buffer.put(it.second.get_command_for_gui());
    }
    output_buffer.put("uciok");
  }

  // Since we are inside parse_command(), the engine is ready.
  if (tokens[0] == "isready")
    output_buffer.put("readyok");

  if (tokens[0] == "setoption")
  {
    // All my parameters have names which are only one token long.
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
    string fen_string = command.substr(13, command.size() - 12);
    logfile << "fen_string = " << fen_string << "\n";
    game.write_diagram(logfile) << "\n";
    game.clear_chessboard();
    reader.parse_FEN_string(fen_string);
  }
  if (tokens[0] == "go")
    return 1;

  return 0;
}

void read_input(Circular_fifo* input_buffer, Shared_ostream* logfile)
{
  while (true)
  {
    string command;
    getline(cin, command);
    (*input_buffer).put(command);
    (*logfile) << "input: " << command << "\n";
    if (command == "quit")
      break;
    this_thread::sleep_for(milliseconds(20));
  }
  cout << "input_thread: quit" << endl;
  this_thread::yield();
}

void write_output(Circular_fifo* output_buffer, Shared_ostream* logfile)
{
  string command;
  while (true)
  {
    command = (*output_buffer).get();
    if (!command.empty())
    {
      if (command == "quit")
        break;
      cout << command << endl;
      (*logfile) << "output: " << command << "\n";
    }
    this_thread::sleep_for(milliseconds(20));
  }
  cout << "output_thread: quit" << endl;
  this_thread::yield();
}

}
using namespace C2_chess;

int main(int argc, char **argv)
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
  Shared_ostream logfile(ofs);
  Game game;
  string command;
  Circular_fifo input_buffer;
  Circular_fifo output_buffer;
  thread output_thread(write_output, &output_buffer, &logfile);
  thread input_thread(read_input, &input_buffer, &logfile);

  map<string, Config_param> config_params;
  init_config_params(config_params);
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
      int rc = parse_command(command, output_buffer, logfile, game, config_params);
      if (rc == 1)
      {
        game.init();
        Move bestmove = game.engine_go(logfile, config_params);
        output_buffer.put(bestmove.bestmove_engine_style());
      }
      if (command == "quit")
        break;
    }
    this_thread::sleep_for(milliseconds(20));
  }
  ofs.close();
  input_thread.join();
  output_thread.join();
  while (true)
  {
    int choice = write_menue_get_choice(cout);
    switch (choice)
    {
      case 1:

      {
        col c = white_or_black();
        Game game(c);
        game.setup_pieces();
        game.init();
        game.start();
        if (back_to_main_menu() != 0)
          exit(0);
        continue;
      }
      case 2:
      {
        string input;
        string filename = GetStdoutFromCommand("java -classpath \".\" ChooseFile");
        col human_color = white_or_black();
        Game game(human_color);
        FEN_reader fr(game);
        Position_reader& pr = fr;
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
  cout << "Main: program completed. Exiting.\n" << endl;
}

