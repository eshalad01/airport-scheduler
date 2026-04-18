#include "../include/scheduler.h"
#include <string.h>

// ═══════════════════════════════════════════
//  HELPER: Reset remaining times
// ═══════════════════════════════════════════
static void reset(Scheduler *s) {
    for (int i = 0; i < s->count; i++) {
        s->passengers[i].remaining_time = s->passengers[i].burst_time;
        s->passengers[i].waiting_time = 0;
        s->passengers[i].turnaround_time = 0;
        s->passengers[i].completion_time = 0;
        s->passengers[i].is_done = 0;
        s->passengers[i].queue_level = s->passengers[i].priority;
    }
    s->gantt_count = 0;
}

// ═══════════════════════════════════════════
//  ALGORITHM 1: MLQ
//  Fixed queues — First, Business, Economy
//  Higher class always served first
//  Within each queue: Round Robin
// ═══════════════════════════════════════════
void run_mlq(Scheduler *s, int time_quantum) {
    reset(s);
    int time = 0;
    int done = 0;
    int total = s->count;

    while (done < total) {
        int found = 0;

        // Always check highest priority queue first (0=First, 1=Business, 2=Economy)
        for (int q = 0; q < NUM_QUEUES && !found; q++) {
            for (int i = 0; i < total; i++) {
                Passenger *p = &s->passengers[i];
                if (p->priority == q && !p->is_done && p->arrival_time <= time) {
                    found = 1;
                    int run = (p->remaining_time < time_quantum) ? p->remaining_time : time_quantum;

                    // Add to Gantt chart
                    s->gantt[s->gantt_count++] = (GanttEntry){
                        p->pid, "", time, time + run, p->priority
                    };
                    strncpy(s->gantt[s->gantt_count - 1].name, p->name, 32);

                    p->remaining_time -= run;
                    time += run;

                    if (p->remaining_time == 0) {
                        p->is_done = 1;
                        p->completion_time = time;
                        done++;
                    }
                    break;
                }
            }
        }
        if (!found) time++; // No passenger ready, advance time
    }
    calculate_metrics(s);
}

// ═══════════════════════════════════════════
//  ALGORITHM 2: MLFQ
//  Processes start at their class queue
//  If they use full quantum → demote to lower queue
//  If waiting too long → promote back up
// ═══════════════════════════════════════════
void run_mlfq(Scheduler *s, int quantum1, int quantum2, int quantum3) {
    reset(s);
    int time = 0;
    int done = 0;
    int total = s->count;
    int quantums[3] = {quantum1, quantum2, quantum3};
    int wait_threshold = 10; // Promote if waiting more than this

    while (done < total) {
        // Aging: promote passengers who waited too long
        for (int i = 0; i < total; i++) {
            Passenger *p = &s->passengers[i];
            if (!p->is_done && p->queue_level > 0 && p->waiting_time > wait_threshold) {
                p->queue_level--;
                p->waiting_time = 0;
            }
        }

        int found = 0;
        for (int q = 0; q < NUM_QUEUES && !found; q++) {
            for (int i = 0; i < total; i++) {
                Passenger *p = &s->passengers[i];
                if (p->queue_level == q && !p->is_done && p->arrival_time <= time) {
                    found = 1;
                    int tq = quantums[q];
                    int run = (p->remaining_time < tq) ? p->remaining_time : tq;

                    s->gantt[s->gantt_count++] = (GanttEntry){
                        p->pid, "", time, time + run, p->queue_level
                    };
                    strncpy(s->gantt[s->gantt_count - 1].name, p->name, 32);

                    // Update waiting time for others
                    for (int j = 0; j < total; j++) {
                        if (j != i && !s->passengers[j].is_done && s->passengers[j].arrival_time <= time)
                            s->passengers[j].waiting_time += run;
                    }

                    p->remaining_time -= run;
                    time += run;

                    if (p->remaining_time == 0) {
                        p->is_done = 1;
                        p->completion_time = time;
                        done++;
                    } else {
                        // Used full quantum → demote
                        if (run == tq && p->queue_level < NUM_QUEUES - 1)
                            p->queue_level++;
                    }
                    break;
                }
            }
        }
        if (!found) time++;
    }
    calculate_metrics(s);
}

// ═══════════════════════════════════════════
//  ALGORITHM 3: RR + Priority Hybrid
//  Higher priority class always goes first
//  Within same priority → Round Robin
// ═══════════════════════════════════════════
void run_rr_priority(Scheduler *s, int time_quantum) {
    reset(s);
    int time = 0;
    int done = 0;
    int total = s->count;

    while (done < total) {
        int found = 0;

        for (int q = 0; q < NUM_QUEUES && !found; q++) {
            // Collect all ready passengers at this priority
            int ready[50], rc = 0;
            for (int i = 0; i < total; i++) {
                Passenger *p = &s->passengers[i];
                if (p->priority == q && !p->is_done && p->arrival_time <= time)
                    ready[rc++] = i;
            }

            // Round Robin among ready passengers at this priority
            if (rc > 0) {
                found = 1;
                int idx = ready[0];
                Passenger *p = &s->passengers[idx];
                int run = (p->remaining_time < time_quantum) ? p->remaining_time : time_quantum;

                s->gantt[s->gantt_count++] = (GanttEntry){
                    p->pid, "", time, time + run, p->priority
                };
                strncpy(s->gantt[s->gantt_count - 1].name, p->name, 32);

                p->remaining_time -= run;
                time += run;

                if (p->remaining_time == 0) {
                    p->is_done = 1;
                    p->completion_time = time;
                    done++;
                }
            }
        }
        if (!found) time++;
    }
    calculate_metrics(s);
}

