#ifndef FOXGUI_LUA_FX_CONSOLE_H
#define FOXGUI_LUA_FX_CONSOLE_H

#include <new>
#include <fx.h>
#include <FXArray.h>

#include "gsl_shell_thread.h"
#include "io_thread.h"
#include "history.h"
#include "MyFXNotebook.h"

class LuaFXConsole : public MyFXNotebook
{
    FXDECLARE(LuaFXConsole)

private:
    enum status_e { not_ready, input_mode, output_mode };
    // enum text_style_e { plain_style = 0, prompt_style = 1, error_style = 2};

    // static const FXchar* prompt;

public:
    LuaFXConsole(gsl_shell_thread* gs, io_redirect* lua_io, FXComposite *p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=3,FXint pr=3,FXint pt=2,FXint pb=2,FXint vs=6);

    ~LuaFXConsole();

    // prepare to accept input
    void init();
    void prepareInput();
    void showErrors();
    void signalNewContent();

    // FXint get_input_length();
    // long update_editable();
    void updateInputLine(const char* line);

    virtual void create();

    // long on_key_press(FXObject*,FXSelector,void*);
    // long on_cmd_delete(FXObject*,FXSelector,void*);
    // long on_cmd_insert_string(FXObject*,FXSelector,void*);
    long onIOLuaOutput(FXObject*, FXSelector, void*);
    long onInputKeypress(FXObject*, FXSelector, void*);

    enum
    {
        ID_LUA_OUTPUT = MyFXNotebook::ID_LAST,
        ID_SCROLL_CONTENT,
        ID_LAST,
    };

protected:
    LuaFXConsole() {}

// private:
    // void init_styles();

private:
    // FXint m_input_begin;
    FXString m_input;
    status_e m_status;
    gsl_shell_thread* m_engine;
    io_redirect* m_lua_io;
    FXObject* m_target;
    long m_message;

    FXText* m_input_section;
    FXText* m_output_section;

    lua_io_thread* m_lua_io_thread;
    FXGUISignal* m_lua_io_signal;
    FXMutex m_lua_io_mutex;
    FXString m_lua_io_buffer;
    FXString m_input_pending;

    FXString m_saved_line;
    history m_history;

    // static FXHiliteStyle m_styles[2];
};

#endif
