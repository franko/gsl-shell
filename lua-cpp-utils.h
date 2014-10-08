#ifndef LUA_CPP_UTILS_H
#define LUA_CPP_UTILS_H

#include <new>

#include "defs.h"
#include "lua-defs.h"
__BEGIN_DECLS
#include "lua.h"
__END_DECLS

#include "gs-types.h"

namespace gslshell {

class ret_status {
    bool m_success;
    const char *m_error_msg;
    const char *m_context;

public:
    ret_status() : m_success(true) {};

    void success() {
        m_success = true;
    }
    void error(const char *msg, const char *ctx)
    {
        m_success = false;
        m_error_msg = msg;
        m_context = ctx;
    };

    const char *error_msg() const {
        return (m_success ? 0 : m_error_msg);
    };
    const char *context()   const {
        return (m_success ? 0 : m_context);
    };
};
}

inline void* operator new(size_t nbytes, lua_State *L, enum gs_type_e tp)
{
    void* p = lua_newuserdata(L, nbytes);
    gs_set_metatable (L, tp);
    return p;
}

template <class T>
T* push_new_object (lua_State *L, enum gs_type_e tp)
{
    return new(L, tp) T();
}

template <class T, class init_type>
T* push_new_object (lua_State *L, enum gs_type_e tp, init_type& init)
{
    return new(L, tp) T(init);
}

template <class T>
int object_free (lua_State *L, int index, enum gs_type_e tp)
{
    T *obj = (T *) gs_check_userdata (L, index, tp);
    obj->~T();
    return 0;
}

template <class T>
T* object_check (lua_State *L, int index, enum gs_type_e tp)
{
    return (T *) gs_check_userdata (L, index, tp);
}

template <class T>
T* object_cast (lua_State *L, int index, enum gs_type_e tp)
{
    return (T *) gs_is_userdata (L, index, tp);
}

#endif
