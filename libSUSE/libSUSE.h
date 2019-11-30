#ifndef LIBSUSE_H_
#define LIBSUSE_H_

#include <hilolay/alumnos.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>

#include "log_suse.h"
#include "protocol.h"
#include "socket.h"

#define COD_PROCESO 'C'

socket_t suse_server;


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

t_dictionary* diccionario_programas;
t_dictionary* diccionario_tid_pid;
t_dictionary* diccionario_tid;

void* serializar(hilo_t);

void set_server_suse(socket_t fd);
//int ejecutar_operacion(int, int);

void hilolay_init(void);
int libsuse_create(int tid);
int libsuse_schudule_next(void);
int libsuse_wait(int tid, char*);
int libsuse_signal(int tid, char*);
int libsuse_join(int tid);
int libsuse_close(int tid);



#endif /* LIBSUSE_H_ */
