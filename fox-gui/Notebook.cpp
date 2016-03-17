#include "Notebook.h"

FXDEFMAP(Notebook) NotebookMap[] = {
    FXMAPFUNC(SEL_PAINT, 0, Notebook::onPaint),
    FXMAPFUNC(SEL_CHANGED, Notebook::ID_TEXT_INPUT, Notebook::onChangeTextInput),
    FXMAPFUNC(SEL_UPDATE, Notebook::ID_TEXT_INPUT, Notebook::onUpdateTextInput),
};

FXIMPLEMENT(Notebook, FXPacker, NotebookMap, ARRAYNUMBER(NotebookMap))

Notebook::Notebook(FXComposite* p, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb, FXint vs):
    FXPacker(p, opts, x, y, w, h),
    m_padleft(pl), m_padright(pr), m_padtop(pt), m_padbottom(pb), m_vspacing(vs),
    m_text_font(nullptr)
{
}

FXText* Notebook::addTextSection(bool editable) {
    auto text = new FXText(this, this, Notebook::ID_TEXT_INPUT, VSCROLLING_OFF|TEXT_AUTOSCROLL);
    text->setVisibleColumns(60);
    text->setVisibleRows(1);
    text->setEditable(editable);
    if (m_text_font) {
        text->setFont(m_text_font);
    }
    text->create();
    return text;
}

FXText* Notebook::addInputSection() {
    FXText* text = addTextSection(true);
    text->setFocus();
    return text;
}

FXText* Notebook::addOutputSection(Notebook::section_type_e section_type) {
    FXText* text = addTextSection(false);
    text->setFocus();
    return text;
}

FXint Notebook::getDefaultWidth() {
    FXint w = 0;
    for(FXWindow* child = getFirst(); child; child = child->getNext()) {
        FXint cw = child->getDefaultWidth();
        if (cw > w) w = cw;
    }
    return (w > 0 ? w + m_padleft + m_padright : FXPacker::getDefaultWidth());
}

FXint Notebook::getDefaultHeight() {
    FXint h = m_padtop;
    for(FXWindow* child = getFirst(); child; child = child->getNext()) {
        FXint ch = child->getDefaultHeight();
        h += ch;
        if (child != getFirst()) h += m_vspacing;
    }
    h += m_padbottom;
    return h;
}

void Notebook::layout() {
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

long Notebook::onPaint(FXObject*, FXSelector, void *ptr) {
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

long Notebook::onChangeTextInput(FXObject* obj, FXSelector, void *ptr) {
    return CheckVisibleRows((FXText*) obj);
}

long Notebook::onUpdateTextInput(FXObject* obj, FXSelector, void *ptr) {
    return CheckVisibleRows((FXText*) obj);
}
