/*
 * pgn_info.hpp
 *
 *  Created on: 15 dec. 2018
 *      Author: torsten
 */

#ifndef _PGN_INFO
#define _PGN_INFO

#include <string>

namespace C2_chess
{

class PGN_info
{
  protected:
    std::string _event = "";
    std::string _site = "";
    std::string _date = "????.??.??";
    std::string _round = "";
    std::string _white = "";
    std::string _black = "";
    std::string _result = "*";

  public:
    const std::string& get_black() const
    {
      return _black;
    }

    void set_black(const std::string& black = "")
    {
      _black = black;
    }

    const std::string& get_date() const
    {
      return _date;
    }

    void set_date(const std::string& date = "????.??.??")
    {
      _date = date;
    }

    const std::string& get_event() const
    {
      return _event;
    }

    void set_event(const std::string& event = "")
    {
      _event = event;
    }

    const std::string& get_round() const
    {
      return _round;
    }

    void set_round(const std::string& round = "")
    {
      _round = round;
    }

    const std::string& get_site() const
    {
      return _site;
    }

    void set_site(const std::string& site = "")
    {
      _site = site;
    }

    const std::string& get_white() const
    {
      return _white;
    }

    void set_white(const std::string& white = "")
    {
      _white = white;
    }

    const std::string& get_result() const
    {
      return _result;
    }

    void set_result(const std::string& result = "*")
    {
      this->_result = result;
    }

    friend std::ostream& operator<<(std::ostream& os, const PGN_info& pgn_info)
    {
      os << "[Event \"" <<  pgn_info._event << "\"]" << std::endl;
      os << "[Site \"" << pgn_info._site <<  "\"]" << std::endl;
      os << "[Date \"" << pgn_info._date <<  "\"]" << std::endl;
      os << "[Round \"" << pgn_info._round <<  "\"]" << std::endl;
      os << "[White \"" << pgn_info._white <<  "\"]" << std::endl;
      os << "[Black \"" << pgn_info._black <<  "\"]" << std::endl;
      os << "[Result \"" << pgn_info._result <<  "\"]" << std::endl;
      return os;
    }
};
}
#endif /* _PGN_INFO */
