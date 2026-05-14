# Real-Time PID Controller with Anti-Windup

**Course: ELE709 — Real-Time and Embedded Systems, Toronto Metropolitan University**

A multithreaded real-time PID controller implemented in C targeting a DC motor position control system. Built on a dedicated real-time hardware platform (dlab), the controller runs a dedicated POSIX control thread synchronized via semaphores, implements a filtered-derivative PID with anti-windup, and was validated under artificial CPU load to characterize timing robustness.

---

## Highlights

- Full PID controller with filtered derivative (backward-difference + first-order IIR) and integral anti-windup
- Dedicated real-time control thread driven by hardware timer interrupts via POSIX semaphore synchronization
- Interactive runtime menu for tuning Kp, Ti, Td, N, sampling frequency, and reference input without recompiling
- Step and square-wave reference input generation with configurable magnitude, frequency, and duty cycle
- System load characterization suite: sequential vs. parallel thread timing benchmarks and parallel matrix multiplication
- Artificial CPU load generator to test PID robustness under stressed system conditions

---

## Architecture

The controller is built around a producer-consumer threading model:

- The **hardware timer ISR** (inside `dlab_def.h`) signals `data_avail` at each sample period
- The **ControlThread** blocks on `sem_wait(&data_avail)`, reads the encoder, computes the PID output, and writes to the D/A converter — all within one sample period
- The **main thread** handles the user menu and is entirely decoupled from the control loop

This structure ensures the control loop timing is driven by hardware, not by OS scheduling of the main thread.

```
Hardware Timer ISR
      |
      | sem_post(&data_avail)
      v
 ControlThread  <--- sem_wait (blocks between samples)
   ReadEncoder()
   PID Compute
   DtoA(VtoD(uk))
      |
 Main Thread (menu, parameter updates)
```

The PID algorithm uses:
- **Integral:** forward-Euler accumulation with gain `1/Ti`
- **Derivative:** backward-difference filtered by a first-order IIR with coefficient `N` to suppress high-frequency noise
- **Control law:** `uk = Kp * (ek + integral + Td * derivative)`

---

## Files

| File | Description |
|---|---|
| `src/pid.c` | Full PID controller with filtered derivative and anti-windup. Interactive menu. Hardware I/O via dlab. |
| `src/pc.c` | Proportional-only controller. Baseline for comparing P vs PID response. |
| `src/load.c` | CPU load generator: 3 threads continuously allocating memory and computing square roots. Used to stress the system during PID runs. |
| `src/lab32.c` | Sequential thread timing benchmark: measures per-operation execution time (+, -, *, /) with 500M iterations each, threads run one at a time. |
| `src/lab33.c` | Parallel thread timing benchmark: same operations launched simultaneously to measure parallelism effects on execution time. |
| `src/lab35.c` | Parallel matrix multiplication (18x16 * 16x18): spawns one POSIX thread per output element (324 threads), each computing one C[i][j]. |
| `docs/Anti PID with Load.pdf` | Lab writeup: PID tuning results, step response plots, and analysis of controller performance under CPU load. |
| `docs/AntiPID.docx` | Supporting writeup for anti-windup design and implementation. |

---

## How to Run

> **Platform requirement:** `pid.c` and `pc.c` require the dlab real-time hardware board and its `dlab_def.h` driver library. `load.c`, `lab32.c`, `lab33.c`, and `lab35.c` run on any POSIX system.

**Build and run the PID controller (on dlab hardware):**
```bash
gcc -o pid src/pid.c -lpthread -lm -ldlab
./pid
```

**Build and run the load generator (in a separate terminal):**
```bash
gcc -o load src/load.c -lpthread -lm
./load
```

**Build and run the timing benchmarks:**
```bash
gcc -o lab32 src/lab32.c -lpthread -lm && ./lab32
gcc -o lab33 src/lab33.c -lpthread -lm && ./lab33
gcc -o lab35 src/lab35.c -lpthread -lm && ./lab35
```

**PID runtime menu options:**

| Key | Action |
|---|---|
| `r` | Run control loop |
| `p` | Set proportional gain Kp |
| `i` | Set integral time constant Ti |
| `d` | Set derivative time constant Td |
| `n` | Set derivative filter coefficient N |
| `f` | Set sampling frequency Fs (Hz) |
| `t` | Set total run time (s) |
| `u` | Set reference input (step or square wave) |
| `g` | Plot position response on screen |
| `h` | Save plot as PostScript |
| `q` | Exit |

---

## Design Decisions

**Semaphore-driven control loop.** Rather than using a `sleep()`-based timer, the control thread blocks on a semaphore that is posted by the hardware timer ISR. This eliminates OS sleep jitter and ties the loop period precisely to the hardware clock.

**Filtered derivative.** A pure backward-difference derivative amplifies measurement noise. The filter `derivative = (Td * raw_deriv + prev_deriv) / (1 + Td / (N * dt))` is a first-order IIR that rolls off at frequency `N * Fs / (2 * pi)`, making the derivative term practical for real motor encoder signals.

**Separated P and PID implementations.** `pc.c` (P-only) and `pid.c` (full PID) are kept as distinct files rather than a single configurable program. This preserves clear experimental baselines matching the lab structure — P response is characterized first, then full PID tuning builds on that.

**Load generator as a separate process.** `load.c` is intentionally a standalone binary rather than threads inside the controller. Running it as a separate process creates realistic OS-level scheduling pressure and CPU contention, which better reflects real embedded deployment conditions than in-process load threads would.

**Per-element threading in lab35.** Spawning one thread per matrix output element (324 threads for an 18x18 result) is pedagogically motivated — it demonstrates thread data partitioning — rather than being an optimal strategy. In practice, thread-pool or BLAS approaches would be used for matrix operations.

---

## Future Improvements

- Add command-line argument parsing so PID gains can be set without an interactive menu
- Implement a thread-safe ring buffer to log control signals and encoder data for post-run analysis without halting the control loop
- Port the derivative filter to a proper bilinear (Tustin) discretization for more accurate frequency response
- Add feed-forward compensation for known reference trajectories to reduce transient tracking error
- Replace the load generator with a configurable load level (number of threads, memory size) for more systematic performance characterization

---

## Skills

**Languages:** C  
**Concurrency:** POSIX threads (`pthreads`), POSIX semaphores, producer-consumer synchronization  
**Control theory:** PID control, anti-windup, filtered derivative, discrete-time integration  
**Real-time systems:** Hardware timer-driven control loops, execution time benchmarking with `clock_gettime`  
**Platform:** dlab real-time hardware board, POSIX (macOS/Linux)
