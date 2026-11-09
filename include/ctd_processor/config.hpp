#pragma once

#include <string>
#include <vector>

namespace ctd_processor {

struct InputConfig {
  std::string directory;
  std::string extension;
  bool deduplicate_by_basename;
};

struct OutputConfig {
  std::string directory;
  bool write_samples_csv;
};

struct QcConfig {
  double conductivity_min_mScm;
  double pressure_min_dbar;
  double settle_time_s;
};

struct WavelengthConfig {
  std::vector<std::string> names;
  std::vector<double> nm;
};

struct RefractionConfig {
  bool apply_pressure_correction;
};

struct Config {
  InputConfig input;
  OutputConfig output;
  QcConfig qc;
  WavelengthConfig wavelengths;
  RefractionConfig refraction;
};

Config load_config(const std::string& path);

}  // namespace ctd
