# ✈️ Airport CPU Scheduling Simulator

A graphical **CPU Process Scheduling Simulator** built in **C + SDL2** with a unique **Airport Check-In Theme**.  
This project demonstrates how classic Operating System scheduling algorithms work using animated queues, passengers, and Gantt chart visualization.

> 🎓 Designed as an Operating Systems semester project.

---

# 📌 Project Idea

Instead of boring CPU processes, we used a **real-world airport check-in system**.

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

📷 **Add Screenshot Here:**  
`/screenshots/main-menu.png`  
(Show main menu with airport theme)

---

# 🚀 Features

✅ 7 CPU Scheduling Algorithms Implemented  
✅ Animated Gantt Chart Visualization  
✅ Passenger Input System  
✅ Results Table (Waiting / Turnaround Time)  
✅ Comparison of All Algorithms  
✅ Beautiful SDL2 GUI  
✅ Real-Life Airport Theme  
✅ Keyboard Controls

📷 **Add Screenshot Here:**  
`/screenshots/results-screen.png`

---

# 🧠 Scheduling Algorithms Included

## 1️⃣ Multi Level Queue (MLQ)

- Separate queues by priority
- Fixed queues
- Round Robin inside each queue

## 2️⃣ Multi Level Feedback Queue (MLFQ)

- Dynamic queue movement
- Aging supported
- Smart scheduling

## 3️⃣ Round Robin + Priority Hybrid

- Priority between classes
- RR fairness inside same class

## 4️⃣ FCFS

First Come First Serve

## 5️⃣ SJF

Shortest Job First

## 6️⃣ Round Robin

Equal time quantum for all

## 7️⃣ Priority Scheduling

Strict priority based execution

📷 **Add Screenshot Here:**  
`/screenshots/algorithm-selection.png`

---

# 🖥️ GUI Screens

## Main Menu

- Select algorithm
- View controls

📷 Add Screenshot:  
`/screenshots/main-menu.png`

---

## Input Screen

- Enter Passenger Name
- Arrival Time
- Burst Time
- Priority Class

📷 Add Screenshot:  
`/screenshots/input-screen.png`

---

## Results Screen

- Animated Gantt Chart
- Waiting Time
- Turnaround Time
- Completion Time

📷 Add Screenshot:  
`/screenshots/results-screen.png`

---

## Comparison Screen

Compare all 7 algorithms side by side.

📷 Add Screenshot:  
`/screenshots/comparison-screen.png`

---

# 🛠️ Technologies Used

- C Language
- SDL2
- SDL2_ttf
- GCC Compiler
- Ubuntu / WSL2
- GitHub

---

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
