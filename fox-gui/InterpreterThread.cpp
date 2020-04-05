#include <stdio.h>

#include "InterpreterThread.h"

#include "LuaInterpreter.h"

static void StartInterpreterThread(InterpreterThread *eng) {
    eng->Lock();
    eng->getInterpreter()->Initialize();
    eng->Run();
}

InterpreterThread::InterpreterThread():
    m_interpreter(std::make_unique<LuaInterpreter>()),
    m_status(Status::kStarting),
    m_request(Request::kNone) {
}

void InterpreterThread::Start() {
    m_thread = std::make_unique<std::thread>(StartInterpreterThread, this);
    m_thread->detach();
}

InterpreterThread::Command InterpreterThread::ProcessRequest() {
    Command command;
    switch (m_request) {
    case Request::kExit:
        command = Command::kExit;
        break;
    case Request::kRestart:
        m_interpreter->Close();
        m_interpreter->Initialize();
        RestartCallback();
        command = Command::kContinue;
        break;
    case Request::kExecute:
        command = Command::kExecute;
        break;
    case Request::kNone:
        command = Command::kContinue;
    }
    m_request = Request::kNone;
    return command;
}

void InterpreterThread::Run() {
    std::string line;
    while (true) {
        Unlock();
        std::unique_lock<std::mutex> request_lock(m_request_mutex);
        m_status = Status::kWaiting;
        m_request_condition.wait(request_lock,
            [this] { return m_request != Request::kNone; }
        );
        Lock();
        BeforeEval();
        Command command = ProcessRequest();
        if (command == Command::kExit) {
            m_status = Status::kTerminated;
            break;
        }
        if (command == Command::kExecute) {
            line = m_line_pending;
        }
        m_status = Status::kBusy;
        request_lock.unlock();
        if (command == Command::kExecute) {
            m_eval_status = m_interpreter->Execute(line.c_str());
            fputc(kEotCharacter, stdout);
            fflush(stdout);
        }
    }
    m_interpreter->Close();
    Unlock();
    QuitCallback();
}

void InterpreterThread::SetRequest(InterpreterThread::Request request, const char* line) {
    m_request_mutex.lock();
    m_request = request;
    if (line) {
        m_line_pending = std::string{line};
    }
    if (m_status == Status::kWaiting) {
        m_request_mutex.unlock();
        m_request_condition.notify_one();
    } else {
        m_request_mutex.unlock();
    }
    std::this_thread::yield();
}

void
InterpreterThread::InterruptRequest() {
    m_request_mutex.lock();
    if (m_status == Status::kBusy) {
        m_interpreter->Interrupt();
    }
    m_request_mutex.unlock();
}
