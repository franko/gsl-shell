
#include <fxkeys.h>

#include "luajit.h"

#include "LuaFXConsole.h"
#include "gsl_shell_app.h"
#include "gsl_shell_thread.h"
#include "fx_plot_window.h"

FXDEFMAP(LuaFXConsole) LuaFXConsole_map[] =
{
    // FXMAPFUNC(SEL_KEYPRESS, 0, LuaFXConsole::on_key_press),
    // FXMAPFUNC(SEL_COMMAND, FXText::ID_BACKSPACE, LuaFXConsole::on_cmd_delete),
    // FXMAPFUNC(SEL_COMMAND, FXText::ID_BACKSPACE_BOL, LuaFXConsole::on_cmd_delete),
    // FXMAPFUNC(SEL_COMMAND, FXText::ID_BACKSPACE_WORD, LuaFXConsole::on_cmd_delete),
    // FXMAPFUNC(SEL_COMMAND, FXText::ID_DELETE_SEL, LuaFXConsole::on_cmd_delete),
    // FXMAPFUNC(SEL_COMMAND, FXText::ID_INSERT_STRING, LuaFXConsole::on_cmd_insert_string),
    FXMAPFUNC(SEL_KEYPRESS, MyFXNotebook::ID_TEXT_INPUT, LuaFXConsole::onInputKeypress),
    FXMAPFUNC(SEL_IO_READ, LuaFXConsole::ID_LUA_OUTPUT, LuaFXConsole::onIOLuaOutput),
};

FXIMPLEMENT(LuaFXConsole,MyFXNotebook,LuaFXConsole_map,ARRAYNUMBER(LuaFXConsole_map))

// const FXchar * LuaFXConsole::prompt = "> ";

LuaFXConsole::LuaFXConsole(gsl_shell_thread* gs, io_redirect* lua_io, FXComposite *p, FXObject* tgt, FXSelector sel, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb, FXint vs):
    MyFXNotebook(p, tgt, sel, opts, x, y, w, h, pl, pr, pt, pb, vs),
    m_status(not_ready), m_engine(gs), m_lua_io(lua_io)
{
    FXApp* app = getApp();
    m_lua_io_signal = new FXGUISignal(app, this, ID_LUA_OUTPUT);
    m_lua_io_thread = new lua_io_thread(m_lua_io, m_lua_io_signal, &m_lua_io_mutex, &m_lua_io_buffer);
    setKeypressTarget(this);
    // init_styles();
}

LuaFXConsole::~LuaFXConsole() {
    delete m_lua_io_thread;
    delete m_lua_io_signal;
}

void LuaFXConsole::prepareInput() {
    FXText* text = addInputSection();
    text->appendText(m_input_pending);
    m_input_pending.clear();
    m_status = input_mode;
}

void LuaFXConsole::showErrors() {
    if (m_engine->eval_status() == gsl_shell::eval_error) {
        FXText* text = addOutputSection(error_text_section);
        text->appendText(m_engine->error_msg());
        text->appendText("\n");
        // text->makePositionVisible(text->getCursorPos());
    }
}

void LuaFXConsole::create() {
    MyFXNotebook::create();
    init();
    m_lua_io_thread->start();
}

void LuaFXConsole::init() {
    FXText* text = addOutputSection(message_section);
    FXString msg;
    msg.format("GSL Shell %s, Copyright (C) 2009-2013 Francesco Abbate\n"
               "GNU Scientific Library, Copyright (C) The GSL Team\n"
               "%s -- %s\n\n"
               "Documentation available at http://www.nongnu.org/gsl-shell/doc/.\n"
               "To obtain help on a function or object type: help(func).\n"
               "Type demo() to see the lists of demo.\n\n",
               GSL_SHELL_RELEASE, LUAJIT_VERSION, LUAJIT_COPYRIGHT);

    text->setText(msg);
    prepareInput();
}

void LuaFXConsole::updateInputLine(const char* line)
{
    FXText* text = getCurrentInput();
    text->replaceText(0, text->getLength(), line, strlen(line));
    // makePositionVisible(getCursorPos());
}

long LuaFXConsole::onIOLuaOutput(FXObject* obj, FXSelector sel, void* ptr)
{
    bool eot = false;

    m_lua_io_mutex.lock();
    FXint len = m_lua_io_buffer.length();
    if (len > 0)
    {
        if (m_lua_io_buffer[len-1] == gsl_shell_thread::eot_character)
        {
            eot = true;
            m_lua_io_buffer.trunc(len-1);
        }
    }
    FXText* text = getCurrentOutput();
    text->appendText(m_lua_io_buffer);
    // makePositionVisible(getCursorPos());

    m_lua_io_buffer.clear();
    m_lua_io_mutex.unlock();

    if (eot) {
        int status = m_engine->eval_status();

        if (status == gsl_shell::incomplete_input) {
            m_history.remove_last();
            m_status = input_mode;
        } else {
            showErrors();
            prepareInput();
        }
    }

    return 1;
}

long LuaFXConsole::onInputKeypress(FXObject* obj, FXSelector sel, void* ptr) {
    FXEvent* event=(FXEvent*)ptr;
    if (event->code == KEY_Return) {
        FXString line;
        FXText* text = getCurrentInput();
        if (text != nullptr) {
            text->getText(line);
            fprintf(stderr, "Line: %s\n", line.text());
        }
    }
    return 0;
}