// ═══════════════════════════════════════════
//  Calculate waiting and turnaround times
// ═══════════════════════════════════════════
void calculate_metrics(Scheduler *s) {
    float total_wait = 0, total_turn = 0;
    for (int i = 0; i < s->count; i++) {
        Passenger *p = &s->passengers[i];
        p->turnaround_time = p->completion_time - p->arrival_time;
        p->waiting_time = p->turnaround_time - p->burst_time;
        total_wait += p->waiting_time;
        total_turn += p->turnaround_time;
    }
    s->avg_waiting_time = total_wait / s->count;
    s->avg_turnaround_time = total_turn / s->count;
}
// ═══════════════════════════════════════════
//  ALGORITHM 4: FCFS
//  First Come First Serve — arrival order
// ═══════════════════════════════════════════
void run_fcfs(Scheduler *s) {
    reset(s);
    int time = 0, done = 0, total = s->count;

    while (done < total) {
        int earliest = -1;
        for (int i = 0; i < total; i++) {
            Passenger *p = &s->passengers[i];
            if (!p->is_done && p->arrival_time <= time) {
                if (earliest == -1 ||
                    p->arrival_time < s->passengers[earliest].arrival_time)
                    earliest = i;
            }
        }
        if (earliest == -1) { time++; continue; }

        Passenger *p = &s->passengers[earliest];
        s->gantt[s->gantt_count++] = (GanttEntry){
            p->pid, "", time, time + p->remaining_time, p->priority
        };
        strncpy(s->gantt[s->gantt_count-1].name, p->name, 32);
        time += p->remaining_time;
        p->remaining_time = 0;
        p->is_done = 1;
        p->completion_time = time;
        done++;
    }
    calculate_metrics(s);
}

// ═══════════════════════════════════════════
//  ALGORITHM 5: SJF
//  Shortest Job First — smallest burst first
// ═══════════════════════════════════════════
void run_sjf(Scheduler *s) {
    reset(s);
    int time = 0, done = 0, total = s->count;

    while (done < total) {
        int shortest = -1;
        for (int i = 0; i < total; i++) {
            Passenger *p = &s->passengers[i];
            if (!p->is_done && p->arrival_time <= time) {
                if (shortest == -1 ||
                    p->burst_time < s->passengers[shortest].burst_time)
                    shortest = i;
            }
        }
        if (shortest == -1) { time++; continue; }

        Passenger *p = &s->passengers[shortest];
        s->gantt[s->gantt_count++] = (GanttEntry){
            p->pid, "", time, time + p->remaining_time, p->priority
        };
        strncpy(s->gantt[s->gantt_count-1].name, p->name, 32);
        time += p->remaining_time;
        p->remaining_time = 0;
        p->is_done = 1;
        p->completion_time = time;
        done++;
    }
    calculate_metrics(s);
}

// ═══════════════════════════════════════════
//  ALGORITHM 6: Round Robin
//  Equal time quantum, no priority
// ═══════════════════════════════════════════
void run_round_robin(Scheduler *s, int time_quantum) {
    reset(s);
    int time = 0, done = 0, total = s->count;

    while (done < total) {
        int found = 0;
        for (int i = 0; i < total; i++) {
            Passenger *p = &s->passengers[i];
            if (!p->is_done && p->arrival_time <= time) {
                found = 1;
                int run = (p->remaining_time < time_quantum)
                    ? p->remaining_time : time_quantum;

                s->gantt[s->gantt_count++] = (GanttEntry){
                    p->pid, "", time, time + run, p->priority
                };
                strncpy(s->gantt[s->gantt_count-1].name, p->name, 32);

                p->remaining_time -= run;
                time += run;

                if (p->remaining_time == 0) {
                    p->is_done = 1;
                    p->completion_time = time;
                    done++;
                }
            }
        }
        if (!found) time++;
    }
    calculate_metrics(s);
}

// ═══════════════════════════════════════════
//  ALGORITHM 7: Priority Scheduling
//  Pure priority — no round robin
// ═══════════════════════════════════════════
void run_priority(Scheduler *s) {
    reset(s);
    int time = 0, done = 0, total = s->count;

    while (done < total) {
        int highest = -1;
        for (int i = 0; i < total; i++) {
            Passenger *p = &s->passengers[i];
            if (!p->is_done && p->arrival_time <= time) {
                if (highest == -1 ||
                    p->priority < s->passengers[highest].priority)
                    highest = i;
            }
        }
        if (highest == -1) { time++; continue; }

        Passenger *p = &s->passengers[highest];
        s->gantt[s->gantt_count++] = (GanttEntry){
            p->pid, "", time, time + p->remaining_time, p->priority
        };
        strncpy(s->gantt[s->gantt_count-1].name, p->name, 32);
        time += p->remaining_time;
        p->remaining_time = 0;
        p->is_done = 1;
        p->completion_time = time;
        done++;
    }
    calculate_metrics(s);
}