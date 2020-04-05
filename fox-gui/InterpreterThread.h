#ifndef FOXGUI_LUA_ENGINE_H
#define FOXGUI_LUA_ENGINE_H

#include <string>
#include <memory>

extern "C" {
#include "lua.h"
}

#include "pthreadpp.h"
#include "Interpreter.h"

class InterpreterThread {
public:
    enum class Status { kStarting, kWaiting, kBusy, kTerminated };
    enum class Request { kNone, kExit, kRestart, kExecute };
    enum { kEotCharacter = 0x04 };

    InterpreterThread();
    virtual ~InterpreterThread() { }

    void SetRequest(Request request, const char* line = nullptr);
    void Start();
    void Run();

    virtual void BeforeEval() { }
    virtual void RestartCallback() { }
    virtual void QuitCallback() { }

    void Lock() {
        pthread_mutex_lock(&m_exec_mutex);
    }

    void Unlock() {
        pthread_mutex_unlock(&m_exec_mutex);
    }

    Interpreter *getInterpreter() {
        return m_interpreter.get();
    }

    Interpreter::Result EvalStatus() const {
        return m_eval_status;
    }

    pthread::mutex& EvalMutex() {
        return m_eval;
    }

    // asyncronous request
    void InterruptRequest();

private:
    enum class Command { kExecute, kExit, kContinue };

    Command ProcessRequest();

    std::unique_ptr<Interpreter> m_interpreter;
    pthread_t m_thread;
    pthread_mutex_t m_exec_mutex;
    Status m_status;
    pthread::cond m_eval;
    std::string m_line_pending;
    Interpreter::Result m_eval_status;
    Request m_request;
};

#endif
