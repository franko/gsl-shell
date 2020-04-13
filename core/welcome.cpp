#include "core/welcome.h"

#include "luajit.h"

#include "version.h"

const char *kWelcomeMessage = \
	"GSL Shell " GSL_SHELL_RELEASE ", Copyright (C) 2009-2013 Francesco Abbate\n" \
    "GNU Scientific Library, Copyright (C) The GSL Team\n" \
    LUAJIT_VERSION " -- " LUAJIT_COPYRIGHT "\n\n" \
    "Documentation available at http://www.nongnu.org/gsl-shell/doc/.\n" \
    "To obtain help on a function or object type: help(func).\n" \
    "Type demo() to see the lists of demo.";
