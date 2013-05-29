#include <sys/stat.h>

#include "agg-pixfmt-config.h"

#ifdef DARWIN_MACOSX
const char *ttf_names[] = {"Arial.ttf", "Trebuchet MS.ttf", "Courier New.ttf", 0};
const char *ttf_system_dir[] = {"/Library/Fonts/", 0};
#define CONSOLE_FONT_NAME "Monaco"
#else
const char *ttf_names[] = {
    "ubuntu-font-family/Ubuntu-R.ttf",
    "freefont/FreeSans.ttf", "freefont-ttf/FreeSans.ttf", "FreeSans.ttf",
    "ttf-dejavu/DejaVuSans.ttf", "dejavu/DejaVuSans.ttf", "DejaVuSans.ttf", 0};
const char *ttf_system_dir[] = {"/usr/share/fonts/truetype", "/usr/share/fonts", "/usr/share/fonts/TTF", 0};
#define CONSOLE_FONT_NAME "monospace"
#endif

const char *gslshell::get_font_name()
{
    for (int i = 0; ttf_system_dir[i]; i++)
    {
        const char* ttf_dir = ttf_system_dir[i];
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
    }

    return 0;
}

const char* gslshell::get_fox_console_font_name()
{
    return CONSOLE_FONT_NAME;
}
