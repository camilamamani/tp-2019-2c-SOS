/*
 * utils.h
 *
 *  Created on: 4 nov. 2019
 *      Author: utnso
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <string.h>
#include "configuracion.h"
#include <errno.h>
#include "struct_suse.h"
//
#include "protocol.h"



typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct {
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

t_log* logger;

void* recibir_buffer(int*, int);
int iniciar_servidor(void);
int esperar_cliente(int);
t_paquete* recibir_paquete(int);
void recibir_mensaje(int);
int recibir_cod_op(int);

//
void agregar_hilo_a_new(hilo_t **hilo_new);

hilo_t * devolver_primer_hilo(t_queue* cola);
void crear_nuevo_hilo(int tid, int pid, hilo_t ** hilo_new);


#endif /* UTILS_H_ */
