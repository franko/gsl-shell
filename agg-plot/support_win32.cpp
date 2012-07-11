#include <windows.h>
#include <shlobj.h>

#include "agg-pixfmt-config.h"

const char *gslshell::get_font_name()
{
    static TCHAR pf[MAX_PATH + 32];
    SHGetFolderPath(0, CSIDL_FONTS, NULL, 0, pf);
    unsigned len = strlen(pf);
    pf[len] = '\\';
    memcpy(pf + len + 1, "arial.ttf", 11 * sizeof(char));
    return pf; 
}
