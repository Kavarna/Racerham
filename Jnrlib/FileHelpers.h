#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Jnrlib
{
std::vector<char> ReadWholeFile(std::string const &path, bool assertIfFail = true);
void DumpWholeFile(std::string const &path, std::vector<unsigned char> const &data, bool assertIfFail = true);
} // namespace Jnrlib
