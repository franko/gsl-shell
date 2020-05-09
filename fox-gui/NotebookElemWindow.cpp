#include "NotebookElemWindow.h"

FXDEFMAP(NotebookElemWindow) NotebookElemWindowMap[] = {
    FXMAPFUNC(SEL_MOTION,0,NotebookElemWindow::onMotion),
};

FXIMPLEMENT(NotebookElemWindow,FXElemBaseWindow,NotebookElemWindowMap,ARRAYNUMBER(NotebookElemWindowMap))

NotebookElemWindow::NotebookElemWindow(FXComposite* p, FXuint opts, FXint x, FXint y, FXint w, FXint h):
    FXElemBaseWindow(p, opts, x, y, w, h) {
}

NotebookElemWindow *NotebookElemWindow::NewFromStartMessage(FXComposite *p, FXuint opts, FXElemStartMessage *message, FXElemCreatePolicy create_flag) {
    auto elem_window = new NotebookElemWindow(p, opts);
    elem_window->setWidth(message->width);
    elem_window->setHeight(message->height);
    elem_window->activateElem(message, create_flag);
    return elem_window;
}

long NotebookElemWindow::onMotion(FXObject *, FXSelector, void* ptr) {
    FXEvent *ev = (FXEvent *) ptr;
    FXDCWindow dc(this, ev);
    dc.drawRectangle(4, 4, width - 8, height - 8);
    return 1;
}
