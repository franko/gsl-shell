#ifndef CORE_INTERPRETER_H_
#define CORE_INTERPRETER_H_

class Interpreter {
public:
    enum class Result {
        kSuccess,
        kError,
        kIncompleteInput,
    };
    virtual ~Interpreter() { }
    virtual Result Initialize() = 0;
    virtual void Close() = 0;
    virtual Result Execute(const char* line) = 0;
    virtual void Interrupt() = 0;
    virtual const char* ErrorMessageString() const = 0;
};

#endif
