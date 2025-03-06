#pragma once

#include <cstring>
#include <functional>
#include <ostream>
#include <vector>

#include "BasicTypes.h"

template <typename T>
std::ostream &operator<<(std::ostream &stream, std::vector<T> const &vct)
{
    if (vct.size() == 0)
    {
        stream << "[]";
    }
    stream << vct.size() << " : [ ";
    for (u32 i = 0; i < vct.size() - 1; ++i)
    {
        if constexpr (std::is_same_v<T, std::string> ||
                      std::is_same_v<T, char *> ||
                      std::is_same_v<T, const char *>)
        {
            stream << "\"" << vct[i] << "\", ";
        }
        else
        {
            stream << vct[i] << ", ";
        }
    }
    if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, char *> ||
                  std::is_same_v<T, const char *>)
    {
        stream << "\"" << vct[vct.size() - 1] << "\" ]";
    }
    else
    {
        stream << vct[vct.size() - 1] << " ]";
    }

    return stream;
}

#ifndef ARRAYSIZE
#define ARRAYSIZE(var) sizeof((var)) / sizeof((var[0]))
#endif

namespace Jnrlib
{

template <typename T> T clamp(T value, T low, T high)
{
    if (value < low)
        return low;
    if (value > high)
        return high;
    return value;
}

class DefferCall
{
public:
    DefferCall(const DefferCall &that) = delete;
    DefferCall &operator=(const DefferCall &that) = delete;

    DefferCall(std::function<void()> &&f) : mFunc(f)
    {
    }

    ~DefferCall()
    {
        Execute();
    }

    void Execute()
    {
        if (mFunc)
        {
            mFunc();
        }
    }

    void Reset()
    {
        mFunc = nullptr;
    }

private:
    std::function<void()> mFunc;
};

inline bool iequals(std::string const left, std::string const right)
{
    if (left.size() != right.size())
        return false;

    for (unsigned int i = 0; i < left.size(); ++i)
    {
        if (::tolower(left[i]) != ::tolower(right[i]))
        {
            return false;
        }
    }

    return true;
}

inline bool contains(std::string_view const toSearch,
                     std::string_view const pattern)
{
    return toSearch.find(pattern) != std::string_view::npos;
}

} // namespace Jnrlib
