#include "../include/gui.h"
#include <stdio.h>
#include <string.h>

// ═══════════════════════════════════════════
//  HELPER FUNCTIONS
// ═══════════════════════════════════════════
void draw_text(SDL_Renderer *r, TTF_Font *f, const char *text, int x, int y, SDL_Color color) {
    SDL_Surface *surf = TTF_RenderText_Blended(f, text, color);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
    SDL_Rect dst = {x, y, surf->w, surf->h};
    SDL_RenderCopy(r, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
    SDL_FreeSurface(surf);
}

void draw_rect_filled(SDL_Renderer *r, int x, int y, int w, int h, SDL_Color c) {
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(r, &rect);
}

void draw_rect_outline(SDL_Renderer *r, int x, int y, int w, int h, SDL_Color c) {
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderDrawRect(r, &rect);
}

SDL_Color get_priority_color(int priority) {
    if (priority == FIRST_CLASS) return (SDL_Color){255, 180, 50,  255};
    if (priority == BUSINESS)    return (SDL_Color){50,  180, 255, 255};
    return                              (SDL_Color){80,  200, 120, 255};
}

// ═══════════════════════════════════════════
//  INIT & QUIT
// ═══════════════════════════════════════════
int gui_init(AppState *app) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 0;
    if (TTF_Init() < 0) return 0;

    app->window = SDL_CreateWindow(
        "✈  Airport CPU Scheduling Simulator",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H, SDL_WINDOW_SHOWN
    );
    if (!app->window) return 0;

    app->renderer = SDL_CreateRenderer(app->window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!app->renderer) return 0;

    app->font_large  = TTF_OpenFont("assets/DejaVuSans.ttf", 28);
    app->font_medium = TTF_OpenFont("assets/DejaVuSans.ttf", 18);
    app->font_small  = TTF_OpenFont("assets/DejaVuSans.ttf", 13);
    if (!app->font_large || !app->font_medium || !app->font_small) {
        printf("Font error: %s\n", TTF_GetError());
        return 0;
    }

    app->screen           = SCREEN_MENU;
    app->selected_algo    = ALGO_MLQ;
    app->time_quantum     = 2;
    app->gantt_anim_index = 0;
    app->input_step      = 0;
    app->input_passenger = 0;
    app->input_field     = 0;
    app->input_buf_len   = 0;
    app->new_count       = 0;
    memset(app->input_buffer, 0, MAX_INPUT_LEN);
    app->last_anim_time   = 0;

    // Load default sample passengers
    app->scheduler.count = 6;
    app->scheduler.passengers[0] = (Passenger){1,"Justin",  0,4,4,FIRST_CLASS,0,0,0,0,0};
    app->scheduler.passengers[1] = (Passenger){2,"Katy",    1,3,3,FIRST_CLASS,0,0,0,0,0};
    app->scheduler.passengers[2] = (Passenger){3,"Nicolas",0,5,5,BUSINESS,   0,0,0,0,0};
    app->scheduler.passengers[3] = (Passenger){4,"Hailey",  2,4,4,BUSINESS,   0,0,0,0,0};
    app->scheduler.passengers[4] = (Passenger){5,"Emily",    0,6,6,ECONOMY,    0,0,0,0,0};
    app->scheduler.passengers[5] = (Passenger){6,"Travis",  3,5,5,ECONOMY,    0,0,0,0,0};
    app->cmp_ready      = 0;
    app->results_scroll = 0;

    return 1;
}

void gui_quit(AppState *app) {
    TTF_CloseFont(app->font_large);
    TTF_CloseFont(app->font_medium);
    TTF_CloseFont(app->font_small);
    SDL_DestroyRenderer(app->renderer);
    SDL_DestroyWindow(app->window);
    TTF_Quit();
    SDL_Quit();
}

