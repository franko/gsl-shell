#ifndef FOXGUI_FX_CONSOLE_H
#define FOXGUI_FX_CONSOLE_H

#include <new>
#include <fx.h>
#include <FXArray.h>

#include "gsl_shell_thread.h"
#include "io_thread.h"

class fx_console : public FXText
{
    FXDECLARE(fx_console)

private:
    enum status_e { not_ready, input_mode, output_mode };

    static char const * const prompt;

public:
    fx_console(gsl_shell_thread* gs, FXComposite *p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=3,FXint pr=3,FXint pt=2,FXint pb=2);

    ~fx_console();

    // prepare to accept input
    void init();
    void prepare_input();
    void show_errors();

public:
    virtual void create();

    long on_key_press(FXObject*,FXSelector,void*);
    long on_lua_output(FXObject*,FXSelector,void*);

    enum
    {
        ID_READ_INPUT = FXText::ID_LAST,
        ID_LUA_OUTPUT,
        ID_LAST,
    };

protected:
    fx_console() {}

private:
    FXint m_input_begin;
    FXString m_input;
    status_e m_status;
    gsl_shell_thread* m_engine;

    lua_io_thread* m_lua_io_thread;
    FXGUISignal* m_lua_io_signal;
    FXMutex m_lua_io_mutex;
    FXString m_lua_io_buffer;
};

#endif
