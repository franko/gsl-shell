#ifndef MY_FX_TEXT_KEYLOG_H
#define MY_FX_TEXT_KEYLOG_H

#include <fx.h>

class MyFXTextKeylog : public FXText {
    FXDECLARE(MyFXTextKeylog)
protected:
    MyFXTextKeylog() {};
    MyFXTextKeylog(const MyFXTextKeylog&) {};
public:
    MyFXTextKeylog(FXComposite* p, FXObject* tgt=NULL, FXSelector sel=0, FXuint opts=0, FXint x=0, FXint y=0, FXint w=0, FXint h=0, FXint pl=8, FXint pr=8, FXint pt=8, FXint pb=8);

    long onCmdNewlineLog(FXObject*, FXSelector, void*);

    enum {
        ID_KEYLOG_NEWLINE = FXText::ID_LAST,
        ID_LAST,
    };
};

#endif
