/*
 * Config_params.hpp
 *
 *  Created on: 20 feb. 2019
 *      Author: torsten
 */

#ifndef CONFIG_PARAM_HPP_
#define CONFIG_PARAM_HPP_

#include <string>
#include "shared_ostream.hpp"
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
    };

    Config_param(const string& id, const string& value, const string& type, const string& default_value, const string& min = "", const string& max = "") :
        _name(id),
        _value(value),
        _type(type),
        _default_value(default_value),
        _min(min),
        _max(max)
    {
    };

    Config_param(const Config_param& param) :
        _name(param._name),
        _value(param._value),
        _type(param._type),
        _default_value(param._default_value),
        _min(param._min),
        _max(param._max)
    {
    };

    void set_value(const string& s)
    {
      _value = s;
    };

    string get_value() const
    {
      return _value;
    };

    string get_UCI_string_for_gui() const
    {
      string cmd = "option name " + _name + " type " + _type + " default " + _default_value;
      if (_type == "spin")
        cmd += " min " + _min + " max " + _max;
      return cmd;
    };
};


class Config_params
{
  protected:
    map<string, Config_param> _config_params;

  public:
    Config_params()
    {
      _config_params.insert(make_pair("max_search_level", *(new Config_param("max_search_level", "7", "spin", "7", "2", "8"))));
      _config_params.insert(make_pair("use_pruning", *(new Config_param("use_pruning", "true", "check", "true"))));
      _config_params.insert(make_pair("use_incremental_search", *(new Config_param("use_incremental_search", "true", "check", "true"))));
      _config_params.insert(make_pair("search_until_no_captures", *(new Config_param("search_until_no_captures", "false", "check", "false"))));
    }

    map<string, Config_param> get_map() const
    {
      return _config_params;
    }

    void set_config_param(const string& name, const string& value)
    {
      auto search = _config_params.find(name);
      if (search != _config_params.end())
        search->second.set_value(value);
    }

    string get_config_param(const string& name) const
    {
      auto it = _config_params.find(name);
      if (it != _config_params.end())
        return it->second.get_value();
      else
      {
        return "";
      }
    }

    string get_params_string() const
    {
      string s = "";
      for (const pair<const string, Config_param>& param:_config_params)
      {
        s += param.first + ": " + param.second.get_value() + "\n";
      }
      return s;
    }
};

} // namespace C2_chess

#endif /* CONFIG_PARAM_HPP_ */
