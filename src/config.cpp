#include "ctd_processing/config.hpp"

#include <stdexcept>

#include <yaml-cpp/yaml.h>

namespace ctd {

namespace {

YAML::Node find_node(const YAML::Node & node, const std::string & key) {
  const auto dot = key.find('.');
  const YAML::Node child = node[key.substr(0, dot)];
  if (!child) {
    throw std::runtime_error("missing parameter: " + key);
  }
  return dot == std::string::npos ? child : find_node(child, key.substr(dot + 1));
}

template <typename T>
T get(const YAML::Node & params, const std::string & key) {
  return find_node(params, key).as<T>();
}

YAML::Node parameters_of(const YAML::Node & root) {
  for (auto it = root.begin(); it != root.end(); ++it) {
    const YAML::Node ros = it->second["ros__parameters"];
    if (ros) {
      return ros;
    }
  }
  return root;
}

}  // namespace

Config load_config(const std::string & path) {
  const YAML::Node params = parameters_of(YAML::LoadFile(path));

  Config c;
  c.input.directory = get<std::string>(params, "input.directory");
  c.input.extension = get<std::string>(params, "input.extension");
  c.input.deduplicate_by_basename = get<bool>(params, "input.deduplicate_by_basename");
  c.output.directory = get<std::string>(params, "output.directory");
  c.output.write_samples_csv = get<bool>(params, "output.write_samples_csv");
  c.qc.conductivity_min_mScm = get<double>(params, "qc.conductivity_min_mScm");
  c.qc.pressure_min_dbar = get<double>(params, "qc.pressure_min_dbar");
  c.qc.settle_time_s = get<double>(params, "qc.settle_time_s");
  c.wavelengths.names = get<std::vector<std::string>>(params, "wavelengths.names");
  c.wavelengths.nm = get<std::vector<double>>(params, "wavelengths.nm");
  c.refraction.apply_pressure_correction = get<bool>(params, "refraction.apply_pressure_correction");

  if (c.wavelengths.names.size() != c.wavelengths.nm.size()) {
    throw std::runtime_error("wavelengths.names and wavelengths.nm differ in length");
  }
  return c;
}

}  // namespace ctd
