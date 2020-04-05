#include <pthread.h>
#include <stdio.h>

#include "InterpreterThread.h"

#include "LuaInterpreter.h"

extern "C" void * luajit_eval_thread (void *userdata);

void * luajit_eval_thread (void *userdata) {
    InterpreterThread* eng = (InterpreterThread*) userdata;
    eng->Lock();
    eng->getInterpreter()->Initialize();
    eng->Run();
    pthread_exit(NULL);
    return NULL;
}

InterpreterThread::InterpreterThread():
    m_interpreter(std::make_unique<LuaInterpreter>()),
    m_status(Status::kStarting),
    m_request(Request::kNone) {
}

void InterpreterThread::Start() {
    pthread_attr_t attr[1];
    pthread_attr_init (attr);
    pthread_attr_setdetachstate (attr, PTHREAD_CREATE_DETACHED);
    if (pthread_create (&m_thread, attr, luajit_eval_thread, (void*)this)) {
        fprintf(stderr, "error creating thread");
    }
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
        m_eval.lock();
        m_status = Status::kWaiting;
        while (m_request == Request::kNone) {
            m_eval.wait();
        }
        Lock();
        BeforeEval();
        Command command = ProcessRequest();
        if (command == Command::kExit) {
            m_status = Status::kTerminated;
            m_eval.unlock();
            break;
        }
        if (command == Command::kExecute) {
            line = m_line_pending;
        }
        m_status = Status::kBusy;
        m_eval.unlock();
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
    m_eval.lock();
    m_request = request;
    if (line) {
        m_line_pending = std::string{line};
    }
    if (m_status == Status::kWaiting) {
        m_eval.signal();
    }
    m_eval.unlock();
    sched_yield();
}

void
InterpreterThread::InterruptRequest()
{
    m_eval.lock();
    if (m_status == Status::kBusy) {
        m_interpreter->Interrupt();
    }
    m_eval.unlock();
}
