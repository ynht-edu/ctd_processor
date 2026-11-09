#pragma once

#include <string>
#include <vector>

namespace ctd_processor {

struct Sample {
  double time_s;
  double conductivity_mScm;
  double temperature_degC;
  double pressure_dbar;
  double depth_instrument_m;
  double salinity_instrument;
  double sound_speed_instrument_mps;
  double density_instrument_kgm3;
};

struct Cast {
  std::string name;
  double latitude_deg;
  double longitude_deg;
  std::vector<Sample> samples;
};

struct ProcessedSample {
  double time_s;
  double pressure_dbar;
  double temperature_degC;
  double depth_m;
  double salinity;
  double density_kgm3;
  std::vector<double> refractive_index;
  bool in_water;
};

struct ProcessedCast {
  std::string name;
  double latitude_deg;
  double longitude_deg;
  std::vector<ProcessedSample> samples;
};

}  // namespace ctd
