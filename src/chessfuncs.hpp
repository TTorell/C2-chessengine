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
string iso_8859_1_to_utf8(const string& str);
string iso_8859_1_to_utf8(const char *c_string);

} // namespace
#endif //CHESSFUNCS_HPP_