// ═══════════════════════════════════════════
//  SCREEN 1: MAIN MENU
// ═══════════════════════════════════════════
void draw_menu(AppState *app) {
    SDL_Renderer *r = app->renderer;
    SDL_Color white  = {255,255,255,255};
    SDL_Color yellow = {255,220, 50,255};
    SDL_Color gray   = {150,150,160,255};

    draw_text(r, app->font_large,  "AIRPORT CPU SCHEDULING SIMULATOR", 150, 20, yellow);
    draw_text(r, app->font_medium, "Select a Scheduling Algorithm", 380, 65, gray);

    const char *algos[] = {
        "1.  MLQ            Multi Level Queue",
        "2.  MLFQ           Multi Level Feedback Queue",
        "3.  RR + Priority  Hybrid Algorithm",
        "4.  FCFS           First Come First Serve",
        "5.  SJF            Shortest Job First",
        "6.  Round Robin    Equal Time Quantum",
        "7.  Priority       Pure Priority Scheduling"
    };
    SDL_Color btn_colors[] = {
        {255,180, 50,255},
        { 50,180,255,255},
        { 80,200,120,255},
        {220, 80, 80,255},
        {180, 80,220,255},
        { 80,220,200,255},
        {220,140, 50,255}
    };

    for (int i = 0; i < ALGO_COUNT; i++) {
        int y = 100 + i * 68;
        SDL_Color bg = (app->selected_algo == i)
            ? btn_colors[i]
            : (SDL_Color){40,40,60,255};

        draw_rect_filled(r, 150, y, 780, 52, bg);
        draw_rect_outline(r, 150, y, 780, 52, btn_colors[i]);

        SDL_Color tc = (app->selected_algo == i)
            ? (SDL_Color){15,15,25,255} : white;
        draw_text(r, app->font_medium, algos[i], 190, y+15, tc);
    }

    
    draw_text(r, app->font_medium, "                  ENTER to run  |  1-7 select  |  I = input  |  C = compare all  |  Q to quit",
    118, 615, (SDL_Color){201, 163, 39, 255});
draw_text(r, app->font_medium, "                  Eshal Adnan (24K-0518)  |  Sarosh Morani (24K-0999)  |  Abdul Wasay (24K-0744)",
    80, 655, (SDL_Color){150, 180, 255, 255});
}

