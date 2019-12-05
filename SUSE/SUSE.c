#include "SUSE.h"
#include "globales.h"

int main() {
	int cliente_fd = 0;
	pthread_t hilo_clientes, hilo_metricas;

	configurar();
	inicializar();
	inicializar_diccionario_semaforos();

	servidor_fd = iniciar_servidor();

	//pthread_create(&hilo_metricas, NULL, (void*)logear_metricas, NULL);

	while(1) {
		cliente_fd = esperar_cliente(servidor_fd);
		pthread_create(&hilo_clientes, NULL, (void*)atender_cliente, cliente_fd);
		pthread_detach(hilo_clientes);
	}

	//pthread_detach(hilo_metricas);
	liberar();
	return EXIT_SUCCESS;
}

long tiempo_actual() {
    struct timeval tiempo_actual;
    gettimeofday(&tiempo_actual, NULL);
    long microsegundos = tiempo_actual.tv_usec/1000;
    return microsegundos;
}

void inicializar_metricas_hilo(hilo_t* hilo) {
	hilo->tiempo_creacion = 0;
	hilo->tiempo_cpu = 0;
	hilo->tiempo_espera = 0;
	hilo->hilos_a_esperar = list_create();
}

bool es_programa_hilo_buscado(programa_t* programa) {
	return programa->hilo_en_exec->tid == tid_hilo_buscado;
}

programa_t* obtener_programa_de_hilo(int tid) {
	tid_hilo_buscado = tid;
	return list_find(programas, (void*)es_programa_hilo_buscado);
}

void eliminar_hilo_de_hilos_a_esperar(hilo_t* hilo_que_espera, int tid) {

	bool es_hilo_buscado(hilo_t* un_hilo) {
		return un_hilo->tid == tid;
	}

	if (!list_is_empty(hilo_que_espera))
		list_remove_by_condition(hilo_que_espera->hilos_a_esperar, es_hilo_buscado);
}

bool no_espera_mas_hilos(hilo_t* hilo) {
	return list_is_empty(hilo->hilos_a_esperar);
}

void validar_si_esta_esperando_hilos(hilo_t* hilo) {
	if (list_is_empty(hilo->hilos_a_esperar))
		encolar_hilo_en_ready(hilo);
}

hilo_t* crear_hilo(int pid, int tid) {
	hilo_t* hilo = malloc(sizeof(hilo_t));
	hilo->pid = pid;
	hilo->tid = tid;
	hilo->tiempo_cpu = 0;
	hilo->tiempo_creacion = 0;
	hilo->tiempo_espera = 0;
	hilo->tiempo_ultima_llegada_a_exec = 0;
	hilo->tiempo_ultima_llegada_a_ready = 0;
	hilo->estimacion_anterior = 0;
	hilo->rafaga_anterior = 0;
	hilo->hilos_a_esperar = list_create();
	return hilo;
}

programa_t* obtener_programa(int pid) {

	bool es_programa_buscado(programa_t* programa) {
		return programa->pid == pid;
	}

	return list_find(programas, (void*)es_programa_buscado);
}

bool esta_bloqueado(hilo_t* hilo) {
	bool es_hilo_buscado(hilo_t* un_hilo) {
		return un_hilo->tid == hilo->tid && un_hilo->pid == hilo->pid;
	}

	return list_any_satisfy(cola_blocked->elements, es_hilo_buscado);
}

