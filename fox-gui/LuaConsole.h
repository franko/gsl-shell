#ifndef FOXGUI_LUA_CONSOLE_H
#define FOXGUI_LUA_CONSOLE_H

#include <new>
#include <fx.h>
#include <FXArray.h>

#include "gsl_shell_thread.h"
#include "io_thread.h"
#include "history.h"
#include "Notebook.h"

class LuaConsole : public Notebook
{
    FXDECLARE(LuaConsole)

private:
    enum status_e { not_ready, input_mode, output_mode };

public:
    LuaConsole(gsl_shell_thread* gs, io_redirect* lua_io, FXComposite *p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=32,FXint pr=8,FXint pt=8,FXint pb=8,FXint vs=6);
    ~LuaConsole();

    void init();
    void prepareInput();
    void showErrors();
    void signalNewContent();
    long sendInputText();

    void updateInputLine(const char* line);
    long historySelect(bool up);

    virtual void create();

    long onIOLuaOutput(FXObject*, FXSelector, void*);
    long onInputKeypress(FXObject*, FXSelector, void*);

    enum
    {
        ID_LUA_OUTPUT = Notebook::ID_LAST,
        ID_SCROLL_CONTENT,
        ID_LAST,
    };

protected:
    LuaConsole() {}

private:
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
};

#endif
