
#include <fxkeys.h>

#include "luajit.h"

#include "fx_console.h"
#include "gsl_shell_app.h"
#include "gsl_shell_thread.h"
#include "fx_plot_window.h"

FXHiliteStyle fx_console::m_styles[2];

FXDEFMAP(fx_console) fx_console_map[]=
{
    FXMAPFUNC(SEL_KEYPRESS, 0, fx_console::on_key_press),
    FXMAPFUNC(SEL_COMMAND, FXText::ID_BACKSPACE, fx_console::on_cmd_delete),
    FXMAPFUNC(SEL_COMMAND, FXText::ID_BACKSPACE_BOL, fx_console::on_cmd_delete),
    FXMAPFUNC(SEL_COMMAND, FXText::ID_BACKSPACE_WORD, fx_console::on_cmd_delete),
    FXMAPFUNC(SEL_COMMAND, FXText::ID_DELETE_SEL, fx_console::on_cmd_delete),
    FXMAPFUNC(SEL_IO_READ, fx_console::ID_LUA_OUTPUT, fx_console::on_lua_output),
};

FXIMPLEMENT(fx_console,FXText,fx_console_map,ARRAYNUMBER(fx_console_map))

const FXchar * fx_console::prompt = "> ";

fx_console::fx_console(gsl_shell_thread* gs, FXComposite *p, FXObject* tgt, FXSelector sel, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb):
    FXText(p, tgt, sel, opts, x, y, w, h, pl, pr, pt, pb),
    m_status(not_ready), m_engine(gs)
{
    FXApp* app = getApp();
    m_lua_io_signal = new FXGUISignal(app, this, ID_LUA_OUTPUT);
    m_lua_io_thread = new lua_io_thread(m_engine, m_lua_io_signal, &m_lua_io_mutex, &m_lua_io_buffer);

    init_styles();
}

fx_console::~fx_console()
{
    delete m_lua_io_thread;
    delete m_lua_io_signal;
}

void fx_console::init_styles()
{
    FXColor bgcol   = FXRGB(0x27, 0x28, 0x22);
    FXColor bgcol_a = FXRGB(0x10, 0x10, 0x42);
    FXColor fgcol   = FXRGB(0xe4, 0xe4, 0xc0);
    FXColor fgcol_a = FXRGB(0x66, 0xd9, 0xef);
    FXColor fgcol_e = FXRGB(0xc0, 0x10, 0x10);

    setBackColor(bgcol);
    setTextColor(fgcol);
    setCursorColor(fgcol);

    m_styles[0].normalForeColor = fgcol_a;
    m_styles[0].normalBackColor = 0;
    m_styles[0].selectForeColor = fgcol_a;
    m_styles[0].selectBackColor = 0;
    m_styles[0].hiliteForeColor = fgcol_a;
    m_styles[0].hiliteBackColor = 0;
    m_styles[0].activeBackColor = bgcol_a;
    m_styles[0].style = STYLE_TEXT;

    m_styles[1].normalForeColor = fgcol_e;
    m_styles[1].normalBackColor = 0;
    m_styles[1].selectForeColor = fgcol_e;
    m_styles[1].selectBackColor = 0;
    m_styles[1].hiliteForeColor = fgcol_e;
    m_styles[1].hiliteBackColor = 0;
    m_styles[1].activeBackColor = 0;
    m_styles[1].style = STYLE_TEXT;

    setStyled();
    setHiliteStyles(m_styles);
}

void fx_console::prepare_input()
{
    appendStyledText(prompt, strlen(prompt), prompt_style);
    m_status = input_mode;
    m_input_begin = getCursorPos();
}

void fx_console::show_errors()
{
    if (m_engine->eval_status() == gsl_shell::eval_error)
    {
        FXchar const * const em = "error reported: ";
        appendStyledText(em, strlen(em), error_style);
        appendText(m_engine->error_msg());
        appendText("\n");
        makePositionVisible(getCursorPos());
    }
}

void fx_console::create()
{
    FXText::create();
    init();
    m_lua_io_thread->start();
}

