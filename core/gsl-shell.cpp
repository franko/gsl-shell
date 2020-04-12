#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "core/welcome.h"
#include "core/LuaInterpreter.h"

const char *prompt = "> ";

int main() {
	LuaInterpreter intepreter{LuaLanguage::kLanguageExtension};
	auto init_status = intepreter.Initialize();
	if (init_status != Interpreter::Result::kSuccess) {
		fprintf(stderr, "Error initializing Lua\n");
		return 1;
	}
	fputs(kWelcomeMessage, stdout);
	fputs("\n\n", stdout);
	char *line_buffer = nullptr;
	size_t line_buffer_size = 0;
	while (true) {
		fputs(prompt, stdout);
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
			auto execute_result = intepreter.Execute(line_buffer);
			if (execute_result == Interpreter::Result::kError) {
				const char *error_message = intepreter.ErrorMessageString();
				fprintf(stderr, "Error reported on execution\n");
				fprintf(stderr, "%s\n", error_message);
			} else if (execute_result == Interpreter::Result::kIncompleteInput) {
				fprintf(stderr, "incomplete input NYI\n");
			}
		}
	}
	intepreter.Close();
	return 0;
}
