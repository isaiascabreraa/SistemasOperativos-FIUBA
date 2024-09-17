#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>

#define MINIMA_PRIORIDAD 10
#define MAXIMA_PRIORIDAD 0


int cant_sched_calls = 0;
int cant_resets = 0;


void sched_halt(void);

// Choose a user environment to run and run it.
void
sched_yield(void)
{
	struct Env *env = NULL;
	cant_sched_calls++;

#ifdef SCHED_ROUND_ROBIN
	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env this CPU was
	// last running. Switch to the first such environment found.
	//
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	//
	// Never choose an environment that's currently running on
	// another CPU (env_status == ENV_RUNNING). If there are
	// no runnable environments, simply drop through to the code
	// below to halt the cpu.

	// Your code here - Round robin

	/*
	        Se busca decidir que proceso de ejecutará a continuacion
	*/

	int start = (curenv == NULL) ? 0 : ENVX(curenv->env_id);
	for (int i = 1; i <= NENV; i++) {
		int j = (start + i) % NENV;
		if (envs[j].env_status == ENV_RUNNABLE) {
			env = &envs[j];
		}
	}


#endif

#ifdef SCHED_PRIORITIES


	// Implement simple priorities scheduling.
	//
	// Environments now have a "priority" so it must be consider
	// when the selection is performed.
	//
	// Be careful to not fall in "starvation" such that only one
	// environment is selected and run every time.

	// Your code here - Priorities}

	// El valor de prioridad fué completamente arbitrario, yo elegí poner 10.
	int prioridad_actual = MINIMA_PRIORIDAD;
	int start = (curenv == NULL) ? 0 : ENVX(curenv->env_id);
	for (int i = 1; i <= NENV; i++) {
		int j = (start + i) % NENV;
		if (envs[j].env_status == ENV_RUNNABLE &&
		    envs[j].env_priority < prioridad_actual) {
			env = envs + j;
			prioridad_actual = envs[j].env_priority;
			cprintf("id: %d\n", envs[j].env_id);
		}
	}

	if (env) {
		if (env->env_runs % 5 == 0) {
			env->env_priority++;
			env->env_cant_priority_decreases++;
		}

		if (env->env_priority == MINIMA_PRIORIDAD) {
			int i = 0;
			while (i < NENV) {
				envs[i].env_priority = MAXIMA_PRIORIDAD;
				i++;
				cant_resets++;
			}
		}
	}


#endif

	if (env) {
		env_run(env);
	}

	// Without scheduler, keep runing the last environment while it exists
	if (!env && curenv) {
		if (curenv->env_status == ENV_RUNNING)
			env_run(curenv);
	}

	// sched_halt never returns
	sched_halt();
}

// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
//
void
sched_halt(void)
{
	int i;

	// For debugging and testing purposes, if there are no runnable
	// environments in the system, then drop into the kernel monitor.
	for (i = 0; i < NENV; i++) {
		if ((envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING ||
		     envs[i].env_status == ENV_DYING))
			break;
	}


	if (i == NENV) {
		cprintf("No runnable environments in the system!\n");
		cprintf("----------Estadisticas----------\n");
		cprintf("Cantidad de llamadas: %d\n", cant_sched_calls);
		cprintf("Cantidad de reinicios: %d\n", cant_resets);

		for (i = 0; i < NENV; i++) {
			if (envs[i].env_runs > 0) {
				cprintf("Cantidad de ejecuciones del proceso "
				        "%d: %d\n",
				        envs[i].env_id,
				        envs[i].env_runs);
				cprintf("Cantidad de decrecimientos de la "
				        "prioridad del proceso %d: %d\n",
				        envs[i].env_id,
				        envs[i].env_cant_priority_decreases);
			}
		}
		while (1)
			monitor(NULL);
	}

	// Mark that no environment is running on this CPU
	curenv = NULL;
	lcr3(PADDR(kern_pgdir));

	// Mark that this CPU is in the HALT state, so that when
	// timer interupts come in, we know we should re-acquire the
	// big kernel lock
	xchg(&thiscpu->cpu_status, CPU_HALTED);

	// Release the big kernel lock as if we were "leaving" the kernel
	unlock_kernel();

	// Once the scheduler has finishied it's work, print statistics on
	// performance. Your code here

	// Reset stack pointer, enable interrupts and then halt.
	asm volatile("movl $0, %%ebp\n"
	             "movl %0, %%esp\n"
	             "pushl $0\n"
	             "pushl $0\n"
	             "sti\n"
	             "1:\n"
	             "hlt\n"
	             "jmp 1b\n"
	             :
	             : "a"(thiscpu->cpu_ts.ts_esp0));
}
