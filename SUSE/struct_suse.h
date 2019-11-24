#ifndef STRUCT_SUSE_H_
#define STRUCT_SUSE_H_

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>


typedef struct {
	int tid;
	int pid;
	int tiempo_ejecucion;
	int tiempo_espera;
	int tiempo_cpu;
	float estimacion_anterior;
	float rafaga_anterior;
} __attribute__((packed)) hilo_t;

typedef struct {
	int pid;
	t_queue* cola_ready;
	hilo_t* hilo_en_exec;
} programa_t;

typedef struct {
	char* id;
	int valor;
} semaforo_t;

t_log* logger;
t_log* logger_metricas;

// ver si no es doble puntero
t_queue* cola_new;
t_queue* cola_blocked;
t_queue* cola_exit;
programa_t* programas;
t_queue* colas_ready[100];
t_queue* colas_exec[100];

sem_t* tid_sem;
sem_t* pid_sem;
sem_t* multiprogramacion_sem;

t_dictionary* diccionario_programas;
t_dictionary* diccionario_tid_pid;
t_dictionary* diccionario_tid;


#endif /* STRUCT_SUSE_H_ */
