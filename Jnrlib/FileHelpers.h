#pragma once

#include <string>
#include <vector>

namespace Jnrlib
{
void RegisterDirectory(std::string_view dir);
std::vector<char> ReadWholeFile(std::string const &path,
                                bool assertIfFail = true);
void DumpWholeFile(std::string const &path,
                   std::vector<unsigned char> const &data,
                   bool assertIfFail = true);
} // namespace Jnrlib
