#include "voraldo1_1.h"
#include "debug.h"
// This contains the very high level expression of what's going on

Voraldo::Voraldo()
{
    pquit = false;

    create_window();
    gl_debug_enable();
    gl_setup();

    while(!pquit)
    {
        draw_everything();
    }
}

Voraldo::~Voraldo()
{
    quit();
}
