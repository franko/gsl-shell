
#include <fxkeys.h>

#include "luajit.h"

#include "LuaConsole.h"
#include "gsl_shell_app.h"
#include "gsl_shell_thread.h"
#include "fx_plot_window.h"

FXDEFMAP(LuaConsole) LuaConsole_map[] =
{
    // FXMAPFUNC(SEL_KEYPRESS, 0, LuaConsole::on_key_press),
    // FXMAPFUNC(SEL_COMMAND, FXText::ID_BACKSPACE, LuaConsole::on_cmd_delete),
    // FXMAPFUNC(SEL_COMMAND, FXText::ID_BACKSPACE_BOL, LuaConsole::on_cmd_delete),
    // FXMAPFUNC(SEL_COMMAND, FXText::ID_BACKSPACE_WORD, LuaConsole::on_cmd_delete),
    // FXMAPFUNC(SEL_COMMAND, FXText::ID_DELETE_SEL, LuaConsole::on_cmd_delete),
    // FXMAPFUNC(SEL_COMMAND, FXText::ID_INSERT_STRING, LuaConsole::on_cmd_insert_string),
    FXMAPFUNC(SEL_KEYPRESS, Notebook::ID_TEXT_INPUT, LuaConsole::onInputKeypress),
    FXMAPFUNC(SEL_IO_READ, LuaConsole::ID_LUA_OUTPUT, LuaConsole::onIOLuaOutput),
};

FXIMPLEMENT(LuaConsole,Notebook,LuaConsole_map,ARRAYNUMBER(LuaConsole_map))

LuaConsole::LuaConsole(gsl_shell_thread* gs, io_redirect* lua_io, FXComposite *p, FXObject* tgt, FXSelector sel, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb, FXint vs):
    Notebook(p, opts, x, y, w, h, pl, pr, pt, pb, vs),
    m_status(not_ready), m_engine(gs), m_lua_io(lua_io), m_target(tgt), m_message(sel),
    m_input_section(nullptr), m_output_section(nullptr)
{
    FXApp* app = getApp();
    m_lua_io_signal = new FXGUISignal(app, this, ID_LUA_OUTPUT);
    m_lua_io_thread = new lua_io_thread(m_lua_io, m_lua_io_signal, &m_lua_io_mutex, &m_lua_io_buffer);
}

LuaConsole::~LuaConsole() {
    delete m_lua_io_thread;
    delete m_lua_io_signal;
}

void LuaConsole::signalNewContent() {
    if (m_target) {
        m_target->handle(this, FXSEL(SEL_COMMAND, ID_SCROLL_CONTENT), nullptr);
    }
}

void LuaConsole::prepareInput() {
    m_input_section = addInputSection();
    m_input_section->appendText(m_input_pending);
    m_input_pending.clear();
    m_status = input_mode;
    signalNewContent();
}

void LuaConsole::showErrors() {
    if (m_engine->eval_status() == gsl_shell::eval_error) {
        FXText* text = addOutputSection(error_text_section);
        text->appendText(m_engine->error_msg());
    }
}

void LuaConsole::create() {
    Notebook::create();
    init();
    m_lua_io_thread->start();
}

void LuaConsole::init() {
    clearContent();
    auto text = addOutputSection(message_section);
    FXString msg;
    msg.format("GSL Shell %s, Copyright (C) 2009-2013 Francesco Abbate\n"
               "GNU Scientific Library, Copyright (C) The GSL Team\n"
               "%s -- %s\n\n"
               "Documentation available at http://www.nongnu.org/gsl-shell/doc/.\n"
               "To obtain help on a function or object type: help(func).\n"
               "Type demo() to see the lists of demo.",
               GSL_SHELL_RELEASE, LUAJIT_VERSION, LUAJIT_COPYRIGHT);

    text->setText(msg);
    updateTextVisibleRows(text);
    prepareInput();
}

static void remove_eot_newline(FXString& s, FXint len) {
    s.trunc(len-1); // Remove EOT char.
    if (s[len-2] == '\n') {
        s.trunc(len-2);
    }
    if (s[len-3] == '\r') {
        s.trunc(len-3);
    }
}

long LuaConsole::onIOLuaOutput(FXObject* obj, FXSelector sel, void* ptr)
{
    bool eot = false;

    m_lua_io_mutex.lock();
    FXint len = m_lua_io_buffer.length();
    if (len > 0)
    {
        if (m_lua_io_buffer[len-1] == gsl_shell_thread::eot_character) {
            eot = true;
            remove_eot_newline(m_lua_io_buffer, len);
        }
    }
    if (!m_lua_io_buffer.empty()) {
        if (!m_output_section) {
            m_output_section = addOutputSection(output_section);
        }
        m_output_section->appendText(m_lua_io_buffer, TRUE);
        signalNewContent();
    }

    m_lua_io_buffer.clear();
    m_lua_io_mutex.unlock();

    if (eot) {
        int status = m_engine->eval_status();
        if (status == gsl_shell::incomplete_input) {
            m_history.remove_last();
            m_status = input_mode;
            m_input_section->appendText("\n");
            m_input_section->setCursorPos(m_input_section->getLength());
            handle(m_input_section, FXSEL(SEL_UPDATE, ID_TEXT_INPUT), nullptr);
            m_input_section->setFocus();
            signalNewContent();
        } else {
            showErrors();
            prepareInput();
        }
    }

    return 1;
}

void LuaConsole::updateInputLine(const char* line) {
    FXint len = strlen(line);
    m_input_section->setText(line, len);
    m_input_section->setCursorPos(len);
    handle(m_input_section, FXSEL(SEL_UPDATE, ID_TEXT_INPUT), nullptr);
    signalNewContent();
}

long LuaConsole::sendInputText() {
    if (m_input_section != nullptr) {
        FXString line = m_input_section->getText();
        const char* input_line = line.text();
        m_history.add(input_line);
        m_output_section = nullptr;
        m_status = output_mode;
        m_engine->set_request(gsl_shell_thread::execute_request, input_line);
        return 1;
    }
    return 0;
}

long LuaConsole::historySelect(bool up) {
    const char* new_line;
    if (up) {
        if (m_history.is_first()) {
            m_saved_line = m_input_section->getText();
        }
        new_line = m_history.previous();
    } else {
        new_line = m_history.next();
        if (m_history.is_first()) {
            new_line = m_saved_line.text();
        }
    }
    if (new_line) {
        updateInputLine(new_line);
    }
    return 1;
}

long LuaConsole::onInputKeypress(FXObject* obj, FXSelector sel, void* ptr) {
    FXEvent* event=(FXEvent*)ptr;
    if (m_status != input_mode || m_input_section == nullptr) return 0;
    switch (event->code) {
    case KEY_Return:
    case KEY_KP_Enter:
        return sendInputText();
    case KEY_Up:
    case KEY_KP_Up:
        if (m_input_section->lineStart(m_input_section->getCursorPos()) == 0) {
            return historySelect(true);
        }
        return 0;
    case KEY_Down:
    case KEY_KP_Down:
        if (m_input_section->lineEnd(m_input_section->getCursorPos()) == m_input_section->getLength()) {
            return historySelect(false);
        }
        return 0;
    case KEY_Escape:
        if (m_input_section->hasFocus()) {
            updateInputLine("");
        }
        return 1;
    default:
        /* */;
    }
    return 0;
}
