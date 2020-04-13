#include "FoxInterpreterThread.h"

#include "elem/elem_lua.h"
#include "GslShellApp.h"
#include "core/LuaInterpreter.h"

class ElemPlotLuaInterpreter : public LuaInterpreter {
public:
    using LuaInterpreter::LuaInterpreter;

    Result CustomLoader(lua_State *L) override {
        elem::LuaOpenLibrary(L);
        return Result::kSuccess;
    }
};

FoxInterpreterThread::FoxInterpreterThread(GslShellApp* app):
    InterpreterThread(std::make_unique<ElemPlotLuaInterpreter>(LuaLanguage::kLanguageExtension)),
    app_(app), close_(nullptr) {
}

FoxInterpreterThread::~FoxInterpreterThread() {
    delete close_;
}

void FoxInterpreterThread::QuitCallback() {
    if (close_) {
        close_->signal();
    }
}

void FoxInterpreterThread::RestartCallback() {
    app_->lock();
    app_->reset_console_request();
    app_->wait_action();
    app_->unlock();
}
