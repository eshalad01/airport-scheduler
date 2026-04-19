# ✈️ Airport CPU Scheduling Simulator

A graphical **CPU Process Scheduling Simulator** built in **C + SDL2** with a unique **Airport Check-In Theme**.  
This project demonstrates how classic Operating System scheduling algorithms work using animated queues, passengers, and Gantt chart visualization.

# Project Idea 
We used a **real world airport checkin system**.
| Airport Concept | OS Concept |
|--------|-----------|
| Passenger | Process |
| Check-in Time | CPU Burst Time |
| Arrival Time | Process Arrival Time |
| Ticket Class | Priority |
| Check-in Counter | CPU |
| Waiting Queue | Ready Queue |

### Passenger Classes
| Class | Priority |
|------|----------|
| First Class | Highest (0) |
| Business | Medium (1) |
| Economy | Lowest (2) |
  
`/screenshots/main-menu.png`  

# Features
- 7 CPU Scheduling Algorithms Implemented  
- Animated Gantt Chart Visualization  
- Passenger Input System  
- Results Table (Waiting / Turnaround Time)  
- Comparison of All Algorithms  
- Beautiful SDL2 GUI  
- Real-Life Airport Theme  
- Keyboard Controls

`/screenshots/results-screen.png`


# Scheduling Algorithms Included

## Multi Level Queue (MLQ)
- Separate queues by priority
- Fixed queues
- Round Robin inside each queue

## Multi Level Feedback Queue (MLFQ)
- Dynamic queue movement
- Aging supported
- Smart scheduling

## Round Robin + Priority Hybrid
- Priority between classes
- RR fairness inside same class

## FCFS
First Come First Serve

## SJF
Shortest Job First

## Round Robin
Equal time quantum for all

## Priority Scheduling
Strict priority based execution

`/screenshots/algorithm-selection.png`

## Results Screen
- Animated Gantt Chart
- Waiting Time
- Turnaround Time
- Completion Time

`/screenshots/results-screen.png`

## Comparison Screen
Compare all 7 algorithms side by side.

`/screenshots/comparison-screen.png`


# Contributors
- [@saroshanwerali](https://github.com/saroshmorani)
- [@eshalad01](https://github.com/eshalad01)

# 📂 Project Structure
```bash
airport_scheduler/
│── include/
│   ├── scheduler.h
│   └── gui.h
│
│── src/
│   ├── main.c
│   ├── scheduler.c
│   └── gui.c
│
│── assets/
│   └── DejaVuSans.ttf
│
│── screenshots/
│── README.md