// ═══════════════════════════════════════════
//  SCREEN 2: RESULTS + ANIMATED GANTT
// ═══════════════════════════════════════════
void draw_results(AppState *app) {
    SDL_Renderer *r = app->renderer;
    SDL_Color white  = {255,255,255,255};
    SDL_Color yellow = {255,220, 50,255};
    SDL_Color gray   = {150,150,160,255};

    const char *algo_names[] = {
        "MLQ — Multi Level Queue",
        "MLFQ — Multi Level Feedback Queue",
        "RR + Priority Hybrid",
        "FCFS — First Come First Serve",
        "SJF — Shortest Job First",
        "Round Robin",
        "Priority Scheduling"
    };

    draw_text(r, app->font_large, algo_names[app->selected_algo], 50, 20, yellow);

    Uint32 now = SDL_GetTicks();
    if (app->gantt_anim_index < app->scheduler.gantt_count &&
        now - app->last_anim_time > 300) {
        app->gantt_anim_index++;
        app->last_anim_time = now;
    }

    draw_text(r, app->font_medium, "Gantt Chart (Execution Timeline)", 50, 70, white);

    int total_time = 0;
    for (int i = 0; i < app->scheduler.gantt_count; i++)
        if (app->scheduler.gantt[i].end_time > total_time)
            total_time = app->scheduler.gantt[i].end_time;

    int gantt_x = 50, gantt_y = 105;
    int gantt_w = 1000, gantt_h = 50;
    float scale = (total_time > 0) ? (float)gantt_w / total_time : 1;

    for (int i = 0; i < app->gantt_anim_index && i < app->scheduler.gantt_count; i++) {
        GanttEntry *g = &app->scheduler.gantt[i];
        int bx = gantt_x + (int)(g->start_time * scale);
        int bw = (int)((g->end_time - g->start_time) * scale);
        if (bw < 2) bw = 2;

        SDL_Color bc = get_priority_color(g->priority);
        draw_rect_filled(r, bx, gantt_y, bw - 1, gantt_h, bc);

        if (bw > 30)
            draw_text(r, app->font_small, g->name, bx + 4, gantt_y + 17,
                (SDL_Color){15,15,25,255});

        char t[16];
        if (g->start_time % 5 == 0) {
            snprintf(t, 16, "%d", g->start_time);
            draw_text(r, app->font_small, t, bx, gantt_y + gantt_h + 4, gray);
        }
    }

    // Final time label
    if (app->gantt_anim_index >= app->scheduler.gantt_count && total_time > 0) {
        char t[16];
        snprintf(t, 16, "%d", total_time);
        draw_text(r, app->font_small, t,
            gantt_x + gantt_w - 15, gantt_y + gantt_h + 4, gray);
    }

    draw_text(r, app->font_medium, "Passenger Results", 50, 195, white);

    SDL_Color hdr = {20,30,60,255};
    draw_rect_filled(r, 50, 225, 1000, 28, hdr);
    draw_text(r, app->font_small, "Name",        60,  230, yellow);
    draw_text(r, app->font_small, "Class",       200, 230, yellow);
    draw_text(r, app->font_small, "Arrival",     330, 230, yellow);
    draw_text(r, app->font_small, "Burst",       460, 230, yellow);
    draw_text(r, app->font_small, "Waiting",     590, 230, yellow);
    draw_text(r, app->font_small, "Turnaround",  720, 230, yellow);
    draw_text(r, app->font_small, "Completion",  880, 230, yellow);

    const char *class_names[] = {"First Class", "Business", "Economy"};

    int max_visible = (WINDOW_H - 320) / 30;
    for (int i = 0; i < app->scheduler.count && i < max_visible; i++) {
        Passenger *p = &app->scheduler.passengers[i];
        int ry = 258 + i * 30;

        SDL_Color row_bg = (i % 2 == 0)
            ? (SDL_Color){10,18,40,255}
            : (SDL_Color){15,24,50,255};
        draw_rect_filled(r, 50, ry, 1000, 30, row_bg);

        SDL_Color cc = get_priority_color(p->priority);
        char buf[32];

        draw_text(r, app->font_small, p->name,                  60,  ry+8, white);
        draw_text(r, app->font_small, class_names[p->priority], 200, ry+8, cc);

        snprintf(buf,32,"%d", p->arrival_time);
        draw_text(r, app->font_small, buf, 330, ry+8, white);
        snprintf(buf,32,"%d", p->burst_time);
        draw_text(r, app->font_small, buf, 460, ry+8, white);
        snprintf(buf,32,"%d", p->waiting_time);
        draw_text(r, app->font_small, buf, 590, ry+8, white);
        snprintf(buf,32,"%d", p->turnaround_time);
        draw_text(r, app->font_small, buf, 720, ry+8, white);
        snprintf(buf,32,"%d", p->completion_time);
        draw_text(r, app->font_small, buf, 880, ry+8, white);
    }

    int max_vis = (WINDOW_H - 320) / 30;
    int shown = app->scheduler.count < max_vis ? app->scheduler.count : max_vis;
    int ay = 258 + shown * 30 + 8;
    draw_rect_filled(r, 50, ay, 1000, 35, (SDL_Color){40,40,65,255});
    char avg[128];
    snprintf(avg, 128, "Avg Waiting Time: %.2f        Avg Turnaround Time: %.2f",
        app->scheduler.avg_waiting_time,
        app->scheduler.avg_turnaround_time);
    draw_text(r, app->font_medium, avg, 100, ay+8, yellow);

    draw_text(r, app->font_small, "Press BACKSPACE to go back  |  1-7 to switch algorithm",
        300, 660, gray);
}

