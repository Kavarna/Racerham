#pragma once

#include "Exceptions.h"
#include "TypeHelpers.h"
#include "TypeMatching.h"
#include <memory>
#include <mutex>
#include <utility>

namespace Jnrlib
{
template <typename type> class ISingletone
{
protected:
    ISingletone() {};
    virtual ~ISingletone()
    {
        Destroy();
    };

public:
    template <typename... Args> constexpr static type *Get(Args &&...args)
    {
        if constexpr (sizeof...(Args) == 0)
        {
            if constexpr (IsDefaultConstructible<type>::value)
            {
                std::call_once(m_singletoneFlags, [&] {
                    m_singletoneInstance =
                        new type(std::forward<Args>(args)...);
                });
                return m_singletoneInstance;
            }
            else
            {
                if (!m_singletoneInstance)
                {
                    throw Exceptions::SingletoneNotCreated();
                }
                return m_singletoneInstance;
            }
        }
        else
        {
            if (m_singletoneInstance)
            {
                throw Exceptions::SingletoneNotUniqueAttempt();
            }
            std::call_once(m_singletoneFlags, [&] {
                m_singletoneInstance = new type(std::forward<Args>(args)...);
            });
            return m_singletoneInstance;
        }
    }

    static void Destroy()
    {
        if (m_singletoneInstance)
        {
            auto ptr =
                m_singletoneInstance; // If we do it this way, we can safely
                                      // call reset() from destructor too
            m_singletoneInstance = nullptr;
            delete ptr;
        }
    };

private:
    static type *m_singletoneInstance;
    static std::once_flag m_singletoneFlags;
};
} // namespace Jnrlib

#define MAKE_SINGLETONE_CAPABLE(name)                                          \
    friend class Jnrlib::ISingletone<name>;                                    \
    friend struct IsDefaultConstructible<name>;

template <typename type>
type *Jnrlib::ISingletone<type>::m_singletoneInstance = nullptr;
template <typename type>
std::once_flag Jnrlib::ISingletone<type>::m_singletoneFlags;