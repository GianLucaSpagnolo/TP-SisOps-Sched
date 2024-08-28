#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>
#include <kern/rand.h>  // Generador de numeros pseudoaleatorios

void sched_halt(void);
void sched_round_robin(void);
void sched_lottery(void);

int ejecuciones = 0;
unsigned int lista_envids[100000];
unsigned int total_envids = 0;

void
agregar_env_a_correr(struct Env *env_a_correr)
{
	ejecuciones++;
	lista_envids[total_envids++] = env_a_correr->env_id;
}

// Cantidad total de llamadas al Scheduler
int llamadas_al_scheduler = 0;

// Choose a user environment to run and run it.
void
sched_yield(void)
{
#ifdef SCHED_ROUND_ROBIN
	sched_round_robin();
#endif

#ifdef SCHED_PRIORITIES
	sched_lottery();
#endif
}


int
get_curenv_index(void)
{
	int curenv_index = 0;
	if (curenv) {
		curenv_index = ENVX(curenv->env_id) + 1;
	}
	return curenv_index;
}

void
run_curenv(void)
{
	if (curenv && curenv->env_status == ENV_RUNNING) {
		agregar_env_a_correr(curenv);
		env_run(curenv);
	}
}

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
//
void
sched_round_robin(void)
{
	llamadas_al_scheduler++;

	int curenv_index = get_curenv_index();
	struct Env *next_env = curenv;

	for (int i = 0; i < NENV; i++) {
		int actual_index = (curenv_index + i) % NENV;

		struct Env *actual_env = &envs[actual_index];
		if (actual_env->env_status == ENV_RUNNABLE) {
			agregar_env_a_correr(actual_env);
			env_run(actual_env);
		}
	}
	run_curenv();

	sched_halt();
}


void
correr_env_actual(struct Env *actual_env)
{
	if (actual_env && actual_env->env_status == ENV_RUNNING) {
		if (actual_env->env_priority > 1) {
			actual_env->env_priority--;
		}

		agregar_env_a_correr(actual_env);
		env_run(actual_env);
	} else {
		sched_halt();
	}
}

void
elegir_env_a_correr(int cant_env_a_correr, struct Env *envs_a_correr[])
{
	int ticket_env = get_random_num() % cant_env_a_correr;
	struct Env *env_a_correr = envs_a_correr[ticket_env];

	if (env_a_correr->env_priority > 1) {
		env_a_correr->env_priority--;
	}

	agregar_env_a_correr(env_a_correr);
	env_run(env_a_correr);
}

// Implement simple priorities scheduling.
//
// Environments now have a "priority" so it must be consider
// when the selection is performed.
//
// Be careful to not fall in "starvation" such that only one
// environment is selected and run every time.
//
// IMPLEMENTACION:
//
// Scheduler de loteria
//
// Cada env tiene una cantidad de tickets, que es corresponde a su prioridad.
// Se sortea un ticket y el env que tenga ese ticket es el que se ejecuta.
//
// Calcula la cantidad maxima de tickets, y genera un numero pseudoaleatorio
// entre 0 y la cantidad maxima de tickets, que determinara el env a correr.
//
void
sched_lottery(void)
{
	llamadas_al_scheduler++;

	struct Env *actual_env = curenv;
	int cant_env_a_correr = 0;

	int total_cant_tickets = 0;

	for (int i = 0; i < NENV; i++) {
		if (envs[i].env_status == ENV_RUNNABLE) {
			total_cant_tickets += envs[i].env_priority;
		}
	}

	if (total_cant_tickets == 0) {
		correr_env_actual(actual_env);
	}

	int ticket = get_random_num() % total_cant_tickets;

	int total_tickets = 0;
	struct Env *envs_a_correr[NENV];
	for (int i = 0; i < NENV; i++) {
		total_tickets += envs[i].env_priority;

		if (envs[i].env_status == ENV_RUNNABLE && total_tickets > ticket) {
			int contador = 0;

			while (cant_env_a_correr > 0 &&
			       contador < cant_env_a_correr) {
				if (envs_a_correr[contador]->env_priority !=
				    envs[i].env_priority) {
					break;
				}
				contador++;
			}

			if (contador == cant_env_a_correr) {
				envs_a_correr[cant_env_a_correr] = &envs[i];
				cant_env_a_correr++;
			}
		}
	}

	if (cant_env_a_correr > 0) {
		elegir_env_a_correr(cant_env_a_correr, envs_a_correr);
	}
}


// IMPRESION DE ESTADISTICAS DEL SCHEDULER
void
impresion_de_estadisticas()
{
	cprintf("\n\033[31;1;4mEstadisticas de todos los Envs:\033[0m\n");

	cprintf("\033[31mLlamadas al scheduler:\033[0m       %d\n",
	        llamadas_al_scheduler);
	cprintf("\033[31mEjecuciones en total:\033[0m        %d\n", ejecuciones);

	cprintf("\033[31mEnvironments:\033[0m\n");
	if (total_envids == 0) {
		cprintf("Ningun Env corrio\n");
	} else {
		unsigned int ejecuciones_por_proceso[100000];
		for (int i = 0; i < total_envids; i++) {
			ejecuciones_por_proceso[ENVX(lista_envids[i])]++;
			cprintf(" - Envid: %d | Corrida nº %d\n",
			        lista_envids[i],
			        ejecuciones_por_proceso[ENVX(lista_envids[i])]);
		}
	}
	cprintf("\n");
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
		impresion_de_estadisticas();

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