// ═══════════════════════════════════════════
//  MAIN LOOP
// ═══════════════════════════════════════════
void gui_run(AppState *app) {
    SDL_Event e;
    int running = 1;
    SDL_StartTextInput();

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) { running = 0; break; }

            if (app->screen == SCREEN_INPUT && e.type == SDL_TEXTINPUT) {
                if (app->input_buf_len < MAX_INPUT_LEN - 1) {
                    char c = e.text.text[0];
                    if (c != 'i' && c != 'I') {
                        app->input_buffer[app->input_buf_len++] = c;
                        app->input_buffer[app->input_buf_len]   = '\0';
                    }
                }
            }

            if (e.type == SDL_KEYDOWN) {

                if (e.key.keysym.sym == SDLK_BACKSPACE) {
                    if (app->screen == SCREEN_INPUT) {
                        if (app->input_buf_len > 0)
                            app->input_buffer[--app->input_buf_len] = '\0';
                    } else if (app->screen == SCREEN_RESULTS) {
                        app->screen = SCREEN_MENU;
                        app->results_scroll = 0;
                    } else if (app->screen == SCREEN_COMPARE) {
                        app->screen = SCREEN_MENU;
                    }
                }

                // ── UP/DOWN scroll on results ──
                if (app->screen == SCREEN_RESULTS) {
                    if (e.key.keysym.sym == SDLK_UP && app->results_scroll > 0)
                        app->results_scroll--;
                    if (e.key.keysym.sym == SDLK_DOWN)
                        app->results_scroll++;
                }

                if (app->screen == SCREEN_INPUT &&
                    e.key.keysym.sym == SDLK_ESCAPE) {
                    app->screen          = SCREEN_MENU;
                    app->input_step      = 0;
                    app->input_passenger = 0;
                    app->input_field     = 0;
                    app->input_buf_len   = 0;
                    memset(app->input_buffer, 0, MAX_INPUT_LEN);
                }

                if (app->screen == SCREEN_INPUT &&
                    e.key.keysym.sym == SDLK_RETURN && app->input_buf_len > 0) {

                    if (app->input_step == 0) {
                        int n = atoi(app->input_buffer);
                        if (n >= 1 && n <= 10) {
                            app->new_count       = n;
                            app->scheduler.count = n;
                            app->input_step      = 1;
                            app->input_passenger = 0;
                            app->input_field     = 0;
                        }
                        app->input_buf_len = 0;
                        memset(app->input_buffer, 0, MAX_INPUT_LEN);

                    } else {
                        Passenger *p = &app->scheduler.passengers[app->input_passenger];
                        switch (app->input_field) {
                            case 0:
                                strncpy(p->name, app->input_buffer, 32);
                                p->pid = app->input_passenger + 1;
                                break;
                            case 1:
                                p->burst_time     = atoi(app->input_buffer);
                                p->remaining_time = p->burst_time;
                                break;
                            case 2:
                                p->arrival_time = atoi(app->input_buffer);
                                break;
                            case 3: {
                                int cls = atoi(app->input_buffer);
                                p->priority    = (cls >= 0 && cls <= 2) ? cls : 2;
                                p->queue_level = p->priority;
                                break;
                            }
                        }
                        app->input_buf_len = 0;
                        memset(app->input_buffer, 0, MAX_INPUT_LEN);
                        app->input_field++;

                        if (app->input_field == 4) {
                            app->input_field = 0;
                            app->input_passenger++;

                            if (app->input_passenger == app->new_count) {
                                switch (app->selected_algo) {
                                    case ALGO_MLQ:      run_mlq(&app->scheduler, app->time_quantum); break;
                                    case ALGO_MLFQ:     run_mlfq(&app->scheduler, 2, 4, 8); break;
                                    case ALGO_RR_PRIO:  run_rr_priority(&app->scheduler, app->time_quantum); break;
                                    case ALGO_FCFS:     run_fcfs(&app->scheduler); break;
                                    case ALGO_SJF:      run_sjf(&app->scheduler); break;
                                    case ALGO_RR:       run_round_robin(&app->scheduler, app->time_quantum); break;
                                    case ALGO_PRIORITY: run_priority(&app->scheduler); break;
                                }
                                app->gantt_anim_index = 0;
                                app->last_anim_time   = SDL_GetTicks();
                                app->results_scroll   = 0;
                                app->screen           = SCREEN_RESULTS;
                                app->input_step       = 0;
                                app->input_passenger  = 0;
                                app->cmp_ready        = 0;
                            }
                        }
                    }
                }

                if (app->screen == SCREEN_MENU) {
                    switch (e.key.keysym.sym) {
                        case SDLK_q: running = 0; break;

                        case SDLK_c:
                            run_all_algos(app);
                            app->screen = SCREEN_COMPARE;
                            break;

                        case SDLK_i:
                            app->screen          = SCREEN_INPUT;
                            app->input_step      = 0;
                            app->input_passenger = 0;
                            app->input_field     = 0;
                            app->input_buf_len   = 0;
                            memset(app->input_buffer, 0, MAX_INPUT_LEN);
                            break;

                        case SDLK_1: app->selected_algo = ALGO_MLQ;      break;
                        case SDLK_2: app->selected_algo = ALGO_MLFQ;     break;
                        case SDLK_3: app->selected_algo = ALGO_RR_PRIO;  break;
                        case SDLK_4: app->selected_algo = ALGO_FCFS;     break;
                        case SDLK_5: app->selected_algo = ALGO_SJF;      break;
                        case SDLK_6: app->selected_algo = ALGO_RR;       break;
                        case SDLK_7: app->selected_algo = ALGO_PRIORITY; break;

                        case SDLK_RETURN:
                            switch (app->selected_algo) {
                                case ALGO_MLQ:      run_mlq(&app->scheduler, app->time_quantum); break;
                                case ALGO_MLFQ:     run_mlfq(&app->scheduler, 2, 4, 8); break;
                                case ALGO_RR_PRIO:  run_rr_priority(&app->scheduler, app->time_quantum); break;
                                case ALGO_FCFS:     run_fcfs(&app->scheduler); break;
                                case ALGO_SJF:      run_sjf(&app->scheduler); break;
                                case ALGO_RR:       run_round_robin(&app->scheduler, app->time_quantum); break;
                                case ALGO_PRIORITY: run_priority(&app->scheduler); break;
                            }
                            app->gantt_anim_index = 0;
                            app->last_anim_time   = SDL_GetTicks();
                            app->results_scroll   = 0;
                            app->screen           = SCREEN_RESULTS;
                            break;
                    }
                }

                if (app->screen == SCREEN_RESULTS) {
                    switch (e.key.keysym.sym) {
                        case SDLK_1: app->selected_algo = ALGO_MLQ;      goto rerun;
                        case SDLK_2: app->selected_algo = ALGO_MLFQ;     goto rerun;
                        case SDLK_3: app->selected_algo = ALGO_RR_PRIO;  goto rerun;
                        case SDLK_4: app->selected_algo = ALGO_FCFS;     goto rerun;
                        case SDLK_5: app->selected_algo = ALGO_SJF;      goto rerun;
                        case SDLK_6: app->selected_algo = ALGO_RR;       goto rerun;
                        case SDLK_7: app->selected_algo = ALGO_PRIORITY; goto rerun;
                        rerun:
                            switch (app->selected_algo) {
                                case ALGO_MLQ:      run_mlq(&app->scheduler, app->time_quantum); break;
                                case ALGO_MLFQ:     run_mlfq(&app->scheduler, 2, 4, 8); break;
                                case ALGO_RR_PRIO:  run_rr_priority(&app->scheduler, app->time_quantum); break;
                                case ALGO_FCFS:     run_fcfs(&app->scheduler); break;
                                case ALGO_SJF:      run_sjf(&app->scheduler); break;
                                case ALGO_RR:       run_round_robin(&app->scheduler, app->time_quantum); break;
                                case ALGO_PRIORITY: run_priority(&app->scheduler); break;
                            }
                            app->gantt_anim_index = 0;
                            app->last_anim_time   = SDL_GetTicks();
                            app->results_scroll   = 0;
                            break;
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(app->renderer, 10, 14, 26, 255);
        SDL_RenderClear(app->renderer);

        if (app->screen == SCREEN_MENU)    draw_menu(app);
        if (app->screen == SCREEN_RESULTS) draw_results(app);
        if (app->screen == SCREEN_INPUT)   draw_input(app);
        if (app->screen == SCREEN_COMPARE) draw_compare(app);

        SDL_RenderPresent(app->renderer);
        SDL_Delay(16);
    }
    SDL_StopTextInput();
}
// ═══════════════════════════════════════════
//  SCREEN: INPUT
// ═══════════════════════════════════════════
void draw_input(AppState *app) {
    SDL_Renderer *r = app->renderer;
    SDL_Color white  = {255,255,255,255};
    SDL_Color yellow = {255,220, 50,255};
    SDL_Color gray   = {150,150,160,255};
    SDL_Color green  = { 80,200,120,255};

    draw_text(r, app->font_large, "ENTER PASSENGER DETAILS", 280, 20, yellow);
    const char *algo_names[] = {
    "MLQ","MLFQ","RR+Priority","FCFS","SJF","Round Robin","Priority"
    };
    SDL_Color algo_colors[] = {
        {255,180,50,255},{50,180,255,255},{80,200,120,255},
        {220,80,80,255},{180,80,220,255},{80,220,200,255},{220,140,50,255}
    };
    char algo_label[64];
    snprintf(algo_label, 64, "Selected Algorithm: %s", algo_names[app->selected_algo]);
    draw_text(r, app->font_medium, algo_label, 350, 55, algo_colors[app->selected_algo]);
    // Step 0: ask how many passengers
    if (app->input_step == 0) {
        draw_text(r, app->font_medium, "How many passengers? (1-10)", 330, 180, white);
        
        // Input box
        draw_rect_filled(r, 350, 230, 400, 50, (SDL_Color){40,40,60,255});
        draw_rect_outline(r, 350, 230, 400, 50, yellow);
        draw_text(r, app->font_large, app->input_buffer, 365, 242, green);

        draw_text(r, app->font_small, "Press ENTER to confirm", 420, 310, gray);
        return;
    }

    // Step 1: entering passenger details
    int p = app->input_passenger;
    int f = app->input_field;

    // Progress bar
    char prog[64];
    snprintf(prog, 64, "Passenger %d of %d", p + 1, app->new_count);
    draw_text(r, app->font_medium, prog, 450, 70, gray);

    // Progress dots
    for (int i = 0; i < app->new_count; i++) {
        SDL_Color dc = (i < p) ? green : (i == p) ? yellow : (SDL_Color){60,60,80,255};
        draw_rect_filled(r, 200 + i * 35, 110, 25, 8, dc);
    }

    const char *fields[]  = {"Name:", "Burst Time:", "Arrival Time:", "Class (0=First 1=Business 2=Economy):"};
    const char *hints[]   = {"e.g. Alice", "e.g. 4", "e.g. 0", "0, 1 or 2"};

    // Show previously entered fields for this passenger
    const char *field_labels[] = {"Name", "Burst", "Arrival", "Class"};
    draw_text(r, app->font_medium, "Current passenger:", 150, 140, white);

    for (int i = 0; i < f; i++) {
        char done_buf[64];
        Passenger *pass = &app->scheduler.passengers[p];
        char val[32];
        if (i == 0) snprintf(val, 32, "%s", pass->name);
        else if (i == 1) snprintf(val, 32, "%d", pass->burst_time);
        else if (i == 2) snprintf(val, 32, "%d", pass->arrival_time);
        else snprintf(val, 32, "%d", pass->priority);
        snprintf(done_buf, 64, "  %s: %s", field_labels[i], val);
        draw_text(r, app->font_small, done_buf, 150, 170 + i * 25, green);
    }

    // Current field input
    draw_text(r, app->font_medium, fields[f], 150, 310, yellow);
    draw_text(r, app->font_small,  hints[f],  150, 338, gray);

    draw_rect_filled(r, 150, 360, 780, 55, (SDL_Color){40,40,60,255});
    draw_rect_outline(r, 150, 360, 780, 55, yellow);
    draw_text(r, app->font_large, app->input_buffer, 165, 372, green);

    // Cursor blink
    if ((SDL_GetTicks() / 500) % 2 == 0) {
        int cursor_x = 165 + app->input_buf_len * 16;
        draw_rect_filled(r, cursor_x, 375, 2, 30, yellow);
    }

    draw_text(r, app->font_small, "ENTER to confirm  |  BACKSPACE to delete", 350, 440, gray);

    // Already entered passengers summary on right
    if (p > 0) {
        draw_text(r, app->font_small, "Entered so far:", 750, 140, gray);
        for (int i = 0; i < p; i++) {
            Passenger *pass = &app->scheduler.passengers[i];
            const char *cls[] = {"FC","BZ","EC"};
            char summary[64];
            snprintf(summary, 64, "%s [%s] B:%d A:%d",
                pass->name, cls[pass->priority],
                pass->burst_time, pass->arrival_time);
            draw_text(r, app->font_small, summary, 720, 165 + i * 22,
                get_priority_color(pass->priority));
        }
    }

    draw_text(r, app->font_small, "ESC to cancel and go back to menu", 380, 650, gray);
}

// ═══════════════════════════════════════════
//  SCREEN: COMPARISON
// ═══════════════════════════════════════════
void run_all_algos(AppState *app) {
    Scheduler temp;

    // Copy passengers into temp and run each algo
    int algorithms[] = {ALGO_MLQ, ALGO_MLFQ, ALGO_RR_PRIO,
                        ALGO_FCFS, ALGO_SJF, ALGO_RR, ALGO_PRIORITY};

    for (int i = 0; i < ALGO_COUNT; i++) {
        memcpy(&temp, &app->scheduler, sizeof(Scheduler));
        switch (algorithms[i]) {
            case ALGO_MLQ:      run_mlq(&temp, app->time_quantum); break;
            case ALGO_MLFQ:     run_mlfq(&temp, 2, 4, 8); break;
            case ALGO_RR_PRIO:  run_rr_priority(&temp, app->time_quantum); break;
            case ALGO_FCFS:     run_fcfs(&temp); break;
            case ALGO_SJF:      run_sjf(&temp); break;
            case ALGO_RR:       run_round_robin(&temp, app->time_quantum); break;
            case ALGO_PRIORITY: run_priority(&temp); break;
        }
        app->cmp_avg_wait[i] = temp.avg_waiting_time;
        app->cmp_avg_turn[i] = temp.avg_turnaround_time;
    }
    app->cmp_ready = 1;
}
void draw_compare(AppState *app) {
    SDL_Renderer *r = app->renderer;
    SDL_Color white  = {255,255,255,255};
    SDL_Color yellow = {255,220, 50,255};
    SDL_Color gray   = {150,150,160,255};

    draw_text(r, app->font_large, "ALGORITHM COMPARISON", 330, 15, yellow);
    draw_text(r, app->font_small, "All algorithms run on the same passengers", 350, 55, gray);

    const char *names[] = {"MLQ","MLFQ","RR+Prio","FCFS","SJF","RndRobin","Priority"};
    SDL_Color colors[] = {
        {255,180, 50,255},
        { 50,180,255,255},
        { 80,200,120,255},
        {220, 80, 80,255},
        {180, 80,220,255},
        { 80,220,200,255},
        {220,140, 50,255}
    };

    // ── Table header ──
    int tx = 50, ty = 85;
    draw_rect_filled(r, tx, ty, 1000, 30, (SDL_Color){40,40,65,255});
    draw_text(r, app->font_small, "Algorithm",       tx+10,  ty+8, yellow);
    draw_text(r, app->font_small, "Avg Wait",        tx+220, ty+8, yellow);
    draw_text(r, app->font_small, "Avg Turnaround",  tx+400, ty+8, yellow);
    draw_text(r, app->font_small, "Wait Bar",        tx+600, ty+8, yellow);
    draw_text(r, app->font_small, "Turnaround Bar",  tx+800, ty+8, yellow);

    // Find max values for bar scaling
    float max_wait = 1, max_turn = 1;
    for (int i = 0; i < ALGO_COUNT; i++) {
        if (app->cmp_avg_wait[i] > max_wait) max_wait = app->cmp_avg_wait[i];
        if (app->cmp_avg_turn[i] > max_turn) max_turn = app->cmp_avg_turn[i];
    }

    // Find best (lowest) wait and turnaround
    int best_wait = 0, best_turn = 0;
    for (int i = 1; i < ALGO_COUNT; i++) {
        if (app->cmp_avg_wait[i] < app->cmp_avg_wait[best_wait]) best_wait = i;
        if (app->cmp_avg_turn[i] < app->cmp_avg_turn[best_turn]) best_turn = i;
    }

    for (int i = 0; i < ALGO_COUNT; i++) {
        int ry = ty + 30 + i * 70;

        // Row background
        SDL_Color row_bg = (i % 2 == 0)
            ? (SDL_Color){25,25,40,255}
            : (SDL_Color){30,30,50,255};
        draw_rect_filled(r, tx, ry, 1000, 65, row_bg);

        // Algorithm name with color
        draw_rect_filled(r, tx, ry, 6, 65, colors[i]);
        draw_text(r, app->font_medium, names[i], tx+15, ry+10, colors[i]);

        // Values
        char buf[32];
        snprintf(buf, 32, "%.2f", app->cmp_avg_wait[i]);
        SDL_Color wc = (i == best_wait) ? (SDL_Color){80,200,120,255} : white;
        draw_text(r, app->font_medium, buf, tx+220, ry+10, wc);
        if (i == best_wait)
            draw_text(r, app->font_small, "BEST", tx+280, ry+10, wc);

        snprintf(buf, 32, "%.2f", app->cmp_avg_turn[i]);
        SDL_Color tc = (i == best_turn) ? (SDL_Color){80,200,120,255} : white;
        draw_text(r, app->font_medium, buf, tx+400, ry+10, tc);
        if (i == best_turn)
            draw_text(r, app->font_small, "BEST", tx+460, ry+10, tc);

        // Wait bar
        int wb = (int)(app->cmp_avg_wait[i] / max_wait * 180);
        draw_rect_filled(r, tx+600, ry+15, wb, 18, colors[i]);
        draw_rect_outline(r, tx+600, ry+15, 180, 18, (SDL_Color){60,60,80,255});

        // Turnaround bar
        int tb = (int)(app->cmp_avg_turn[i] / max_turn * 180);
        draw_rect_filled(r, tx+800, ry+15, tb, 18, colors[i]);
        draw_rect_outline(r, tx+800, ry+15, 180, 18, (SDL_Color){60,60,80,255});

        // Second line — summary insight
        const char *insights[] = {
            "Fixed queues, strict priority",
            "Adaptive, prevents starvation",
            "Fair within priority levels",
            "Simple, arrival order",
            "Minimizes avg waiting time",
            "Fair, no starvation",
            "Strict priority, may starve low"
        };
        draw_text(r, app->font_small, insights[i], tx+15, ry+38, gray);
    }

    draw_text(r, app->font_small,
        "Press BACKSPACE to go back",
        430, 660, gray);
}