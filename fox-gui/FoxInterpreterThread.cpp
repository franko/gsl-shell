#include "FoxInterpreterThread.h"

#include "GslShellApp.h"
#include "LuaInterpreter.h"

FoxInterpreterThread::FoxInterpreterThread(GslShellApp* app):
    InterpreterThread(std::make_unique<LuaInterpreter>(LuaLanguage::kLanguageExtension)),
    app_(app), close_(nullptr) { }

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
