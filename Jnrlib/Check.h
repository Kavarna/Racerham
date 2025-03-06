#pragma once

#include <assert.h>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>

template <typename... Args> inline std::string Format(Args &&...args)
{
    if constexpr (sizeof...(Args) == 0)
        return "";

    std::ostringstream stream;

    (stream << ... << args);

    return stream.str();
}

template <typename... Args> inline void Print(Args &&...args)
{
    if constexpr (sizeof...(Args) == 0)
        return;

    (std::cout << ... << args);
    std::cout.flush();
}

template <typename... Args> inline void PrintLine(Args &&...args)
{
    if constexpr (sizeof...(Args) == 0)
        return;

    Print(std::forward<Args>(args)..., "\n");
}

template <typename... Args>
inline void Log(const char *prefix, const char *file, unsigned int line,
                const char *functionName, Args &&...args)
{
    if constexpr (sizeof...(Args) == 0)
        return;

    Print("[", prefix, "] ", file, ":", line, " (", functionName, ") => ");
    Print(std::forward<Args>(args)..., "\n");
}

constexpr const char *GetShortFileName(const char *str, uint32_t len)
{
    const char *result = str;
    for (uint32_t i = len - 1; i >= 0 && i < len; --i)
    {
        if (str[i] == '\\')
        {
            result = str + i + 1;
            break;
        }
    }
    return result;
}

constexpr const char *GetShortFunctionName(const char *str, uint32_t len)
{
    const char *result = str;
    for (uint32_t i = len - 1; i >= 0 && i < len; --i)
    {
        if (str[i] == ':')
        {
            result = str + i + 1;
            break;
        }
    }
    return result;
}

class JNRFailure : public std::exception
{
public:
    JNRFailure(std::string const &err) : mError(err)
    {
    }

    const char *what() const noexcept
    {
        return mError.c_str();
    }

private:
    std::string mError;
};

template <typename... Args> void ThrowIfFailed(bool expression, Args &&...args)
{
    if (!expression)
    {
        std::string message = Format(std::forward<Args>(args)...);
        throw JNRFailure(message);
    }
}

#define SHOWINFO(...)                                                          \
    {                                                                          \
        constexpr auto fileName =                                              \
            GetShortFileName(__FILE__, sizeof(__FILE__) - 1);                  \
        constexpr auto functionName =                                          \
            GetShortFunctionName(__FUNCTION__, sizeof(__FUNCTION__) - 1);      \
        Log("  LOG  ", fileName, __LINE__, functionName, __VA_ARGS__);         \
    }

#define SHOWWARNING(...)                                                       \
    {                                                                          \
        constexpr auto fileName =                                              \
            GetShortFileName(__FILE__, sizeof(__FILE__) - 1);                  \
        constexpr auto functionName =                                          \
            GetShortFunctionName(__FUNCTION__, sizeof(__FUNCTION__) - 1);      \
        Log("WARNING", fileName, __LINE__, functionName, __VA_ARGS__);         \
    }

#define SHOWERROR(...)                                                         \
    {                                                                          \
        constexpr auto fileName =                                              \
            GetShortFileName(__FILE__, sizeof(__FILE__) - 1);                  \
        constexpr auto functionName =                                          \
            GetShortFunctionName(__FUNCTION__, sizeof(__FUNCTION__) - 1);      \
        Log(" ERROR ", fileName, __LINE__, functionName, __VA_ARGS__);         \
    }

#define SHOWFATAL(...)                                                         \
    {                                                                          \
        constexpr auto fileName =                                              \
            GetShortFileName(__FILE__, sizeof(__FILE__) - 1);                  \
        constexpr auto functionName =                                          \
            GetShortFunctionName(__FUNCTION__, sizeof(__FUNCTION__) - 1);      \
        Log(" FATAL ", fileName, __LINE__, functionName, __VA_ARGS__);         \
    }

#define CHECK_SHOWINFO(cond, ...)                                              \
    if (!(cond))                                                               \
    {                                                                          \
        SHOWERROR(__VA_ARGS__);                                                \
    }

#define CHECK_FATAL(cond, ...)                                                 \
    if (!(cond))                                                               \
    {                                                                          \
        SHOWFATAL(__VA_ARGS__);                                                \
        assert(false);                                                         \
    }

#define CHECK(cond, retVal, ...)                                               \
    if (!(cond))                                                               \
    {                                                                          \
        SHOWERROR(__VA_ARGS__);                                                \
        return (retVal);                                                       \
    }

#define CHECKBRK(cond, ...)                                                    \
    if (!(cond))                                                               \
    {                                                                          \
        SHOWERROR(__VA_ARGS__);                                                \
        break;                                                                 \
    }

#define CHECKCNT(cond, ...)                                                    \
    if (!(cond))                                                               \
    {                                                                          \
        SHOWERROR(__VA_ARGS__);                                                \
        continue;                                                              \
    }

#if DEBUG || _DEBUG
#define DSHOWINFO(...)                                                         \
    {                                                                          \
        constexpr auto fileName =                                              \
            GetShortFileName(__FILE__, sizeof(__FILE__) - 1);                  \
        constexpr auto functionName =                                          \
            GetShortFunctionName(__FUNCTION__, sizeof(__FUNCTION__) - 1);      \
        Log("  LOG  ", fileName, __LINE__, functionName, __VA_ARGS__);         \
    }

#define DSHOWWARNING(...)                                                      \
    {                                                                          \
        constexpr auto fileName =                                              \
            GetShortFileName(__FILE__, sizeof(__FILE__) - 1);                  \
        constexpr auto functionName =                                          \
            GetShortFunctionName(__FUNCTION__, sizeof(__FUNCTION__) - 1);      \
        Log("WARNING", fileName, __LINE__, functionName, __VA_ARGS__);         \
    }

#define DSHOWERROR(...)                                                        \
    {                                                                          \
        constexpr auto fileName =                                              \
            GetShortFileName(__FILE__, sizeof(__FILE__) - 1);                  \
        constexpr auto functionName =                                          \
            GetShortFunctionName(__FUNCTION__, sizeof(__FUNCTION__) - 1);      \
        Log(" ERROR ", fileName, __LINE__, functionName, __VA_ARGS__);         \
    }

#define DSHOWFATAL(...)                                                        \
    {                                                                          \
        constexpr auto fileName =                                              \
            GetShortFileName(__FILE__, sizeof(__FILE__) - 1);                  \
        constexpr auto functionName =                                          \
            GetShortFunctionName(__FUNCTION__, sizeof(__FUNCTION__) - 1);      \
        Log(" FATAL ", fileName, __LINE__, functionName, __VA_ARGS__);         \
    }

#define DCHECKNR(cond, ...)                                                    \
    if (!(cond))                                                               \
    {                                                                          \
        SHOWERROR(__VA_ARGS__);                                                \
    }

#define DCHECK_FATAL(cond, ...)                                                \
    if (!(cond))                                                               \
    {                                                                          \
        SHOWFATAL(__VA_ARGS__);                                                \
        assert(false);                                                         \
    }

#define DCHECK(cond, retVal, ...)                                              \
    if (!(cond))                                                               \
    {                                                                          \
        SHOWERROR(__VA_ARGS__);                                                \
        return (retVal);                                                       \
    }

#else

#define DSHOWINFO(...)

#define DSHOWWARNING(...)

#define DSHOWERROR(...)

#define DSHOWFATAL(...)

#define DCHECKNR(cond, ...)

#define DCHECK_FATAL(cond, ...)

#define DCHECK(cond, retVal, ...)

#endif
