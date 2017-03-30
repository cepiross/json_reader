#ifndef UTILS_HPP__
#define UTILS_HPP__

#include "common.hpp"

#include <map>
#include <string>
// A simple registry for commands.
typedef int (*BrewFunction)();
typedef std::map<std::string, BrewFunction> BrewMap;

#endif // UTILS_HPP__