void atender_cliente(int cliente_fd) {
	sem_wait(&atender_clientes_sem);

	int pedido = 0;
	int valor_semaforo = 0;
	int offset = 0;
	int opcode = 0;
	int size = 0;
	int tid = 0;
	int pid = 0;
	int proximo = 0;
	int tamanio = 0;
	int tamanio_nombre_semaforo = 0;
	char* nombre_semaforo = malloc(100);

	if (opcode == 5 || opcode == 6) {
		recv(cliente_fd, &tamanio_nombre_semaforo, sizeof(int), MSG_WAITALL);
		printf("tamaño: %i\n", tamanio_nombre_semaforo);
		recv(cliente_fd, nombre_semaforo, tamanio_nombre_semaforo, MSG_WAITALL);
		printf("nombre del semáforo: %s\n", nombre_semaforo);
		sem_value = dictionary_get(diccionario_semaforos, nombre_semaforo);
	}

	/*hilo_t* hilo = malloc(sizeof(hilo_t));
	inicializar_metricas_hilo(hilo);
	hilo->tid = tid;
	hilo->pid = pid;
	char* proximo;

	bool es_hilo_buscado(hilo_t* un_hilo) {
		return un_hilo->tid == hilo->tid && un_hilo->pid == hilo->pid;
	}

	programa_t* programa = malloc(sizeof(programa_t));
	programa = obtener_programa(pid);
	hilo_t* proximo_hilo = malloc(sizeof(hilo_t));*/

	recv(cliente_fd, &opcode, sizeof(int), MSG_WAITALL);

	switch (opcode) {
		case INIT:
			recv(cliente_fd, &pid, sizeof(int), MSG_WAITALL);
			agregar_programa(pid);
			break;
		case CREATE:
			recv(cliente_fd, &pid, sizeof(int), MSG_WAITALL);
			recv(cliente_fd, &tid, sizeof(int), MSG_WAITALL);
			log_info(logger, "el hilo %i del programa %i llegó a NEW", tid, pid);
			hilo_t* hilo_nuevo = malloc(sizeof(hilo_t));
			hilo_nuevo = crear_hilo(pid, tid);
			hilo_nuevo->tiempo_creacion = tiempo_actual();
			sem_wait(&cola_new_sem);
			queue_push(cola_new, hilo_nuevo);
			sem_post(&cola_new_sem);

			if (GRADO_MULTIPROGRAMACION < MAX_MULTIPROG) {

				if (!queue_is_empty(cola_new)) {
					sem_wait(&cola_new_sem);
					hilo_nuevo = queue_pop(cola_new);
					printf("hilo que estaba en NEW: %i\n", hilo_nuevo->tid);
					sem_post(&cola_new_sem);
					sem_wait(&cola_ready_sem);
					encolar_hilo_en_ready(hilo_nuevo);
					sem_post(&cola_ready_sem);
				}
			}

			break;
		case SCHEDULE_NEXT:
			recv(cliente_fd, &pid, sizeof(int), MSG_WAITALL);
			recv(cliente_fd, &tid, sizeof(int), MSG_WAITALL);
			//hilo_t* proximo_hilo; = malloc(sizeof(hilo_t));
			hilo_t* proximo_hilo = crear_hilo(pid, tid);
			programa_t* programa = obtener_programa(hilo_nuevo);
			int proximo_tid = ejecutar_nuevo_hilo(programa);
			//proximo = string_itoa(proximo_hilo->tid);
			send(cliente_fd, &proximo_tid, sizeof(int), MSG_WAITALL);
			break;
		case JOIN:

			printf("elementos bloqueados: %i\n", queue_size(cola_blocked));


			recv(cliente_fd, &pid, sizeof(int), MSG_WAITALL);
			recv(cliente_fd, &tid, sizeof(int), MSG_WAITALL);
			hilo_t* hilo = crear_hilo(pid, tid);

			//if (!esta_bloqueado(hilo))
				//bloquear_hilo(hilo);

			if (!esta_bloqueado(hilo)) {
				bool hilo_encontrado(hilo_t* un_hilo) {
					return un_hilo->tid == hilo->tid && un_hilo->pid == hilo->pid;
				}

				programa_t* programa_del_hilo = obtener_programa(hilo->pid);
				list_remove_by_condition(programa_del_hilo->hilos_en_ready, hilo_encontrado);
				queue_push(cola_blocked, hilo);
				log_info(logger, "El hilo %i del programa %i llegó a BLOCKED", hilo->tid, hilo->pid);
			}



			break;
		case CLOSE:
			/*
			cerrar_hilo(hilo);

			if (!queue_is_empty(cola_blocked)) {
				list_iterate(cola_blocked->elements, eliminar_hilo_de_hilos_a_esperar);
				list_iterate(cola_blocked->elements, validar_si_esta_esperando_hilos);
			}

			ejecutar_nuevo_hilo(hilo);
			*/
			break;
		case WAIT:
			semaforo_wait(nombre_semaforo);
			break;
		case SIGNAL:
			semaforo_signal(nombre_semaforo);
			break;
	}

	sem_post(&atender_clientes_sem);
}

int ejecutar_nuevo_hilo(hilo_t* hilo) {

	bool hilo_encontrado(hilo_t* un_hilo) {
		return un_hilo->tid == hilo->tid && un_hilo->pid == hilo->pid;
	}

	hilo_t* hilo_anterior = malloc(sizeof(hilo_t));
	programa_t* programa = malloc(sizeof(programa_t));
	programa->hilos_en_ready = list_create();
	programa->hilo_en_exec = malloc(sizeof(hilo_t));
	programa = obtener_programa(hilo->pid);

	if (programa->hilo_en_exec != NULL)
		hilo_anterior = programa->hilo_en_exec;

	hilo_t* siguiente_hilo = siguiente_hilo_a_ejecutar(programa);

	log_info(logger, "El hilo %i del programa %i llegó a EXEC.", siguiente_hilo->tid, siguiente_hilo->pid);
	siguiente_hilo->tiempo_ultima_llegada_a_exec = tiempo_actual();
	siguiente_hilo->tiempo_espera += tiempo_actual() - hilo->tiempo_ultima_llegada_a_ready;
	programa->hilo_en_exec = siguiente_hilo;
	list_remove_by_condition(programa->hilos_en_ready, hilo_encontrado);
	return siguiente_hilo->tid;
}

