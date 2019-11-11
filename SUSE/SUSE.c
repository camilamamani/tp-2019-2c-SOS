#include "SUSE.h"

int main() {
	int cliente_fd;
	pthread_t hilo_clientes, hilo_metricas;

	inicializar();
	configurar();
	int servidor_fd = iniciar_servidor();

	pthread_create(&hilo_metricas, NULL, logear_metricas, NULL);

	while(1) {
		cliente_fd = esperar_cliente(servidor_fd);
		pthread_create(&hilo_clientes, NULL, atender_cliente, cliente_fd);
	}

	liberar();
	return EXIT_SUCCESS;
}

void inicializar() {
	logger = log_create("../SUSE.log", "SUSE", 1, LOG_LEVEL_DEBUG);
	logger_metricas = log_create("../METRICAS.log", "SUSE", 1, LOG_LEVEL_DEBUG);
	diccionario_programas = dictionary_create();
	diccionario_tid_pid = dictionary_create();
	diccionario_tid = dictionary_create();

	cola_new = queue_create();
	cola_blocked = queue_create();
	cola_exit = queue_create();
	programas = malloc(100 * sizeof(programa_t));

	for (int i = 0; i < 100; i++) {
		programas[i].pid = 0;
		programas[i].cola_ready = queue_create();
		programas[i].exec = NULL;
	}

	tid_sem = malloc(sizeof(sem_t));
	sem_init(tid_sem, 0 , 1);

	pid_sem = malloc(sizeof(sem_t));
	sem_init(pid_sem, 0, 1);

	multiprogramacion_sem = malloc(sizeof(sem_t));
	sem_init(multiprogramacion_sem, 0, 1);
}

void logear_metricas() {
	int hilos_new = 0;

	while (1) {
		sleep(METRICS_TIMER);
		log_info(logger_metricas, "Grado de multiprogramación: %i", GRADO_MULTIPROGRAMACION);
	}
}

int cantidad_de_hilos_en_new(programa_t programa) {
	int cant = 0;
	int indice = obtener_indice_de_programa(programa.pid);
	t_queue* aux = cola_new;
	hilo_t* hilo = malloc(sizeof(hilo_t));

	while (aux != NULL) {
		hilo = queue_pop(cola_new);

		if (hilo->pid == programa.pid)
			cant++;
	}

	return cant;
}

void atender_cliente(int cliente_fd) {
	mensaje_t* mensaje = malloc(sizeof(mensaje_t));
	mensaje = recibir_paquete(cliente_fd);

	switch (mensaje->operacion) {
		case 1:
			encolar_hilo_en_new(mensaje->hilo);
			agregar_programa(mensaje->hilo);

			if (GRADO_MULTIPROGRAMACION < MAX_MULTIPROG)
				encolar_hilo_en_ready(mensaje->hilo);

			//imprimir_programas();

			break;
		case 3:
			bloquear_hilo(mensaje->hilo);
			break;
		case 4:
			cerrar_hilo(mensaje->hilo);
			break;
	}
}

void imprimir_programas() {
	int pid = 0;

	for(int i = 0; i < 10; i++) {
		printf("programa %i\n", programas[i].pid);
	}
}

void agregar_programa(hilo_t* hilo) {
	programa_t programa;
	programa.pid = 0;
	programa.cola_ready = queue_create();
	programa.exec = 0;
	programas[PID].pid = PID;
	programas[PID].cola_ready = programa.cola_ready;
	programas[PID].exec = NULL;
	PID++;
}

void atender_nuevo_cliente(int cliente_fd) {
	hilo_t* hilo = malloc(sizeof(hilo_t));
	hilo = recibir_paquete(cliente_fd);
}

int programa_nuevo(hilo_t* hilo) {
	int i = 0;

	while (programas[i].pid != -1) {
		if (programas->pid != hilo->pid)
			i++;
		else
			return 0;
	}

	return 1;
}

void encolar_hilo_en_new(hilo_t* hilo) {
	queue_push(cola_new, hilo);
	log_info(logger, "El hilo %i del programa %i llegó a NEW", hilo->tid, hilo->pid);
}

void cerrar_hilo(hilo_t* hilo) {
	queue_push(cola_exit, hilo);
	log_info(logger, "El ULT %i ha llegado a la cola de EXIT.", hilo->tid);
	sem_wait(multiprogramacion_sem);
	GRADO_MULTIPROGRAMACION--;
	sem_post(multiprogramacion_sem);
}

int obtener_indice_de_programa(int pid) {
	int i = 0;

	while (programas[i].pid != pid)
		i++;

	return i;
}

void encolar_hilo_en_ready() {
	hilo_t* hilo = queue_pop(cola_new);
	int indice = obtener_indice_de_programa(hilo->pid);
	queue_push(programas[indice].cola_ready, hilo);
	log_info(logger, "El hilo %d del programa %d está en READY", hilo->tid, hilo->pid);
	GRADO_MULTIPROGRAMACION++;

	if (GRADO_MULTIPROGRAMACION == MAX_MULTIPROG)
		log_warning(logger, "Se ha alcanzado el grado máximo de multiprogramación");
}

void bloquear_hilo(hilo_t* hilo) {
	char* thread_id = string_itoa(hilo->tid);
	int pid = dictionary_get(diccionario_tid_pid, thread_id);
	hilo_t* thread = queue_pop(colas_exec[pid]);
	queue_push(cola_blocked, thread);
	log_info(logger, "El thread %d ha llegado a la cola de BLOCKED", dictionary_get(diccionario_tid, thread_id));
}

void liberar() {
	config_destroy(config);
	log_destroy(logger);
	log_destroy(logger_metricas);
	dictionary_destroy(diccionario_programas);
	dictionary_destroy(diccionario_tid_pid);
	dictionary_destroy(diccionario_tid);
	sem_destroy(tid_sem);
	sem_destroy(pid_sem);
	sem_destroy(multiprogramacion_sem);
}















