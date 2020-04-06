#ifndef FOX_GUI_LUA_INTERPRETER_H_
#define FOX_GUI_LUA_INTERPRETER_H_

#include <string>

extern "C" {
#include <lua.h>
}

#include "defs.h"
#include "Interpreter.h"

enum class LuaLanguage {
    kStandard,
    kLanguageExtension,
};

class LuaInterpreter : public Interpreter {
public:
    LuaInterpreter(LuaLanguage language):
        lua_state_(nullptr), lua_language_(language) {
    }

    Result Initialize() override;
    void Close() override;
    Result Execute(const char* line) override;
    void Interrupt() override;

    // Can be overriden by derived class to load custom code.
    virtual Result CustomLoader(lua_State *L) {
        return Result::kSuccess;
    }

    const char* ErrorMessageString() const override {
        return error_message_.c_str();
    }
private:
    void LuaErrorStoreMessage(int result);

    lua_State *lua_state_;
    const LuaLanguage lua_language_;
    std::string error_message_;
};

#endif
