#ifndef FOX_GUI_FOX_INTERPRETER_THREAD_H_
#define FOX_GUI_FOX_INTERPRETER_THREAD_H_

#include <memory>

#include "fx.h"

#include "InterpreterThread.h"

class GslShellApp;

class FoxInterpreterThread : public InterpreterThread
{
public:
    FoxInterpreterThread(GslShellApp* app);
    ~FoxInterpreterThread();

    void RestartCallback() override;
    void QuitCallback() override;

    void SetClosingSignal(FXGUISignal* s) {
        close_ = s;
    }

private:
    GslShellApp* app_;
    FXGUISignal* close_;
};

#endif
