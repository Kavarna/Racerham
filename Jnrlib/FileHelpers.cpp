#include "FileHelpers.h"
#include "Check.h"
#include "Jnrlib/Exceptions.h"
#include "Jnrlib/Singletone.h"

#include <filesystem>
#include <fstream>

class KnownDirectoriesStorage
    : public Jnrlib::ISingletone<KnownDirectoriesStorage>
{
    MAKE_SINGLETONE_CAPABLE(KnownDirectoriesStorage);

private:
    KnownDirectoriesStorage()
    {
        m_knownDirectories.push_back(".");
    }

public:
    void RegisterDirectory(std::string_view dir)
    {
        m_knownDirectories.push_back(std::filesystem::absolute(dir));
    }

    std::filesystem::path ResolveFilePath(std::string const &givenPath)
    {
        using namespace std::filesystem;
        for (const auto &dir : m_knownDirectories)
        {
            path currentPath(dir);
            currentPath /= givenPath;
            if (exists(currentPath))
            {
                return currentPath;
            }
        }

        throw Jnrlib::Exceptions::CannotResolveSymbol(givenPath);
    }

private:
    std::vector<std::filesystem::path> m_knownDirectories;
};

namespace Jnrlib
{
void RegisterDirectory(std::string_view dir)
{
    KnownDirectoriesStorage::Get()->RegisterDirectory(dir);
}
std::vector<char> ReadWholeFile(std::string const &givenPath, bool assertIfFail)
{
    std::filesystem::path path =
        KnownDirectoriesStorage::Get()->ResolveFilePath(givenPath);
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
