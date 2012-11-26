#include <sys/stat.h>

#include "agg-pixfmt-config.h"

#ifdef DARWIN_MACOSX
const char *ttf_names[] = {"Trebuchet MS.ttf", "Arial.ttf", 0};
#define TTF_SYSTEM_DIR "/Library/Fonts/"
#define CONSOLE_FONT_NAME "Monaco"
#else
const char *ttf_names[] = {"ubuntu-font-family/Ubuntu-R.ttf", "freefont/FreeSans.ttf", "ttf-dejavu/DejaVuSans.ttf", 0};
#define TTF_SYSTEM_DIR "/usr/share/fonts/truetype/"
#define CONSOLE_FONT_NAME "monospace"
#endif

const char *gslshell::get_font_name()
{
    const char* ttf_dir = TTF_SYSTEM_DIR;
    unsigned len = strlen(ttf_dir);
    static char pf[256];
    memcpy(pf, ttf_dir, len + 1);
    for (int k = 0; ttf_names[k]; k++)
    {
        const char* font_name = ttf_names[k];
        memcpy(pf + len, font_name, (strlen(font_name) + 1) * sizeof(char));
        struct stat inf[1];
        int status = stat(pf, inf);
        if (status == 0)
            return pf;
    }

    return 0;
}

const char* gslshell::get_fox_console_font_name()
{
    return CONSOLE_FONT_NAME;
}
