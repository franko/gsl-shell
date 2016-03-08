#ifndef MY_FX_NOTEBOOX_H
#define MY_FX_NOTEBOOX_H

#include <fx.h>

namespace FX {

class MyFXNotebook : public FXPacker {
    FXDECLARE(MyFXNotebook)
protected:
    MyFXNotebook() {};
    MyFXNotebook(const MyFXNotebook&) {};
public:
    MyFXNotebook(FXComposite* p, FXObject* tgt=NULL, FXSelector sel=0, FXuint opts=0, FXint x=0, FXint y=0, FXint w=0, FXint h=0, FXint pl=8, FXint pr=8, FXint pt=8, FXint pb=8, FXint vs=6);

    FXint getDefaultWidth() override;
    FXint getDefaultHeight() override;

    long onPaint(FXObject*, FXSelector, void* ptr);
    long onChangeTextInput(FXObject*, FXSelector, void* ptr);

    void layout() override;

    enum {
        ID_TEXT_INPUT = FXPacker::ID_LAST,
        ID_LAST
    };

private:
    FXint   m_padleft;          // Left margin
    FXint   m_padright;         // Right margin
    FXint   m_padtop;           // Top margin
    FXint   m_padbottom;        // Bottom margin
    FXint   m_vspacing;         // Vertical child spacing
};
}

#endif
