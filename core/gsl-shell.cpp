#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "core/welcome.h"
#include "core/LuaInterpreter.h"

const char *prompt = "> ";
const char *prompt_continuation = ". ";

char *add_to_multi_line(char *input, char *line) {
    int line_len = strlen(line);
    int existing_input_len = (input ? strlen(input) : 0);
    const char *separator = " ";
    int separator_len = 1;
    char *new_input = (char *) malloc(existing_input_len + line_len + separator_len + 1);
    char *new_input_pointer = new_input;
    if (input) {
        memcpy(new_input_pointer, input, existing_input_len);
        new_input_pointer += existing_input_len;
        memcpy(new_input_pointer, separator, separator_len);
        new_input_pointer += separator_len;
    }
    memcpy(new_input_pointer, line, line_len + 1);
    free(input);
    return new_input;
}

char *dispose_multi_line(char *input) {
    free(input);
    return nullptr;
}

int main() {
    LuaInterpreter intepreter{LuaLanguage::kLanguageExtension};
    auto init_status = intepreter.Initialize();
    if (init_status != Interpreter::Result::kSuccess) {
        fprintf(stderr, "Error initializing Lua\n");
        return 1;
    }
    fputs(kWelcomeMessage, stdout);
    fputs("\n\n", stdout);
    char *multi_line_input = nullptr;
    char *line_buffer = nullptr;
    size_t line_buffer_size = 0;
    while (true) {
        if (multi_line_input) {
            fputs(prompt_continuation, stdout);
        } else {
            fputs(prompt, stdout);
        }
        fflush(stdout);
        ssize_t getline_return_code = getline(&line_buffer, &line_buffer_size, stdin);
        if (getline_return_code < 0) {
            int getline_errno = errno;
            if (getline_errno != 0) {
                const char *getline_error_string = strerror(getline_errno);
                fprintf(stderr, "error reading line: %s\n", getline_error_string);
            } else {
                fputc('\n', stdout);
            }
            break;
        } else {
            ssize_t len = getline_return_code;
            if (len > 0 && line_buffer[len - 1] == '\n') {
                line_buffer[len - 1] = 0;
            }
            multi_line_input = add_to_multi_line(multi_line_input, line_buffer);
            auto execute_result = intepreter.Execute(multi_line_input);
            if (execute_result == Interpreter::Result::kError) {
                const char *error_message = intepreter.ErrorMessageString();
                fprintf(stderr, "Error reported on execution\n");
                fprintf(stderr, "%s\n", error_message);
            }
            if (execute_result != Interpreter::Result::kIncompleteInput) {
                multi_line_input = dispose_multi_line(multi_line_input);
            }
        }
    }
    intepreter.Close();
    return 0;
}
