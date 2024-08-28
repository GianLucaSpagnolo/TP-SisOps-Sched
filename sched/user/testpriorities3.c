// Test con el objetivo de cambiar la propiedad de otros procesos.

#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	cprintf("Inicio del test 3\n\n");

	int current_priority = sys_env_get_priority(0);
	if (current_priority < 0) {
		cprintf("Error al obtener la prioridad del proceso\n");
		return;
	}
	cprintf("La prioridad de este proceso es %d\n", current_priority);

	int status = sys_env_change_priority(0, 50);
	if (status < 0) {
		cprintf("Error al cambiar la prioridad del proceso\n");
		return;
	}

	current_priority = sys_env_get_priority(0);
	if (current_priority < 0) {
		cprintf("Error al obtener la prioridad del proceso\n");
		return;
	}
	cprintf("La prioridad de este proceso ha cambiado, ahora es %d\n",
	        current_priority);
	cprintf("Ha disminuido porque se esta corriendo\n\n");

	envid_t parent_envid = sys_getenvid();

	envid_t envid;
	cprintf("Iniciando segundo env...\n\n");
	if ((envid = fork()) == 0) {
		cprintf("Corriendo env hijo\n");

		int status1 = sys_env_change_priority(parent_envid, 30);
		if (status1 < 0) {
			cprintf("Error al cambiar la prioridad del proceso "
			        "padre\n");
			sys_env_destroy(envid);
			return;
		}

		cprintf("La prioridad del proceso padre ha cambiado\n\n");
	} else {
		cprintf("Corriendo env padre\n");

		int status2 = sys_env_change_priority(envid, 40);
		if (status2 < 0) {
			cprintf("Error al cambiar la prioridad del proceso "
			        "hijo\n");
			sys_env_destroy(envid);
			return;
		}

		cprintf("La prioridad del proceso hijo ha cambiado\n\n");
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
	cprintf("La prioridad del proceso %d es %d\n", envid, current_priority);
	cprintf("Ha incrementado porque ha cambiado cuando se estaba "
	        "corriendo\n\n");

	sys_yield();

	sys_env_destroy(envid);

	return;
}
