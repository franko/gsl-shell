#include <stdio.h>
#include "MyFXTextKeylog.h"

FXDEFMAP(MyFXTextKeylog) MyFXTextKeylogMap[] = {
    FXMAPFUNC(SEL_COMMAND, FXText::ID_INSERT_NEWLINE, MyFXTextKeylog::onCmdNewlineLog),
};

FXIMPLEMENT(MyFXTextKeylog, FXText, MyFXTextKeylogMap, ARRAYNUMBER(MyFXTextKeylogMap))

MyFXTextKeylog::MyFXTextKeylog(FXComposite* p, FXObject* tgt, FXSelector sel, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb):
    FXText(p, tgt, sel, opts, x, y, w, h, pl, pr, pt, pb)
{
}

long MyFXTextKeylog::onCmdNewlineLog(FXObject* obj, FXSelector sel, void *ptr) {
    if (target) {
        long status = handle(target, FXSEL(SEL_KEYPRESS, message), ptr);
        if (status != 0) {
            return status;
        }
    }
    return FXText::onCmdInsertNewline(obj, sel, ptr);
}
