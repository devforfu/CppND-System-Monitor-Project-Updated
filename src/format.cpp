#include "format.h"

#include <iomanip>
#include <sstream>
#include <string>

using std::string;

// Formats measuring seconds as human-readable HH:MM:SS string
string Format::ElapsedTime(long seconds) {
  int secondsPartialMinute = seconds % 60;
  int minutesPartialHour = (seconds / 60) % 60;
  int hours = seconds / 3600;
  std::stringstream s;
  s << std::setfill('0') << std::setw(2) << hours;
  s << ":" << std::setfill('0') << std::setw(2) << minutesPartialHour;
  s << ":" << std::setfill('0') << std::setw(2) << secondsPartialMinute;
  return s.str();
}
