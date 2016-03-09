#include "fxkeys.h"

#include "MyFXNotebook.h"
#include "MyFXTextKeylog.h"

FXDEFMAP(MyFXNotebook) MyFXNotebookMap[] = {
    FXMAPFUNC(SEL_PAINT, 0, MyFXNotebook::onPaint),
    FXMAPFUNC(SEL_CHANGED, MyFXNotebook::ID_TEXT_INPUT, MyFXNotebook::onChangeTextInput),
    FXMAPFUNC(SEL_KEYPRESS, MyFXNotebook::ID_TEXT_INPUT, MyFXNotebook::onKeylogNewline),
};

FXIMPLEMENT(MyFXNotebook, FXPacker, MyFXNotebookMap, ARRAYNUMBER(MyFXNotebookMap))

MyFXNotebook::MyFXNotebook(FXComposite* p, FXObject* tgt, FXSelector sel, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb, FXint vs):
    FXPacker(p, opts, x, y, w, h),
    m_padleft(pl), m_padright(pr), m_padtop(pt), m_padbottom(pb), m_vspacing(vs)
{
}

FXText* MyFXNotebook::addInputText(FXObject* tgt) {
    FXText *text = new MyFXTextKeylog(this, tgt, MyFXNotebook::ID_TEXT_INPUT, VSCROLLING_OFF);
    text->setVisibleColumns(60);
    text->setVisibleRows(1);
    return text;
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

void MyFXNotebook::layout() {
    FXint hcum = m_padtop;
    FXint x0 = m_padleft;
    for(FXWindow* child = getFirst(); child; child = child->getNext()) {
        FXint x = x0, y = hcum;
        FXint w = getWidth() - m_padleft - m_padright;
        FXint h = child->getDefaultHeight();
        child->position(x, y, w, h);
        hcum += h + m_vspacing;
    }
}

long MyFXNotebook::onPaint(FXObject*, FXSelector, void *ptr) {
    FXEvent *ev= (FXEvent*)ptr;
    FXDCWindow dc(this, ev);
    dc.setForeground(backColor);
    dc.fillRectangle(ev->rect.x, ev->rect.y, ev->rect.w, ev->rect.h);
    return 1;
}

long MyFXNotebook::onChangeTextInput(FXObject* obj, FXSelector, void *ptr) {
    FXText* text = (FXText*) obj;
    if (text->getNumRows() != text->getVisibleRows()) {
        text->setVisibleRows(text->getNumRows());
        return 1;
    }
    return 0;
}

long MyFXNotebook::onKeylogNewline(FXObject*, FXSelector, void* ptr) {
    FXEvent* event=(FXEvent*)ptr;
    fprintf(stderr, ">> reported keypress %d vs %d\n", event->code, KEY_Return);
    return 0;
}
