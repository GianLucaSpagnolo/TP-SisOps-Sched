// Test con el objetivo de obtener la prioridad de un proceso actual.

#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	cprintf("Inicio del test\n\n");

	cprintf("La prioridad de este proceso es %d\n", sys_env_get_priority(0));

	int status = sys_env_set_priority(50);
	if (status < 0) {
		cprintf("Error al setear la prioridad del proceso\n");
		return;
	} else if (status == 0) {
		int priority = sys_env_get_priority(0);
		if (priority < 0) {
			cprintf("Error al obtener la prioridad del proceso\n");
			return;
		} else {
			cprintf("La prioridad de este proceso paso a ser "
			        "%d\n\n",
			        priority);
		}
	}

	cprintf("Se intenta incrementar la prioridad del proceso actual\n\n");

	int status2 = sys_env_set_priority(80);
	if (status2 == 0) {
		cprintf("Error al setear la prioridad del proceso: no se "
		        "deberia poder incrementar\n");
		return;
	} else {
		cprintf("No se pudo incrementar la prioridad del proceso "
		        "actual (deseado)\n");
		int priority2 = sys_env_get_priority(0);
		if (priority2 < 0) {
			cprintf("Error al obtener la prioridad del proceso\n");
			return;
		} else {
			cprintf("La prioridad de este proceso ahora es %d\n\n",
			        priority2);
		}
	}

	cprintf("Fin del test\n");

	return;
}
