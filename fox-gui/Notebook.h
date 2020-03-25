#ifndef MY_FX_NOTEBOOX_H
#define MY_FX_NOTEBOOX_H

#include <fx.h>

namespace FX {

class Notebook : public FXScrollArea {
    FXDECLARE(Notebook)
public:
    enum section_type_e { message_section, error_text_section, output_section };

protected:
    Notebook() {};
    Notebook(const Notebook&) {};
public:
    Notebook(FXComposite* p, FXuint opts=0, FXint x=0, FXint y=0, FXint w=0, FXint h=0, FXint pl=32, FXint pr=8, FXint pt=8, FXint pb=8, FXint vs=6);
    ~Notebook();

    FXText* addTextSection(FXComposite* p, bool editable);
    FXText* addInputSection();
    FXText* addOutputSection(section_type_e section_type);

    void clearContent();

    void setFont(FXFont* font) {
        m_text_font = font;
    }

    FXint getDefaultWidth() override;
    FXint getDefaultHeight() override;

    long onPaint(FXObject*, FXSelector, void* ptr);
    long onChangeTextInput(FXObject*, FXSelector, void*);
    long onUpdateTextInput(FXObject*, FXSelector, void*);
    long onKeylogNewline(FXObject*, FXSelector, void*);
    long onElemWindowStart(FXObject*, FXSelector, void*);

    void layout() override;

    FXGUISignal *elem_start_signal() {
        return m_new_window_signal;
    }

    enum {
        ID_TEXT_INPUT = FXScrollArea::ID_LAST,
        ID_ELEM_WINDOW_START,
        ID_LAST
    };

protected:
    FXint getChildWidth(FXWindow *child);
    FXint getChildExpandWidth(FXWindow *child, FXint expand_width);
    FXint getChildHeight(FXWindow *child);
    void cursorMarkRepaint(FXWindow* w);
    void sectionMarkRepaint(FXWindow* w);
    void addElementUpdateLayout(FXWindow* w);
    long updateTextVisibleRows(FXText* text);

private:
    enum { BORDER_GRAY = 207, INPUT_GRAY = 247 };

    FXint   m_padleft;          // Left margin
    FXint   m_padright;         // Right margin
    FXint   m_padtop;           // Top margin
    FXint   m_padbottom;        // Bottom margin
    FXint   m_vspacing;         // Vertical child spacing
    FXFont* m_text_font;
    FXWindow* m_active_child;
    FXGUISignal *m_new_window_signal;
};
}

#endif
