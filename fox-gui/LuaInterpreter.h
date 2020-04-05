#ifndef FOX_GUI_LUA_INTERPRETER_H_
#define FOX_GUI_LUA_INTERPRETER_H_

#include <string>

extern "C" {
#include <lua.h>
}

#include "defs.h"
#include "Interpreter.h"

class LuaInterpreter : public Interpreter {
public:
    LuaInterpreter() : m_lua_state(nullptr) {  }

    void Initialize() override;
    void Close() override;
    Result Execute(const char* line) override;
    void Interrupt() override;

    const char* ErrorMessageString() const override {
        return m_error_message.c_str();
    }
private:
    void LuaErrorStoreMessage(int result);

    lua_State *m_lua_state;
    std::string m_error_message;
};

#endif
