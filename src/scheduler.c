#include "../include/scheduler.h"
#include <string.h>
#include<limits.h>


// ═══════════════════════════════════════════
//  HELPER: Reset remaining times
// ══════════════════════════════════════════
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

    // Per-queue RR pointer: tracks where we left off in each queue
    int rr_index[NUM_QUEUES] = {0};

    while (done < total) {
        int found = 0;

        for (int q = 0; q < NUM_QUEUES && !found; q++) {

            // Count how many processes belong to this queue
            int q_size = 0;
            for (int i = 0; i < total; i++)
                if (s->passengers[i].priority == q) q_size++;

            if (q_size == 0) continue;

            // Try each process in this queue using round-robin order
            for (int attempt = 0; attempt < q_size; attempt++) {

                // Build index list for this queue in arrival order
                // (or you can pre-sort; here we scan for the rr_index-th member)
                int count = 0;
                for (int i = 0; i < total; i++) {
                    Passenger *p = &s->passengers[i];
                    if (p->priority != q) continue;

                    if (count == rr_index[q] % q_size) {
                        // Found the next candidate in RR order
                        if (!p->is_done && p->arrival_time <= time) {
                            found = 1;
                            int run = (p->remaining_time < time_quantum)
                                        ? p->remaining_time : time_quantum;

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

                            // Advance RR pointer for this queue
                            rr_index[q] = (rr_index[q] + 1) % q_size;
                        } else {
                            // This slot is done or not arrived — skip, advance pointer
                            rr_index[q] = (rr_index[q] + 1) % q_size;
                        }
                        break;
                    }
                    count++;
                }
                if (found) break;
                // If not found at this index, the outer attempt loop tries the next
            }
        }

        if (!found) time++;
    }
    calculate_metrics(s);
}

// ═══════════════════════════════════════════
//  ALGORITHM 2: MLFQ  — your logic
//
//  Three queues, top-down demotion only:
// Q1: Round Robin (q1)
// - All new processes start here
// - If not finished → move to Q2
//
// Q2: Round Robin (q2)
// - Takes processes from Q1
// - If not finished → move to Q3
//
// Q3: FCFS
// - Runs processes till completion
//
// Always execute higher queue first (Q1 → Q2 → Q3)
// No promotion, only downward movement
// ═══════════════════════════════════════════

void run_mlfq(Scheduler *s, int quantum1, int quantum2, int quantum3) {
    reset(s);

    int time  = 0;
    int done  = 0;
    int total = s->count;

    // Q3 uses INT_MAX so remaining_time is always less → true FCFS
    // (run == tq can never be true for Q3, so demotion is impossible)
    int quantums[3] = {quantum1, quantum2, INT_MAX};

    // Per-queue round-robin pointer.
    // Tracks which process is next in line within each queue.
    // Stored as an index into s->passengers[] — we advance it
    // modulo the number of processes belonging to that queue.
    int rr_ptr[NUM_QUEUES] = {0};

    while (done < total) {
        int found = 0;

        // Always scan highest-priority queue first (Q1 → Q2 → Q3)
        for (int q = 0; q < NUM_QUEUES && !found; q++) {

            // Count how many passengers live in this queue right now
            // and build an ordered index list for RR cycling.
            // We use arrival order as the natural queue order.
            int members[50]; // indices into s->passengers
            int m_count = 0;

            for (int i = 0; i < total; i++) {
                Passenger *p = &s->passengers[i];
                if (p->queue_level == q && !p->is_done)
                    members[m_count++] = i;
            }

            if (m_count == 0) continue; // nobody in this queue

            // Clamp rr_ptr in case count shrank since last visit
            rr_ptr[q] = rr_ptr[q] % m_count;

            // Try up to m_count candidates to find one that has arrived
            for (int attempt = 0; attempt < m_count; attempt++) {
                int idx = members[rr_ptr[q]];
                Passenger *p = &s->passengers[idx];

                if (p->arrival_time <= time) {
                    // ── This process runs ────────────────────────────
                    found = 1;
                    int tq  = quantums[q];
                    int run = (p->remaining_time < tq) ? p->remaining_time : tq;

                    // Record Gantt entry
                    s->gantt[s->gantt_count++] = (GanttEntry){
                        p->pid, "", time, time + run, q
                    };
                    strncpy(s->gantt[s->gantt_count - 1].name, p->name, 32);

                    // Update waiting time for all other ready passengers
                    for (int j = 0; j < total; j++) {
                        if (j != idx
                            && !s->passengers[j].is_done
                            && s->passengers[j].arrival_time <= time) {
                            s->passengers[j].waiting_time += run;
                        }
                    }

                    p->remaining_time -= run;
                    time += run;

                    if (p->remaining_time == 0) {
                        // Finished — mark done
                        p->is_done = 1;
                        p->completion_time = time;
                        done++;
                        // Do NOT advance rr_ptr — next member slides into this slot
                    } else {
                        // ── Demotion ──────────────────────────────────
                        // Used full quantum (run == tq only true for Q1 and Q2;
                        // Q3 has tq=INT_MAX so this branch never fires there).
                        if (run == tq && q < NUM_QUEUES - 1) {
                            p->queue_level++; // demote to next lower queue
                        }
                        // Advance RR pointer so the next process in this queue
                        // gets the CPU next time we visit (after higher queues checked)
                        rr_ptr[q] = (rr_ptr[q] + 1) % m_count;
                    }
                    break; // restart outer loop from Q1 (preemption)

                } else {
                    // Not arrived yet — skip, try next in RR order
                    rr_ptr[q] = (rr_ptr[q] + 1) % m_count;
                }
            }
        }

        if (!found) time++; // no process ready — advance time
    }

    calculate_metrics(s);
}


