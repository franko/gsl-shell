#include "MyFXNotebook.h"

MyFXNotebook::MyFXNotebook(FXComposite* p, FXObject* tgt, FXSelector sel, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb, FXint vs):
    FXScrollArea(p, VSCROLLER_ALWAYS|HSCROLLING_OFF, x, y, w, h),
    m_padleft(pl), m_padright(pr), m_padtop(pt), m_padbottom(pb), m_vspacing(vs)
{
}

MyFXNotebook::onPaint(FXObject*, FXSelector, void *) {
    FXEvent *ev= (FXEvent*)ptr;
    FXDCWindow dc(this, ev);
    dc.setForeground(backColor);
    dc.fillRectangle(ev->rect.x, ev->rect.y, ev->rect.w, ev->rect.h);
}

void MyFXNotebook::layout() {
    FXint hcum = m_padtop;
    FXint x = m_padleft;
    for(FXWindow* child = getFirst(); child; child = child->getNext()) {
        FXint x = x0, y = hcum;
        FXint w = child->getDefaultWidth(), h = child->getDefaultHeight();
        child->position(x, y, w, h);
        hcum += h + m_vspacing;
    }
}
