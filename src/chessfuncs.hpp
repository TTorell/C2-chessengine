/*
 * chessfuncs.hpp
 *
 *  Created on: 15 feb. 2019
 *      Author: torsten
 *
 *  Contains declarattion of some useful functions
 */

#ifndef CHESSFUNCS_HPP_
#define CHESSFUNCS_HPP_

#include <iostream>
#include <regex>
#include <vector>
#include "shared_ostream.hpp"



namespace C2_chess
{

class Move;

using std::ostream;
using std::string;

void require(bool b, string file, string method, int line);
void require_m(bool b, string file, string method, int line, const Move &m);
ostream& print_backtrace(ostream &os);
string GetStdoutFromCommand(string cmd);
bool regexp_match(const string &line, const string &regexp_string);
void log_time_diff(uint64_t nsec_stop, uint64_t nsec_start, Shared_ostream& logfile, int search_level, const Move& best_move, float score);

} // namespace
#endif //CHESSFUNCS_HPP_
