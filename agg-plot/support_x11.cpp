#include <sys/stat.h>

#include "agg-pixfmt-config.h"

const char *ttf_names[] = {"ubuntu-font-family/Ubuntu-R.ttf", "freefont/FreeSans.ttf", "ttf-dejavu/DejaVuSans.ttf", 0};

const char *gslshell::get_font_name()
{
    const char* ttf_dir = "/usr/share/fonts/truetype/";
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
    return "monospace";
}