void agregar_programa(int pid) {
	programa_t* programa = malloc(sizeof(programa_t));
	programa->pid = pid;
	programa->hilos_en_ready = list_create();
	programa->hilo_en_exec = malloc(sizeof(hilo_t));
	list_add(programas, programa);
}

void atender_nuevo_cliente(int cliente_fd) {
	hilo_t* hilo = malloc(sizeof(hilo_t));
	hilo = recibir_paquete(cliente_fd);
}

void cerrar_hilo(hilo_t* hilo) {
	programa_t* programa = obtener_programa(hilo->pid);
	queue_push(cola_exit, hilo);
	log_info(logger, "El hilo %i del programa %i llegó a EXIT.", hilo->tid, hilo->pid);
	sem_wait(multiprogramacion_sem);
	GRADO_MULTIPROGRAMACION--;
	sem_post(multiprogramacion_sem);
}

void encolar_hilo_en_ready(hilo_t* hilo) {
	printf("me llego el hilo %i del programa %i\n", hilo->tid, hilo->pid);

	bool hilo_encontrado(hilo_t* un_hilo) {
		 return un_hilo->tid == hilo->tid && un_hilo->pid == hilo->pid;
	}

	programa_t* programa = malloc(sizeof(programa_t));
	programa->hilos_en_ready = list_create();
	programa->hilo_en_exec = malloc(sizeof(hilo_t));
	programa = obtener_programa(hilo->pid);

	//if (list_any_satisfy(cola_blocked->elements, (void*)hilo_encontrado))
		//list_remove_by_condition(cola_blocked->elements, (void*)hilo_encontrado);

	list_add(programa->hilos_en_ready, hilo);
	hilo->tiempo_ultima_llegada_a_ready = tiempo_actual();
	log_info(logger, "El hilo %i del programa %i llegó a READY", hilo->tid, hilo->pid);
	GRADO_MULTIPROGRAMACION++;

	if (GRADO_MULTIPROGRAMACION == MAX_MULTIPROG)
		log_warning(logger, "Se ha alcanzado el grado máximo de multiprogramación");

}

void bloquear_hilo(hilo_t* hilo) {
	printf("entra a bloquear hilo\n");

	bool hilo_encontrado(hilo_t* un_hilo) {
		return un_hilo->tid == hilo->tid && un_hilo->pid == hilo->pid;
	}

	programa_t* programa_del_hilo = obtener_programa(hilo->pid);
	list_remove_by_condition(programa_del_hilo->hilos_en_ready, hilo_encontrado);
	queue_push(cola_blocked, hilo);
	log_info(logger, "El hilo %i del programa %i llegó a BLOCKED", hilo->tid, hilo->pid);
}

hilo_t* siguiente_hilo_a_ejecutar(programa_t* programa) {
	hilo_t* siguiente = malloc(sizeof(hilo_t));
	t_list* hilos = list_create();
	hilos = programa->hilos_en_ready;

	double estimacion(hilo_t* hilo) {
		return (1 - ALPHA_SJF) * hilo->estimacion_anterior + ALPHA_SJF * hilo->rafaga_anterior;
	}

	bool comparador(hilo_t* hilo1, hilo_t* hilo2) {
		return estimacion(hilo1) <= estimacion(hilo2);
	}

	list_sort(hilos, comparador);
	siguiente = list_get(hilos, 0);
	return siguiente;
}

void liberar() {

	void liberar_programa(programa_t* programa) {
		free(programa);
	}

	void liberar_hilos_programa(programa_t* programa) {
		list_destroy(programa->hilos_en_ready);
		free(programa->hilo_en_exec);
	}

	config_destroy(config);
	log_destroy(logger);
	log_destroy(logger_metricas);
	sem_destroy(multiprogramacion_sem);
	sem_destroy(atender_clientes_sem);
	list_iterate(programas, liberar_hilos_programa);
	list_destroy_and_destroy_elements(programas, liberar_programa);
}


















