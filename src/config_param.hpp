/*
 * Config_params.hpp
 *
 *  Created on: 20 feb. 2019
 *      Author: torsten
 */

#ifndef CONFIG_PARAM_HPP_
#define CONFIG_PARAM_HPP_

#include <string>
#include <map>
#include "shared_ostream.hpp"

namespace C2_chess
{
class Config_param
{
  protected:
    std::string _name;
    std::string _value;
    std::string _type;
    std::string _default_value;
    std::string _min;
    std::string _max;

  public:

    Config_param(const std::string &id,
                 const std::string &value,
                 const std::string &type,
                 const std::string &default_value,
                 const std::string &min = "",
                 const std::string &max = "") :
        _name(id),
        _value(value),
        _type(type),
        _default_value(default_value),
        _min(min),
        _max(max)
    {
    }

    Config_param(const Config_param &param) :
        _name(param._name),
        _value(param._value),
        _type(param._type),
        _default_value(param._default_value),
        _min(param._min),
        _max(param._max)
    {
    }

    void set_value(const std::string &s)
    {
      _value = s;
    }

    std::string get_value() const
    {
      return _value;
    }

    std::string get_UCI_string_for_gui() const
    {
      std::string cmd = "option name " + _name + " type " + _type + " default " + _default_value;
      if (_type == "spin")
        cmd += " min " + _min + " max " + _max;
      return cmd;
    }
};

class Config_params
{
  protected:
    std::map<std::string, Config_param> _config_params;

  public:
    Config_params() :
        _config_params()
    {
      // This construct is just to not leakmemory
      Config_param p1("max_search_depth", "7", "spin", "7", "2", "8");
      Config_param p2("use_pruning", "true", "check", "true");
      Config_param p3("use_incremental_search", "true", "check", "true");
      Config_param p4("search_until_no_captures", "false", "check", "false");
      _config_params.insert(std::make_pair("max_search_depth", p1));
      _config_params.insert(std::make_pair("use_pruning", p2));
      _config_params.insert(std::make_pair("use_incremental_search", p3));
      _config_params.insert(std::make_pair("search_until_no_captures", p4));
    }

    std::map<std::string, Config_param> get_map() const
    {
      return _config_params;
    }

    void set_config_param(const std::string &name, const std::string &value)
    {
      auto search = _config_params.find(name);
      if (search != _config_params.end())
        search->second.set_value(value);
      else
      {
        Shared_ostream& logfile = *(Shared_ostream::get_instance());
        logfile << "Couldn't find config-parameter " << name << "\n";
      }
    }

    std::string get_config_param(const std::string &name) const
    {
      auto it = _config_params.find(name);
      if (it != _config_params.end())
        return it->second.get_value();
      else
      {
        return "";
      }
    }

    std::string get_params_string() const
    {
      std::string s = "";
      for (const std::pair<const std::string, Config_param> &param : _config_params)
      {
        s += param.first + ": " + param.second.get_value() + "\n";
      }
      return s;
    }

    friend std::ostream& operator<<(std::ostream& os, const Config_params& m);
};

} // namespace C2_chess

#endif /* CONFIG_PARAM_HPP_ */