void fx_console::init()
{
    FXString msg;
    msg.format("GSL Shell %s, Copyright (C) 2009-2012 Francesco Abbate\n"
               "GNU Scientific Library, Copyright (C) The GSL Team\n"
               "%s -- %s\n",
               GSL_SHELL_RELEASE, LUAJIT_VERSION, LUAJIT_COPYRIGHT);

    setText(msg);
    FXint pos = getLength();
    setCursorPos(pos);
    setFocus();
    prepare_input();
}

FXint fx_console::get_input_length()
{
    if (m_status == input_mode && getCursorPos() >= m_input_begin)
    {
        FXint buf_len = getLength();
        FXint line_len = buf_len - m_input_begin;
        return line_len;
    }
    return (-1);
}

long fx_console::update_editable()
{
    if (getCursorPos() < m_input_begin && isEditable())
    {
        setEditable(false);
        return 1;
    }
    else if (getCursorPos() >= m_input_begin && !isEditable())
    {
        setEditable(true);
        return 1;
    }
    return 0;
}

long fx_console::on_key_press(FXObject* obj, FXSelector sel, void* ptr)
{
    FXEvent* event = (FXEvent*)ptr;
    switch (event->code)
    {
    case KEY_Return:
    case KEY_KP_Enter:
    {
        FXint line_len = get_input_length();
        if (line_len < 0) return 1;

        extractText(m_input, m_input_begin, line_len);
        setCursorPos(m_input_begin + line_len);
        appendText("\n");

        if (m_input == "exit")
        {
            FXApp* app = getApp();
            app->handle(this, FXSEL(SEL_COMMAND, gsl_shell_app::ID_CONSOLE_CLOSE), NULL);
        }
        else
        {
            const char* input_line = m_input.text();
            m_history.add(input_line);
            this->m_status = output_mode;
            m_engine->set_request(gsl_shell_thread::execute_request, input_line);
        }

        return 1;
    }
    case KEY_Up:
    case KEY_KP_Up:
    {
        FXint line_len = get_input_length();
        if (line_len < 0) break;

        if (m_history.is_first())
            extractText(m_saved_line, m_input_begin, line_len);

        const char* line = m_history.previous();
        if (line)
            replaceText(m_input_begin, line_len, line, strlen(line));

        return 1;
    }
    case KEY_Down:
    case KEY_KP_Down:
    {
        FXint line_len = get_input_length();
        if (line_len < 0) break;

        const char* line = m_history.next();

        if (m_history.is_first())
            line = m_saved_line.text();

        replaceText(m_input_begin, line_len, line, strlen(line));
        return 1;
    }
    case KEY_Home:
    case KEY_KP_Home:
    {
        FXint line_len = get_input_length();
        if (line_len < 0) break;

        setCursorPos(m_input_begin);
        return 1;
    }
    }

    return FXText::onKeyPress(obj, sel, ptr);
}

long fx_console::on_lua_output(FXObject* obj, FXSelector sel, void* ptr)
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
    appendText(m_lua_io_buffer);
    makePositionVisible(getCursorPos());

    m_lua_io_buffer.clear();
    m_lua_io_mutex.unlock();

    if (eot)
    {
        int status = m_engine->eval_status();

        if (status == gsl_shell::incomplete_input)
        {
            m_status = input_mode;
        }
        else
        {
            show_errors();
            prepare_input();
        }
    }

    return 1;
}

long fx_console::on_cmd_delete(FXObject* obj, FXSelector sel, void* ptr)
{
    int pos = getCursorPos();

    if (isPosSelected(pos))
    {
        if (getSelStartPos() < m_input_begin)
            return 1;
    }
    else
    {
        int del_pos = pos;

        switch (FXSELID(sel))
        {
        case ID_BACKSPACE:
            del_pos = pos - 1;
            break;
        case ID_BACKSPACE_BOL:
        case ID_BACKSPACE_WORD:
            del_pos = rowStart(pos);
            break;
        default:
            /* */;
        }
        if (del_pos < m_input_begin)
            return 1;
    }

    return this->FXText::handle(obj, sel, ptr);
}
