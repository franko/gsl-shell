#pragma once

#include "FXElemBaseWindow.h"

class NotebookElemWindow : public FXElemBaseWindow {
    FXDECLARE(NotebookElemWindow)
protected:
    NotebookElemWindow() { }
private:
    NotebookElemWindow(const NotebookElemWindow&);
    NotebookElemWindow &operator=(const NotebookElemWindow&);
public:
    NotebookElemWindow(FXComposite* p, FXuint opts=FRAME_NORMAL, FXint x=0, FXint y=0, FXint w=0, FXint h=0);

    static NotebookElemWindow *NewFromStartMessage(FXComposite *p, FXuint opts, FXElemStartMessage *message, FXElemCreatePolicy create_flag);

    long onMotion(FXObject *, FXSelector, void* ptr);
};
