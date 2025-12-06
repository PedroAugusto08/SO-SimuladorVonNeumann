Done — I ran the metric test and inspected the results.

Summary of what I ran and observed

I ran the project’s test binary via Makefile: make test-metrics, and collected runtime logs and generated metrics files.
The Round-Robin (RR) policy finished successfully; FCFS, SJN and PRIORITY did not finish within the cycle budget and were reported as "falhou" in the CSV.
Report output and CSV snippet (4 cores) from run:
CSV header now includes new columns: FailedProcesses, Success, Error.
Excerpt from metricas_4cores.csv:
RR row: Success true
FCFS, SJN, PRIORITY: Success false, Error: "atingiu o limite de ciclos antes de concluir todos os processos"
The scheduler debug logs show repeated "URGENT-COLLECT" events and several scheduler_dumps.log entries showing finished counts and idle cores.
scheduler_dumps.log shows cases where finished count < total and ready/blocked lists empty => something is left “not finished” (an orphan process).
Important details & potential causes

The problem is consistent: RR completed while the non-preemptive schedulers (FCFS, SJN, PRIORITY) hit the cycle limit. That suggests either:
One or more processes are long running under non-preemptive policies (makes sense — RR’s preemption lets it finish other processes), and the cycle budget (capped at 5,000,000) is insufficient; or
There is an orphaned process (not in finished list, not in ready list, not in blocked list, and not running on any core) that the scheduler never collects — causing has_pending_processes() to remain true, so the scheduler waits until the cycle budget is consumed.
The logs show states like:
finished=7/8 ready=0 blocked=0 cores idle=4 — which strongly suggests an orphaned process.
Possible causes for orphaned processes:
Race between clearing core’s current_process and process state transitions — when cores clear the current_process pointer, process state might still be Running and not get re-enqueued or collected. If process is neither Ready/Finished nor in blocked_list and not in any core, it becomes “lost”.
A core may clear current_process without ensuring the PCB gets re-enqueued (i.e., the old_process->get_state() is Running at that time), resulting in it being neither in a queue nor finished.
Core threads might set stop/exit flags leaving a stale process state, or the Core’s run path can override a Failed state with Finished (observed bug).
I found a bug in Core::run_process: on exception, mark_failed() is called inside the catch block, but afterwards the code still sets process->set_state(State::Finished) when context.endProgram is true — this unintentionally overrides a Failed state with Finished. This can hide real failures and muddle scheduler counts.
Logging: There's a large amount of debug messages. You can control verbosity using SIM_LOG_LEVEL — we saw SIM_LOG_LEVEL=info or warn reduces the spam significantly.
Concrete evidence

The scheduler_dumps.log entries show multiple scheduler max-cycle dumps for FCFS & SJN, showing finished counts less than the expected total (e.g., finished=7/8) with no ready/blocked processes and idle cores.
CSV row: FCFS: Success false, error: “FCFS atingiu o limite de ciclos antes de concluir todos os processos”
Suggested next steps (pick one or more)

Short-term test to validate whether this is a cycle budget issue:

Re-run test_metrics but increase MAX_CYCLES or remove the 5M cap; or run only the FCFS policy to save time and increase cycle budget. If FCFS eventually finishes with a higher budget, it indicates it's just a long-running workload issue rather than a logic bug.
Command suggestion (edit test_metrics.cpp or run a quick debug run): Run SIM_LOG_LEVEL=warn ./bin/test_metrics and observe logs; or temporarily set MAX_CYCLES to a higher number before building.
Debugging orphan processes:

Add debug logs/asserts in scheduler drain/collect logic to detect orphaned processes:
After the main scheduling loop, assert finished_count + ready_count + blocked_list.size() == total_count (or log differences and the list of PCBs that remain “missing”).
Extensively log PID and state on collect; in collect_finished_processes() and urgent collect, log the old_process->get_state() in detail if old process state is not Ready/Finished/Blocked.
Add logging to identify PCBs that are in an “I don’t belong anywhere” state and print their ID and last state transition.
Fix Core::run_process bug:

Ensure that if a process is marked Failed inside the catch, we do not override it. Change the final if (context.endProgram) block into:
If get_state() == Failed => keep it as Failed (or set finish_time and update counters accordingly).
Else if context.endProgram => set Finished.
This will yield correct failed_count, and prevent swallowing exceptions.
Fix potential race / orphaning:

Set current_process to nullptr only when the scheduler collects the process (already the behavior), or set a consistent protocol for clearing:
Ensure that prior to clear_current_process() we set the PCB state to Ready (if preempted) or Finished (if done), or if the state remains Running this indicates an unresolved condition — then do not clear pointer until the process state changes.
Add check in urgent collect: if old_process->get_state() is Running but core->is_idle() and !core->is_thread_running() — print a warning and call core->wait_completion or old_process->set_state(State::Ready) as a last-resort recovery — but log this event (it’s a safety fallback).
Add a unit test that asserts invariants:

finished_count + ready_count + blocked_list.size() + running_count == total_count.
If the invariant fails, log an error and dump in logs for investigation.
Ask for the next action

Do you want me to:
A) Increase the cycle budget and re-run a single-policy (FCFS) benchmark to see if it finishes with more cycles? (Fast)
B) Add debug instrumentation to detect and log orphaned processes (e.g., final invariant checks and logging differences)? (Moderate)
C) Fix core bugs: avoid overriding Failed to Finished in Core::run_process — and add safety checks to collect/schedule_cycle to avoid orphaning (I can implement this now and run tests). (Moderate + build/test)
D) Tackle a combination: fix Core::run_process, add some assertions, and re-run tests with increased budget to confirm. (Most robust)
Which would you prefer me to do next?

If you don’t know, I recommend starting with (C), as fix to run_process prevents hiding real failures, AND (B) short-term detection instrumentation to help find the orphaned process. After that, re-run test_metrics with (A) a larger cycle budget to differentiate between a genuine long process (harmless) vs a bug.
Notes & tips

Use SIM_LOG_LEVEL to reduce noise:
debug / info / warn levels are supported via Log::init_from_env() using env var SIM_LOG_LEVEL.
If you want, I can implement the Core::run_process fix and the detecting assertion now and re-run make test-metrics to confirm the next status and capture more detailed logs.
I can also run a single policy test (modify test_metrics policy list to test only FCFS or run from a small harness) if you prefer.
If you choose a next step, tell me which option (A, B, C, D) you'd like me to implement and I’ll proceed.....................