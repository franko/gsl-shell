#include <stdio.h>

#include "InterpreterThread.h"

static void StartInterpreterThread(InterpreterThread *eng) {
    eng->Lock();
    eng->getInterpreter()->Initialize();
    eng->Run();
}

InterpreterThread::InterpreterThread(std::unique_ptr<Interpreter> interpreter):
    interpreter_(std::move(interpreter)),
    status_(Status::kStarting),
    request_(Request::kNone) {
}

void InterpreterThread::Start() {
    thread_ = std::make_unique<std::thread>(StartInterpreterThread, this);
    thread_->detach();
}

InterpreterThread::Command InterpreterThread::ProcessRequest() {
    Command command;
    switch (request_) {
    case Request::kExit:
        command = Command::kExit;
        break;
    case Request::kRestart:
        interpreter_->Close();
        interpreter_->Initialize();
        RestartCallback();
        command = Command::kContinue;
        break;
    case Request::kExecute:
        command = Command::kExecute;
        break;
    case Request::kNone:
        command = Command::kContinue;
    }
    request_ = Request::kNone;
    return command;
}

void InterpreterThread::Run() {
    std::string line;
    while (true) {
        Unlock();
        std::unique_lock<std::mutex> request_lock(request_mutex_);
        status_ = Status::kWaiting;
        request_condition_.wait(request_lock,
            [this] { return request_ != Request::kNone; }
        );
        Lock();
        BeforeEval();
        Command command = ProcessRequest();
        if (command == Command::kExit) {
            status_ = Status::kTerminated;
            break;
        }
        if (command == Command::kExecute) {
            line = line_pending_;
        }
        status_ = Status::kBusy;
        request_lock.unlock();
        if (command == Command::kExecute) {
            eval_status_ = interpreter_->Execute(line.c_str());
            fputc(kEotCharacter, stdout);
            fflush(stdout);
        }
    }
    interpreter_->Close();
    Unlock();
    QuitCallback();
}

void InterpreterThread::SetRequest(InterpreterThread::Request request, const char* line) {
    request_mutex_.lock();
    request_ = request;
    if (line) {
        line_pending_ = std::string{line};
    }
    if (status_ == Status::kWaiting) {
        request_mutex_.unlock();
        request_condition_.notify_one();
    } else {
        request_mutex_.unlock();
    }
    std::this_thread::yield();
}

void
InterpreterThread::InterruptRequest() {
    request_mutex_.lock();
    if (status_ == Status::kBusy) {
        interpreter_->Interrupt();
    }
    request_mutex_.unlock();
}
