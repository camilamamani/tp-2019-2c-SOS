#include <hilolay/alumnos.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "libSUSE.h"

int max_tid = 0;

void* serializar(ult_t ult) {
	void* buffer = malloc(sizeof(ult_t));
	memcpy(buffer, &ult.pid, sizeof(int));
	memcpy(buffer + sizeof(int), &ult.tid, sizeof(int));
}

void enviar_datos_ult(int tid, int pid, int socket) {
	ult_t ult;
	ult.tid = tid;
	ult.pid = pid;
	void* buffer = serializar(ult);
	send(socket, buffer, 100, 0);
}

int suse_create(int tid){
	  ult_t ult;
	  struct sockaddr_in cliente;
	  struct hostent *servidor;
	  servidor = gethostbyname("localhost");

	  if(servidor == NULL)
	  {
	    printf("Host erróneo\n");
	    return 1;
	  }

	  char buffer[100];
	  int conexion = socket(AF_INET, SOCK_STREAM, 0);
	  bzero((char *)&cliente, sizeof((char *)&cliente));

	  cliente.sin_family = AF_INET;
	  cliente.sin_port = htons(8000);
	  bcopy((char *)servidor->h_addr, (char *)&cliente.sin_addr.s_addr, sizeof(servidor->h_length));

	  if(connect(conexion,(struct sockaddr *)&cliente, sizeof(cliente)) < 0)
	  {
	    printf("Error conectando con el host\n");
	    close(conexion);
	    return 1;
	  }

	  //printf("ingresar TID: ");
	  //fgets(buffer, 100, stdin);
	  enviar_datos_ult(tid, getpid(), conexion);
	  //send(conexion, buffer, 100, 0);
	  bzero(buffer, 100);
	  recv(conexion, buffer, 100, 0);
	  printf("%s", buffer);
}

int suse_schedule_next(void){
	int next = max_tid;
	printf("Scheduling next item %i...\n", next);
	return next;
}

int suse_join(int tid){
	// Not supported
	return 0;
}

int suse_close(int tid){
	printf("Closed thread %i\n", tid);
	max_tid--;
	return 0;
}

static struct hilolay_operations hiloops = {
		.suse_create = &suse_create,
		.suse_schedule_next = &suse_schedule_next,
		.suse_join = &suse_join,
		.suse_close = &suse_close
};

void hilolay_init(void){
	init_internal(&hiloops);
}
