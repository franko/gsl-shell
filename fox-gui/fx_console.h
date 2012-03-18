#ifndef FOXGUI_FX_CONSOLE_H
#define FOXGUI_FX_CONSOLE_H

#include <new>

#include <fx.h>
#include <FXArray.h>

class input_history {
public:
  input_history() {}

  void append(const FXString& s) { m_input.append(s); }

  const FXString* operator[] (unsigned i) const
  {
    unsigned n = m_input.no();
    const FXString* p = (i < n ? &m_input[i] : 0);
    return p;
  }

private:
  FXArray<FXString> m_input;
};

class fx_console : public FXText {
  FXDECLARE(fx_console)

private:
  enum status_e { not_ready, input_mode, output_mode };

  static char const * const prompt;

public:
  fx_console(FXComposite *p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=3,FXint pr=3,FXint pt=2,FXint pb=2);

  // prepare to accept input
  void init(const FXString& greeting);
  void prepare_input();

public:
  virtual void create();

  long on_key_press(FXObject*,FXSelector,void*);

protected:
  fx_console() {}

private:
  input_history m_history;
  status_e m_status;
};

#endif
