/*
 * Config_params.hpp
 *
 *  Created on: 20 feb. 2019
 *      Author: torsten
 */

#ifndef CONFIG_PARAM_HPP_
#define CONFIG_PARAM_HPP_

#include <string>

using std::string;

namespace C2_chess
{
class Config_param
{
  protected:
    string _name;
    string _value;
    string _type;
    string _default_value;
    string _min;
    string _max;

  public:
    Config_param()
    {
    }

    Config_param(const string& id, const string& value, const string& type, const string& default_value, const string& min = "", const string& max = "") :
        _name(id),
        _value(value),
        _type(type),
        _default_value(default_value),
        _min(min),
        _max(max)
    {
    }

    void set_value(const string& s)
    {
      _value = s;
    }

    string& get_value()
    {
      return _value;
    }

    string get_command_for_gui()
    {
      string cmd = "option name " + _name + " type " + _type + " default " + _default_value;
      if (_type == "spin")
        cmd += " min " + _min + " max " + _max;
      return cmd;
    }

};
}

#endif /* CONFIG_PARAM_HPP_ */
