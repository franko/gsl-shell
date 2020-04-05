#ifndef FOXGUI_INTERPRETER_THREAD_H_
#define FOXGUI_INTERPRETER_THREAD_H_

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

extern "C" {
#include "lua.h"
}

#include "Interpreter.h"

/* This class is responsible to start and run an interpreter in a
 * separate thread.
 * It provides the methods to issue a request to the interpreter
 * and to interrupt it.
 */
class InterpreterThread {
public:
    enum class Status { kStarting, kWaiting, kBusy, kTerminated };
    enum class Request { kNone, kExit, kRestart, kExecute };
    enum { kEotCharacter = 0x04 };

    InterpreterThread(std::unique_ptr<Interpreter> interpreter);
    virtual ~InterpreterThread() { }

    void SetRequest(Request request, const char* line = nullptr);
    void Start();
    void Run();

    virtual void BeforeEval() { }
    virtual void RestartCallback() { }
    virtual void QuitCallback() { }

    void Lock() {
        exec_mutex_.lock();
    }

    void Unlock() {
        exec_mutex_.unlock();
    }

    Interpreter *getInterpreter() {
        return interpreter_.get();
    }

    Interpreter::Result EvalStatus() const {
        return eval_status_;
    }

    // asyncronous request
    void InterruptRequest();

private:
    enum class Command { kExecute, kExit, kContinue };

    Command ProcessRequest();

    std::unique_ptr<Interpreter> interpreter_;
    std::unique_ptr<std::thread> thread_;
    std::mutex exec_mutex_;
    Status status_;
    std::mutex request_mutex_;
    std::condition_variable request_condition_;
    std::string line_pending_;
    Interpreter::Result eval_status_;
    Request request_;
};

#endif
