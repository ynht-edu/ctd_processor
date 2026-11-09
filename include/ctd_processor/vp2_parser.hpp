#pragma once

#include <string>
#include <vector>

#include "ctd_processor/config.hpp"
#include "ctd_processor/types.hpp"

namespace ctd_processor {

Cast read_vp2(const std::string& path);

std::vector<std::string> find_files(const InputConfig& input);

}  // namespace ctd
