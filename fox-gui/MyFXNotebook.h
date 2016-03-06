#ifndef MY_FX_NOTEBOOX_H
#define MY_FX_NOTEBOOX_H
#include "FXScrollArea.h"
#include "FXComposite.h"

namespace FX {

class MyFXNotebook : public FXScrollArea {
    FXDECLARE(MyFXNotebook)
protected:
    MyFXNotebook() {};
    MyFXNotebook(const MyFXNotebook&) {};
public:
    MyFXNotebook(FXComposite* p, FXObject* tgt=NULL, FXSelector sel=0, FXuint opts=0, FXint x=0, FXint y=0, FXint w=0, FXint h=0, FXint pl=8, FXint pr=8, FXint pt=8, FXint pb=8, FXint vs=6);

    void onPaint(FXObject*, FXSelector, void* ptr) override;

    void FXint getDefaultWidth() override;
    void FXint getDefaultHeight() override;

    void layout() override;

private:
    FXint   m_padleft;          // Left margin
    FXint   m_padright;         // Right margin
    FXint   m_padtop;           // Top margin
    FXint   m_padbottom;        // Bottom margin
    FXint   m_vspacing;         // Vertical child spacing
};
}

#endif
