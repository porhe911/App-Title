// SPDX-License-Identifier: GPL-3.0-or-later
// Independent write allowlists for the optional user-supplied firmware fixtures.
// Numeric metadata only; firmware contents are not included.
#pragma once
#include <array>
#include <cstddef>
namespace app_title_test {
struct HermesFixtureProfile {
    std::size_t plain_size;
    std::array<unsigned,7> functions;
    std::array<std::size_t,7> offsets,lengths;
};
inline constexpr HermesFixtureProfile hermes1160{
    1780936,{1461,6665,6513,6509,282,6448,1079},
    {0xe3d20,0x15b120,0x15654a,0x156394,0xcbf25,0x153de8,0xdaff4},
    {183,622,78,266,402,1747,361}};
inline constexpr HermesFixtureProfile hermes1360{
    1795460,{1473,6703,6549,6545,282,6484,1079},
    {0xe5e75,0x15d93f,0x158d12,0x158b5c,0xcdd83,0x1565b0,0xdce6f},
    {199,622,78,266,402,1747,376}};
}
