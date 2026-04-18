#ifndef GUI_H
#define GUI_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "../include/scheduler.h"

// Window dimensions
#define WINDOW_W 1100
#define WINDOW_H 720

// Colors (R, G, B, A)
#define COLOR_BG          10,  14,  26, 255
#define COLOR_PANEL       15,  22,  45, 255
#define COLOR_WHITE      255, 255, 255, 255
#define COLOR_YELLOW     255, 220,  50, 255
#define COLOR_GRAY       150, 150, 160, 255

// Passenger class colors
#define COLOR_FIRST      255, 180,  50, 255   // Gold - First Class
#define COLOR_BUSINESS    50, 180, 255, 255   // Blue - Business
#define COLOR_ECONOMY     80, 200, 120, 255   // Green - Economy

// App screens
#define SCREEN_MENU       0
#define SCREEN_RUNNING    1
#define SCREEN_RESULTS    2
#define SCREEN_INPUT      3
#define MAX_INPUT_LEN     31
#define SCREEN_COMPARE    4

// GUI state
typedef struct {
    SDL_Window   *window;
    SDL_Renderer *renderer;
    TTF_Font     *font_large;
    TTF_Font     *font_medium;
    TTF_Font     *font_small;
    int           screen;
    int           selected_algo;
    int           time_quantum;
    Scheduler     scheduler;
    int           gantt_anim_index;  // For animation
    // Input screen state
    int           input_step;        // 0=num passengers, 1=entering details
    int           input_passenger;   // which passenger we're entering
    int           input_field;       // which field (0=name,1=burst,2=arrival,3=class)
    char          input_buffer[MAX_INPUT_LEN];
    int           input_buf_len;
    int           new_count;         // how many passengers user wants
    // Comparison screen data
    float         cmp_avg_wait[ALGO_COUNT];
    float         cmp_avg_turn[ALGO_COUNT];
    int           cmp_ready;
    int           results_scroll;
    Uint32        last_anim_time;    // Timer for animation
} AppState;

// Function declarations
int  gui_init(AppState *app);
void gui_quit(AppState *app);
void gui_run(AppState *app);
void draw_menu(AppState *app);
void draw_input(AppState *app);
void draw_running(AppState *app);
void draw_results(AppState *app);
void draw_text(SDL_Renderer *r, TTF_Font *f, const char *text, int x, int y, SDL_Color color);
void draw_rect_filled(SDL_Renderer *r, int x, int y, int w, int h, SDL_Color color);
void run_all_algos(AppState *app);
void draw_compare(AppState *app);
#endif