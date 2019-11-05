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
#include <string.h>

#include "configuracion.h"

typedef enum
{
	MENSAJE,
	PAQUETE
} op_code;

typedef struct {
	int pid;
	int tid;
} ult_t;

t_log* logger;

void* recibir_buffer(int*, int);
int iniciar_servidor(void);
int esperar_cliente(int);
ult_t* recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);

#endif /* UTILS_H_ */