// ═══════════════════════════════════════════
//  ALGORITHM 3: RR + Priority Hybrid (PREEMPTIVE)
//  Higher priority class always goes first
//  Within same priority → Round Robin with time quantum
//  Process is interrupted after quantum expires
//  Higher priority process preempts lower priority
// ═══════════════════════════════════════════
void run_rr_priority(Scheduler *s, int time_quantum) {
    reset(s);
    int time  = 0;
    int done  = 0;
    int total = s->count;

    // Tracks last served index per queue for Round Robin fairness
    int rr_last[NUM_QUEUES];
    for (int q = 0; q < NUM_QUEUES; q++) rr_last[q] = -1;

    while (done < total) {
        int found = 0;

        // Always check highest priority queue first (0 = First, 1 = Business, 2 = Economy)
        for (int q = 0; q < NUM_QUEUES && !found; q++) {

            // Collect all ready passengers at this priority level
            int ready[50], rc = 0;
            for (int i = 0; i < total; i++) {
                Passenger *p = &s->passengers[i];
                if (p->priority == q && !p->is_done && p->arrival_time <= time)
                    ready[rc++] = i;
            }

            if (rc > 0) {
                found = 1;

                // Round Robin: find next process after the last served one
                int chosen = 0; // default to first ready
                if (rr_last[q] != -1) {
                    // Find where last served process is in ready list
                    for (int i = 0; i < rc; i++) {
                        if (ready[i] == rr_last[q]) {
                            // Pick the next one after it (wrap around)
                            chosen = (i + 1) % rc;
                            break;
                        }
                    }
                }

                int idx      = ready[chosen];
                rr_last[q]   = idx; // remember who we just served
                Passenger *p = &s->passengers[idx];

                // PREEMPTIVE: run only up to time quantum
                int run = (p->remaining_time < time_quantum)
                          ? p->remaining_time
                          : time_quantum;

                // Add to Gantt chart
                s->gantt[s->gantt_count++] = (GanttEntry){
                    p->pid, "", time, time + run, p->priority
                };
                strncpy(s->gantt[s->gantt_count - 1].name, p->name, 32);

                p->remaining_time -= run;
                time += run;

                // Only mark done if fully finished
                if (p->remaining_time == 0) {
                    p->is_done         = 1;
                    p->completion_time = time;
                    done++;
                }
                // If not done, it goes back to ready queue next iteration
                // Higher priority processes will preempt it if they arrive
            }
        }

        if (!found) time++; // CPU idle, advance time
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