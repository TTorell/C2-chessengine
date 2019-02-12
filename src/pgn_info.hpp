/*
 * pgn_info.hpp
 *
 *  Created on: 15 dec. 2018
 *      Author: torsten
 */

#ifndef _PGN_INFO
#define _PGN_INFO


class PGN_info
{
  protected:
    string _event = "";
    string _site = "";
    string _date = "????.??.??";
    string _round = "";
    string _white = "";
    string _black = "";
    string _result = "*";

  public:
    const string& get_black() const
    {
      return _black;
    }

    void set_black(const string& black = "")
    {
      _black = black;
    }

    const string& get_date() const
    {
      return _date;
    }

    void set_date(const string& date = "????.??.??")
    {
      _date = date;
    }

    const string& get_event() const
    {
      return _event;
    }

    void set_event(const string& event = "")
    {
      _event = event;
    }

    const string& get_round() const
    {
      return _round;
    }

    void set_round(const string& round = "")
    {
      _round = round;
    }

    const string& get_site() const
    {
      return _site;
    }

    void set_site(const string& site = "")
    {
      _site = site;
    }

    const string& get_white() const
    {
      return _white;
    }

    void set_white(const string& white = "")
    {
      _white = white;
    }

    const string& get_result() const
    {
      return _result;
    }

    void set_result(const string& result = "*")
    {
      this->_result = result;
    }

    friend ostream& operator<<(ostream& os, const PGN_info& pgn_info)
    {
      os << "[Event \"" <<  pgn_info._event << "\"]" << endl;
      os << "[Site \"" << pgn_info._site <<  "\"]" << endl;
      os << "[Date \"" << pgn_info._date <<  "\"]" << endl;
      os << "[Round \"" << pgn_info._round <<  "\"]" << endl;
      os << "[White \"" << pgn_info._white <<  "\"]" << endl;
      os << "[Black \"" << pgn_info._black <<  "\"]" << endl;
      os << "[Result \"" << pgn_info._result <<  "\"]" << endl;
      return os;
    }
};

#endif /* _PGN_INFO */
