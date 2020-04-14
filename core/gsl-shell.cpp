#include <iostream>
#include <string>

#include "core/welcome.h"
#include "core/LuaInterpreter.h"

const char *prompt = "> ";
const char *prompt_continuation = ". ";

int main() {
    LuaInterpreter intepreter{LuaLanguage::kLanguageExtension};
    auto init_status = intepreter.Initialize();
    if (init_status != Interpreter::Result::kSuccess) {
        std::cerr << "Error initializing Lua" << std::endl;
        return 1;
    }
    std::cout << kWelcomeMessage << std::endl << std::endl;
    std::string multi_line;
    bool multi_line_continued = false;
    while (!std::cin.bad() && !std::cin.eof()) {
        std::cout << (multi_line_continued ? prompt_continuation : prompt);
        std::cout.flush();
        std::string line;
        std::getline(std::cin, line);
        if (multi_line_continued) {
            multi_line += " " + line;
        } else {
            multi_line = line;
        }
        if (multi_line.length() > 0) {
            auto execute_result = intepreter.Execute(multi_line.c_str());
            if (execute_result == Interpreter::Result::kError) {
                std::cout << intepreter.ErrorMessageString() << std::endl;
            }
            multi_line_continued = (execute_result == Interpreter::Result::kIncompleteInput);
        }
        if (std::cin.bad()) {
            std::cerr << "error reading input" << std::endl;
        }
    }
    intepreter.Close();
    return 0;
}
