// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "app_title.hpp"

namespace app_title {
enum class HomeStyleResult : unsigned { ignored=0, applied=1, unsupported=2, invalid=3 };
// Only the in-memory, driver-decrypted HomeUI buffer is modified. A known
// whole-payload digest and module/function metadata must all agree. Supports
// the exact 4.03 JavaScript RAM profile and the verified 11.60 Hermes profile.
HomeStyleResult patch_home_style(void* bundle,std::size_t size,const Config& config,StyleInput* observed=nullptr);
// Exposed for bytecode/VM verification; generated independently of Sony code.
std::size_t build_title_style(std::uint8_t* output,std::size_t capacity,const Config& config);
void sha1_digest(const void* data,std::size_t size,std::uint8_t output[20]);
}

