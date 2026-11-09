#include "ctd_processor/vp2_parser.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>

namespace ctd_processor {

namespace {

namespace fs = std::filesystem;

std::vector<std::string> split_tabs(const std::string & line) {
  std::vector<std::string> fields;
  std::size_t start = 0;
  while (true) {
    const auto tab = line.find('\t', start);
    fields.push_back(line.substr(start, tab == std::string::npos ? tab : tab - start));
    if (tab == std::string::npos) {
      break;
    }
    start = tab + 1;
  }
  return fields;
}

double to_double(const std::string & text) {
  char * end = nullptr;
  const double value = std::strtod(text.c_str(), &end);
  return end == text.c_str() ? std::numeric_limits<double>::quiet_NaN() : value;
}

long days_from_civil(int y, int m, int d) {
  y -= m <= 2;
  const long era = (y >= 0 ? y : y - 399) / 400;
  const long yoe = y - era * 400;
  const long doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  const long doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + doe - 719468;
}

double parse_timestamp_s(const std::string & text) {
  int y, mo, d, h, mi;
  double s;
  if (std::sscanf(text.c_str(), "%d/%d/%d %d:%d:%lf", &y, &mo, &d, &h, &mi, &s) != 6) {
    throw std::runtime_error("bad timestamp: " + text);
  }
  return days_from_civil(y, mo, d) * 86400.0 + h * 3600.0 + mi * 60.0 + s;
}

std::string strip_cr(std::string line) {
  if (!line.empty() && line.back() == '\r') {
    line.pop_back();
  }
  return line;
}

}  // namespace

Cast read_vp2(const std::string & path) {
  std::ifstream in(path);
  if (!in) {
    throw std::runtime_error("cannot open " + path);
  }

  std::map<std::string, std::string> header;
  std::string line, section;
  while (std::getline(in, line)) {
    line = strip_cr(line);
    if (line.size() > 1 && line.front() == '[' && line.back() == ']') {
      section = line.substr(1, line.size() - 2);
      if (section == "DATA") {
        break;
      }
      continue;
    }
    const auto eq = line.find('=');
    if (section == "HEADER" && eq != std::string::npos) {
      header[line.substr(0, eq)] = line.substr(eq + 1);
    }
  }

  std::string names_line, units_line;
  std::getline(in, names_line);
  std::getline(in, units_line);
  const auto names = split_tabs(strip_cr(names_line));
  const auto column = [&](const std::string & name) {
    const auto it = std::find(names.begin(), names.end(), name);
    if (it == names.end()) {
      throw std::runtime_error(path + ": missing column " + name);
    }
    return static_cast<std::size_t>(it - names.begin());
  };
  const auto c_time = column("Date/Time");
  const auto c_depth = column("Depth");
  const auto c_pressure = column("Pressure");
  const auto c_temperature = column("Temperature");
  const auto c_conductivity = column("Conductivity");
  const auto c_salinity = column("Salinity");
  const auto c_sound = column("Sound Velocity");
  const auto c_density = column("Density");

  Cast cast;
  cast.name = fs::path(path).filename().string();
  cast.latitude_deg = to_double(header.at("Latitude"));
  cast.longitude_deg = to_double(header.at("Longitude"));

  double t_first = 0.0;
  while (std::getline(in, line)) {
    line = strip_cr(line);
    if (line.find_first_not_of(" \t") == std::string::npos) {
      continue;
    }
    const auto f = split_tabs(line);
    if (f.size() < names.size()) {
      continue;
    }
    const double t = parse_timestamp_s(f[c_time]);
    if (cast.samples.empty()) {
      t_first = t;
    }
    cast.samples.push_back({t - t_first, to_double(f[c_conductivity]), to_double(f[c_temperature]),
                            to_double(f[c_pressure]), to_double(f[c_depth]),
                            to_double(f[c_salinity]), to_double(f[c_sound]),
                            to_double(f[c_density])});
  }
  return cast;
}

std::vector<std::string> find_files(const InputConfig & input) {
  std::vector<std::string> paths;
  for (const auto & entry : fs::recursive_directory_iterator(input.directory)) {
    if (entry.is_regular_file() && entry.path().extension() == input.extension) {
      paths.push_back(entry.path().string());
    }
  }
  std::sort(paths.begin(), paths.end());

  if (!input.deduplicate_by_basename) {
    return paths;
  }
  std::set<std::string> seen;
  std::vector<std::string> unique;
  for (const auto & p : paths) {
    if (seen.insert(fs::path(p).filename().string()).second) {
      unique.push_back(p);
    }
  }
  return unique;
}

}  // namespace ctd
