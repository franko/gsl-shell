#include <fx.h>

#include "MyFXNotebook.h"

int main(int argc,char *argv[]) {
    FXApp app("Notebook Test", "FoxTest");
    app.init(argc, argv);
    FXMainWindow *win = new FXMainWindow(&app, "Notebook Test Window", NULL, NULL, DECOR_ALL, 0, 0, 600, 400);
    MyFXNotebook *nb = new MyFXNotebook(win, NULL, 0, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXText *text = new FXText(nb, nb, MyFXNotebook::ID_TEXT_INPUT, VSCROLLING_OFF);
    text->setVisibleColumns(60);
    text->setVisibleRows(1);
    text->appendText("Hello world!", 12);
    app.create();
    win->show(PLACEMENT_SCREEN);
    app.run();
    return 0;
}
