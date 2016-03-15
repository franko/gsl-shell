#include "MyFXNotebook.h"
#include "MyFXTextKeylog.h"

FXDEFMAP(MyFXNotebook) MyFXNotebookMap[] = {
    FXMAPFUNC(SEL_PAINT, 0, MyFXNotebook::onPaint),
    FXMAPFUNC(SEL_CHANGED, MyFXNotebook::ID_TEXT_INPUT, MyFXNotebook::onChangeTextInput),
    FXMAPFUNC(SEL_UPDATE, MyFXNotebook::ID_TEXT_INPUT, MyFXNotebook::onUpdateTextInput),
};

FXIMPLEMENT(MyFXNotebook, FXPacker, MyFXNotebookMap, ARRAYNUMBER(MyFXNotebookMap))

MyFXNotebook::MyFXNotebook(FXComposite* p, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb, FXint vs):
    FXPacker(p, opts, x, y, w, h),
    m_padleft(pl), m_padright(pr), m_padtop(pt), m_padbottom(pb), m_vspacing(vs),
    m_keypress_target(nullptr), m_text_font(nullptr)
{
}

FXText* MyFXNotebook::addTextSection(bool editable) {
    FXText *text = new MyFXTextKeylog(this, m_keypress_target, MyFXNotebook::ID_TEXT_INPUT, VSCROLLING_OFF|TEXT_AUTOSCROLL);
    text->setVisibleColumns(60);
    text->setVisibleRows(1);
    text->setEditable(editable);
    if (m_text_font) {
        text->setFont(m_text_font);
    }
    text->create();
    return text;
}

FXText* MyFXNotebook::addInputSection() {
    FXText* text = addTextSection(true);
    text->setFocus();
    return text;
}

FXText* MyFXNotebook::addOutputSection(MyFXNotebook::section_type_e section_type) {
    FXText* text = addTextSection(false);
    text->setFocus();
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

static FXint CheckVisibleRows(FXText* text) {
    if (text->getNumRows() != text->getVisibleRows()) {
        text->setVisibleRows(text->getNumRows());
        return 1;
    }
    return 0;
}

long MyFXNotebook::onChangeTextInput(FXObject* obj, FXSelector, void *ptr) {
    return CheckVisibleRows((FXText*) obj);
}

long MyFXNotebook::onUpdateTextInput(FXObject* obj, FXSelector, void *ptr) {
    return CheckVisibleRows((FXText*) obj);
}
