#include "FileHelpers.h"
#include "Check.h"

#include <filesystem>
#include <fstream>

namespace Jnrlib
{
std::vector<char> ReadWholeFile(std::string const &given_path,
                                bool assertIfFail)
{
    std::filesystem::path path(std::filesystem::absolute(given_path));
    std::ifstream file(path.c_str(), std::ios::binary | std::ios::ate);
    if (assertIfFail)
    {
        ThrowIfFailed(file.is_open(), "Unable to open file ", path,
                      " for reading");
    }
    else
    {
        CHECK(file.is_open(), std::vector<char>{}, "Unable to open file ", path,
              " for reading");
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (assertIfFail)
    {
        ThrowIfFailed(file.read(buffer.data(), size).good(),
                      "Unable to read file ", path, " after opening it");
    }
    else
    {
        CHECK(file.read(buffer.data(), size).good(), std::vector<char>{},
              "Unable to read file ", path, " after opening it");
    }

    return buffer;
}
void DumpWholeFile(std::string const &path,
                   std::vector<unsigned char> const &data, bool assertIfFail)
{
    std::ofstream file(path.c_str(), std::ios::binary);
    if (assertIfFail)
    {
        ThrowIfFailed(file.is_open(), "Unable to open file ", path,
                      " for writing");
    }
    else
    {
        SHOWERROR("Unable to write to open file ", path, " for writing");
        return;
    }

    file.write((const char *)data.data(), data.size());
}
} // namespace Jnrlib
