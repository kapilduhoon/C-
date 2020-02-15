#include <string>
#include <vector>

#include "format.h"

using std::string;
using std::vector;
using std::to_string;

// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function

string ConvertToString(int value, int limit) {
  if (value < limit) {
    return "0" + to_string(value);
  }
  return to_string(value);
}

string PrettifyTime(int hh, int mm, int ss) {
  string pretty;
  pretty.append(ConvertToString(hh, 10));
  pretty.append(":");
  pretty.append(ConvertToString(mm, 10));
  pretty.append(":");
  pretty.append(ConvertToString(ss, 10));
  return pretty;
}

string Format::ElapsedTime(long seconds[[maybe_unused]]) {
  int seconds_in_hour = 3600;
  int hours = seconds / seconds_in_hour;
  int remaining_seconds = seconds % seconds_in_hour;
  int minutes = remaining_seconds / 60;
  remaining_seconds = remaining_seconds % 60;
  return PrettifyTime(hours, minutes, remaining_seconds);
}