#include <stdio.h>
#include "../include/gui.h"

int main() {
    AppState app;

    if (!gui_init(&app)) {
        printf("Failed to initialize: %s\n", SDL_GetError());
        return 1;
    }

    gui_run(&app);
    gui_quit(&app);

    return 0;
}