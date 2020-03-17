#include "elem_plot.h"
#include "elem_plot_fox.h"
#include "Notebook.h"

FXDEFMAP(Notebook) NotebookMap[] = {
    FXMAPFUNC(SEL_PAINT, 0, Notebook::onPaint),
    FXMAPFUNC(SEL_CHANGED, Notebook::ID_TEXT_INPUT, Notebook::onChangeTextInput),
    FXMAPFUNC(SEL_UPDATE, Notebook::ID_TEXT_INPUT, Notebook::onUpdateTextInput),
    FXMAPFUNC(SEL_IO_READ, Notebook::ID_ELEM_WINDOW_START, Notebook::onElemWindowStart),
};

FXIMPLEMENT(Notebook, FXPacker, NotebookMap, ARRAYNUMBER(NotebookMap))

Notebook::Notebook(FXComposite* p, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb, FXint vs):
    FXPacker(p, opts, x, y, w, h),
    m_padleft(pl), m_padright(pr), m_padtop(pt), m_padbottom(pb), m_vspacing(vs),
    m_text_font(nullptr), m_active_child(nullptr)
{
    m_new_window_signal = new FXGUISignal(getApp(), this, ID_ELEM_WINDOW_START, nullptr);
}

Notebook::~Notebook() {
    delete m_new_window_signal;
}

void Notebook::clearContent() {
    FXWindow* p;
    for(FXWindow* c = getLast(); c; c = p) {
        p = c->getPrev();
        delete c;
    }
    m_active_child = nullptr;
}

FXText* Notebook::addTextSection(FXComposite* p, bool editable) {
    auto text = new FXText(p, this, Notebook::ID_TEXT_INPUT, LAYOUT_FILL_X|LAYOUT_FILL_Y|VSCROLLING_OFF|TEXT_AUTOSCROLL);
    text->setVisibleColumns(60);
    text->setVisibleRows(1);
    text->setEditable(editable);
    if (m_text_font) {
        text->setFont(m_text_font);
    }
    return text;
}

void Notebook::cursorMarkRepaint(FXWindow* w) {
    update(0, w->getY(), m_padleft, w->getHeight());
}

void Notebook::sectionMarkRepaint(FXWindow* w) {
    update(0, w->getY(), width, w->getHeight());
}

void Notebook::addElementUpdateLayout(FXWindow* new_child) {
    FXint hcum = m_padtop;
    for(FXWindow* child = getFirst(); child; child = child->getNext()) {
        if (child != new_child) {
            FXint h = child->getDefaultHeight();
            hcum += h + m_vspacing;
        }
    }
    FXint w = width - m_padleft - m_padright;
    FXint h = new_child->getDefaultHeight();
    new_child->position(m_padleft, hcum, w, h);

    setHeight(height + m_vspacing + h);
}

FXText* Notebook::addInputSection() {
    auto frame = new FXVerticalFrame(this, FRAME_LINE);
    auto text = addTextSection(frame, true);
    frame->setBorderColor(FXRGB(BORDER_GRAY,BORDER_GRAY,BORDER_GRAY));
    frame->setBackColor(FXRGB(INPUT_GRAY,INPUT_GRAY,INPUT_GRAY));
    text->setBackColor(FXRGB(INPUT_GRAY,INPUT_GRAY,INPUT_GRAY));
    frame->create();
    addElementUpdateLayout(frame);
    if (m_active_child) {
        cursorMarkRepaint(m_active_child);
    }
    sectionMarkRepaint(frame);
    m_active_child = frame;
    text->setFocus();
    return text;
}

FXText* Notebook::addOutputSection(Notebook::section_type_e section_type) {
    auto text = addTextSection(this, false);
    if (section_type == output_section) {
        text->setTextColor(FXRGB(30, 30, 180));
    } else if (section_type == error_text_section) {
        text->setTextColor(FXRGB(180, 30, 30));
    }
    text->create();
    addElementUpdateLayout(text);
    text->setFocus();
    return text;
}

long Notebook::onElemWindowStart(FXObject *, FXSelector, void *ptr) {
    FXElemStartMessage *message = (FXElemStartMessage *) ptr;
    if (!message) {
        fprintf(stderr, "internal error: no message data with window's start signal\n");
        return 1;
    }
    auto packer = new FXPacker(this, 0, 0, 0, message->width, message->height);
    FXElemBuildWindow(packer, LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, message, ELEM_CREATE_DEFER);
    packer->create();
    addElementUpdateLayout(packer);
    return 1;
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
    FXEvent* ev= (FXEvent*)ptr;
    FXDCWindow dc(this, ev);
    dc.setForeground(FXRGB(255,255,255));
    dc.fillRectangle(ev->rect.x, ev->rect.y, ev->rect.w, ev->rect.h);
    dc.setFont(m_text_font);
    dc.setForeground(FXRGB(30,30,180));
    if (m_active_child) {
        FXint y = m_active_child->getY();
        y += m_text_font->getFontHeight();
        dc.drawText(m_padleft / 2, y, ">", 1);
    }
    return 1;
}

long Notebook::updateTextVisibleRows(FXText* text) {
    if (text->getNumRows() != text->getVisibleRows()) {
        text->setVisibleRows(text->getNumRows());
        return 1;
    }
    return 0;
}

long Notebook::onChangeTextInput(FXObject* obj, FXSelector, void *ptr) {
    return updateTextVisibleRows((FXText*) obj);
}

long Notebook::onUpdateTextInput(FXObject* obj, FXSelector, void *ptr) {
    return updateTextVisibleRows((FXText*) obj);
}
