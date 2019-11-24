#include "libSUSE.h"

int max_tid = 0;
int cant_proceso = 0;

//esto lo hace suse
hilo_t* crear_nuevo_hilo(int tid, int pid) {
	hilo_t* hilo = malloc(sizeof(hilo_t));
	hilo->tid = tid;
	hilo->pid = pid;
	hilo->tiempo_ejecucion = 0;
	hilo->tiempo_espera = 0;
	hilo->tiempo_cpu = 0;
	hilo->estimacion_anterior = 0;
	hilo->rafaga_anterior = 0;
	return hilo;
}

int ejecutar_operacion(int tid, int operacion) {
 return 0;
}
/*
int ejecutar_operacion(int tid, int operacion) {
	int conexion = conectarse_a_suse();
	int pid = getpid();
	t_paquete* paquete = crear_paquete(operacion);
	hilo_t* hilo = crear_nuevo_hilo(tid, pid);
	agregar_a_paquete(paquete, hilo, sizeof(hilo_t));
	enviar_paquete(paquete, conexion);

	if (operacion == 2) {
		int tid_proximo_hilo;
		recv(conexion, &tid_proximo_hilo, sizeof(int), 0);
		return tid_proximo_hilo;
	}

	return 0;
}
*/

void conectarse_a_suse() {
//	struct sockaddr_in cliente;
//	struct hostent *servidor;
//	servidor = gethostbyname("127.0.0.1");
//
//	if(servidor == NULL)
//	{
//	  printf("Host erróneo\n");
//	  return 1;
//	}
//
//	char buffer[100];
//	int conexion = socket(AF_INET, SOCK_STREAM, 0);
//	bzero((char *)&cliente, sizeof((char *)&cliente));
//
//	cliente.sin_family = AF_INET;
//	cliente.sin_port = htons(8524);
//	bcopy((char *)servidor->h_addr, (char *)&cliente.sin_addr.s_addr, sizeof(servidor->h_length));
//
//	if(connect(conexion,(struct sockaddr *)&cliente, sizeof(cliente)) < 0)
//	{
//	  printf("Error conectando con el host\n");
//	  close(conexion);
//	  return 1;
//	}

	char * ip = "127.0.0.1";
	char * puerto = "8524";

	log_iniciar("libsuse.log", "libsuse", true);
	log_msje_info("Iniciando libSUSE");

	suse_server.fd = crear_conexion(ip, puerto);

	log_msje_info("Socket [ %d ] conectado a [ %s:%d  ]", suse_server.fd, ip, puerto);

	handshake_enviar(suse_server.fd, COD_PROCESO);
	handshake_recibir(suse_server.fd);

	socket_liberar(suse_server.fd);

}


int libsuse_create(int tid){
	log_msje_info("Operacion CREATE para el hilo: [%i]", tid);
//este if creo que va del lado del server
	if (tid > max_tid) max_tid = tid;

	package_t paquete, respuesta;

//	se serializa el paquete a enviar
	paquete = slz_cod_create(tid);

//	se valida se envio el paquete
	if(!paquete_enviar(suse_server.fd, paquete))
		log_msje_error("No se pudo enviar el paquete");
	else
		log_msje_info("Se envio operacion create al server");

//	espero a que me responda el server SUSE
	respuesta = paquete_recibir(suse_server.fd);

//	valido lo que resivo de SUSE
	if(respuesta.header.cod_operacion == COD_ERROR){
		int err;
		log_msje_error("create me llego cod error");
		dslz_res_error(respuesta.payload, &err);
		return -err;
	}

//	sila respuesta el OK genero un log
	log_msje_info("libSUSE CREE UN HILO DE ID: [ %i ]", tid);
	return 0;

//	return ejecutar_operacion(tid, 1);
}

int suse_schedule_next(void){
	int next = max_tid;
//	next = ejecutar_operacion(next, 2);
	return next;
	//printf("suse_schedule_next() (hilo %i)\n", next);
	//SUSE debe devolver el TID del próximo hilo a planificar.
}

int suse_join(int tid){
	//printf("suse_join(%i)\n", tid);
	return ejecutar_operacion(tid, 3);
}

int suse_close(int tid){
	//printf("suse_close(%i)\n", tid);
	return ejecutar_operacion(tid, 4);
}

int suse_wait(int tid, char *sem_name){
	return 0;
}

int suse_signal(int tid, char *sem_name){
	return 0;
}

static struct hilolay_operations hiloops = {
		.suse_create = &libsuse_create,
		.suse_schedule_next = &suse_schedule_next,
		.suse_join = &suse_join,
		.suse_close = &suse_close
};

void hilolay_init(void){
	init_internal(&hiloops);
	conectarse_a_suse();
}
/*
void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;
	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	int opcode, size, tid;
	memcpy(&opcode, magic, sizeof(int));
	memcpy(&size, magic + 4, sizeof(int));
	memcpy(&tid, magic + 8, sizeof(int));

	return magic;
}
*/

/*t_paquete* crear_paquete(int operacion)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = operacion;
	crear_buffer(paquete);
	return paquete;
}
*/
/*
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);
	paquete->buffer->size += tamanio + sizeof(int);
	int opcode;
	int size;
	int val;
	memcpy(&opcode, &paquete->codigo_operacion, sizeof(int));
	memcpy(&size, &paquete->buffer->size, sizeof(int));
	memcpy(&val, valor, sizeof(int));
}


// se eliminaria
void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);
	send(socket_cliente, a_enviar, bytes, 0);
	int opcode, size, tid;
	memcpy(&opcode, a_enviar, 4);
	memcpy(&size, a_enviar + 4, 4);
	memcpy(&tid, a_enviar + 8, 4);

}

void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}
*/

