// Test con el objetivo de cambiar la propiedad de cada procesos.

#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	cprintf("Inicio del test 2\n\n");

	int status = sys_env_set_priority(50);
	if (status < 0) {
		cprintf("Error al setear la prioridad del proceso\n");
		return;
	}

	int current_priority = sys_env_get_priority(0);
	if (current_priority < 0) {
		cprintf("Error al obtener la prioridad del proceso\n");
		return;
	}
	cprintf("La prioridad de este proceso es %d\n", current_priority);

	envid_t envid;
	cprintf("Iniciando segundo env...\n");
	if ((envid = fork()) == 0) {
		cprintf("\nCorriendo segundo env\n");

		int status = sys_env_set_priority(30);
		if (status < 0) {
			cprintf("Error al setear la prioridad del proceso\n");
			sys_env_destroy(envid);
			return;
		}
		cprintf("La prioridad del proceso hijo se ha disminuido\n\n");
	}

	sys_yield();

	current_priority = sys_env_get_priority(0);
	if (current_priority < 0) {
		cprintf("Error al obtener la prioridad del proceso\n");
		sys_env_destroy(envid);
		return;
	}

	sys_yield();

	if (envid == 0) {
		cprintf("Proceso hijo:\n");
	} else {
		cprintf("Proceso padre:\n");
	}
	cprintf("La prioridad del proceso %d es %d\n\n", envid, current_priority);

	sys_yield();

	sys_env_destroy(envid);

	return;
}
