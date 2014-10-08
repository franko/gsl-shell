#ifndef AGGPLOT_PRINTF_CHECK_H
#define AGGPLOT_PRINTF_CHECK_H

enum arg_type_e {
    argument_int,
    argument_double,
};

extern int check_printf_argument(const char *label, const char*& tail, arg_type_e& arg_type);

#endif
