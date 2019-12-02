/*
 * estructuras.h
 *
 *  Created on: 29 nov. 2019
 *      Author: utnso
 */

#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include <commons/collections/list.h>
#include <commons/collections/queue.h>

typedef struct {
	int tid;
	int pid;
	long long tiempo_espera;
	long long tiempo_cpu;
	long long tiempo_creacion;
	long long tiempo_ultima_llegada_a_ready;
	long long tiempo_ultima_llegada_a_exec;
	double estimacion_anterior;
	double rafaga_anterior;
	t_list* hilos_a_esperar;
} __attribute__((packed)) hilo_t;

typedef struct {
	int pid;
	t_list* hilos_en_ready;
	hilo_t* hilo_en_exec;
} programa_t;

typedef enum {
	CREATE,
	SCHEDULE_NEXT,
	JOIN,
	CLOSE,
	WAIT,
	SIGNAL
} operacion;

#endif /* ESTRUCTURAS_H_ */
