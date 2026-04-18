#ifndef SCHEDULER_H
#define SCHEDULER_H

// ═══════════════════════════════════════════
//  AIRPORT CPU SCHEDULER - Data Structures
// ═══════════════════════════════════════════

// Passenger classes (maps to process priority)
#define FIRST_CLASS    0
#define BUSINESS       1
#define ECONOMY        2
#define NUM_QUEUES     3

// Scheduling algorithms
// Scheduling algorithms
#define ALGO_MLQ       0
#define ALGO_MLFQ      1
#define ALGO_RR_PRIO   2
#define ALGO_FCFS      3
#define ALGO_SJF       4
#define ALGO_RR        5
#define ALGO_PRIORITY  6
#define ALGO_COUNT     7

// A Passenger = A Process
typedef struct {
    int pid;              // Passenger ID
    char name[32];        // Passenger name
    int arrival_time;     // When they arrived at airport
    int burst_time;       // How long check-in takes (CPU burst)
    int remaining_time;   // Time left to finish
    int priority;         // 0=First, 1=Business, 2=Economy
    int waiting_time;     // How long they waited
    int turnaround_time;  // Total time from arrival to done
    int completion_time;  // When they finished
    int queue_level;      // Current queue (used in MLFQ)
    int is_done;          // 1 if finished, 0 if not
} Passenger;

// Gantt chart entry (one block in the timeline)
typedef struct {
    int pid;
    char name[32];
    int start_time;
    int end_time;
    int priority;
} GanttEntry;

// Scheduler state
typedef struct {
    Passenger passengers[10];
    int count;
    GanttEntry gantt[200];
    int gantt_count;
    float avg_waiting_time;
    float avg_turnaround_time;
} Scheduler;

// Function declarations
void run_mlq(Scheduler *s, int time_quantum);
void run_mlfq(Scheduler *s, int quantum1, int quantum2, int quantum3);
void run_rr_priority(Scheduler *s, int time_quantum);
void run_fcfs(Scheduler *s);
void run_sjf(Scheduler *s);
void run_round_robin(Scheduler *s, int time_quantum);
void run_priority(Scheduler *s);
void calculate_metrics(Scheduler *s);

#endif