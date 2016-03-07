#include "MyFXNotebook.h"

FXDEFMAP(MyFXNotebook) MyFXNotebookMap[] = {
    FXMAPFUNC(SEL_PAINT, 0, MyFXNotebook::onPaint),
};

FXIMPLEMENT(MyFXNotebook, FXPacker, MyFXNotebookMap, ARRAYNUMBER(MyFXNotebookMap))

MyFXNotebook::MyFXNotebook(FXComposite* p, FXObject* tgt, FXSelector sel, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb, FXint vs):
    FXPacker(p, opts, x, y, w, h),
    m_padleft(pl), m_padright(pr), m_padtop(pt), m_padbottom(pb), m_vspacing(vs)
{
}

FXint MyFXNotebook::getDefaultWidth() {
    FXint w = 0;
    for(FXWindow* child = getFirst(); child; child = child->getNext()) {
        FXint cw = child->getDefaultWidth();
        if (cw > w) w = cw;
    }
    return (w > 0 ? w + m_padleft + m_padright : FXPacker::getDefaultWidth());
}

FXint MyFXNotebook::getDefaultHeight() {
    FXint h = m_padtop;
    for(FXWindow* child = getFirst(); child; child = child->getNext()) {
        FXint ch = child->getDefaultHeight();
        h += ch;
        if (child != getFirst()) h += m_vspacing;
    }
    h += m_padbottom;
    return h;
}

long MyFXNotebook::onPaint(FXObject*, FXSelector, void *ptr) {
    FXEvent *ev= (FXEvent*)ptr;
    FXDCWindow dc(this, ev);
    dc.setForeground(backColor);
    dc.fillRectangle(ev->rect.x, ev->rect.y, ev->rect.w, ev->rect.h);
    return 1;
}

void MyFXNotebook::layout() {
    FXint hcum = m_padtop;
    FXint x0 = m_padleft;
    for(FXWindow* child = getFirst(); child; child = child->getNext()) {
        FXint x = x0, y = hcum;
        FXint w = getWidth(), h = child->getDefaultHeight();
        child->position(x, y, w, h);
        hcum += h + m_vspacing;
    }
}
