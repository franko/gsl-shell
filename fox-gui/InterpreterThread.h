#ifndef FOXGUI_LUA_ENGINE_H
#define FOXGUI_LUA_ENGINE_H

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

extern "C" {
#include "lua.h"
}

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
        m_exec_mutex.lock();
    }

    void Unlock() {
        m_exec_mutex.unlock();
    }

    Interpreter *getInterpreter() {
        return m_interpreter.get();
    }

    Interpreter::Result EvalStatus() const {
        return m_eval_status;
    }

    // asyncronous request
    void InterruptRequest();

private:
    enum class Command { kExecute, kExit, kContinue };

    Command ProcessRequest();

    std::unique_ptr<Interpreter> m_interpreter;
    std::unique_ptr<std::thread> m_thread;
    std::mutex m_exec_mutex;
    Status m_status;
    std::mutex m_request_mutex;
    std::condition_variable m_request_condition;
    std::string m_line_pending;
    Interpreter::Result m_eval_status;
    Request m_request;
};

#endif
