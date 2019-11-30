#include "libSUSE.h"

int max_tid = 0;
int cant_proceso = 0;


/* ver si no se mueve en otro parte */
void conectarse_a_suse() {

	socket_t server;
	char *ip = "127.0.0.1";
	int puerto = 8524;

	log_iniciar("libsuse.log", "libsuse", true);
	log_msje_info("Iniciando libSUSE");

	server.fd = crear_conexion(ip, puerto);

	log_msje_info("Socket [ %d ] conectado a [ %s : %d  ]", server.fd, ip, puerto);
	set_server_suse(server);

	handshake_enviar(server.fd, COD_PROCESO);
	handshake_recibir(server.fd);

//	socket_liberar(server.fd);

}


int libsuse_create(int tid){

	log_msje_info("Operacion CREATE para el hilo: [%i]", tid);

	package_t paquete, respuesta;

//	se serializa el paquete a enviar
	paquete = slz_cod_comun(tid, COD_CREATE);

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
}

int libsuse_schedule_next(void){
	log_msje_info("Operacion SCHEDULE_NEXT para saber el prox. hilo");

		package_t paquete, respuesta;

	//	se serializa el paquete a enviar
		paquete = slz_cod_schedule_next();

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
		log_msje_info("libSUSE determino el proximo hilo");
		return 0;
}

int libsuse_join(int tid){
	log_msje_info("Operacion JOIN para el hilo: [%i]", tid);

	package_t paquete, respuesta;

//	se serializa el paquete a enviar
	paquete = slz_cod_comun(tid, JOIN);

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
	log_msje_info("libSUSE  JOIN PARA EL HILO DE ID: [ %i ]", tid);

	return 0;
}

int libsuse_close(int tid){
	log_msje_info("Operacion CLOSE para el hilo: [%i]", tid);

	package_t paquete, respuesta;

//	se serializa el paquete a enviar
	paquete = slz_cod_comun(tid, CLOSE);

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
	log_msje_info("libSUSE CLOSE EL HILO DE ID: [ %i ]", tid);

	return 0;
}

int libsuse_wait(int tid, char *sem_name){

	log_msje_info("Operacion WAIT para el hilo: [%i] de semaforo: [%s]", tid, sem_name);

	package_t paquete, respuesta;

//	se serializa el paquete a enviar
	paquete = slz_cod_wait(tid, sem_name);

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
	log_msje_info("libSUSE WAIT EN EL SEMAFORO: [%s] PARA EL HILO DE ID: [ %i ]", sem_name, tid);

	return 0;
}

int libsuse_signal(int tid, char *sem_name){
	log_msje_info("Operacion SIGNAL para el hilo: [%i] de semaforo: [%s]", tid, sem_name);

	package_t paquete, respuesta;

//	se serializa el paquete a enviar
	paquete = slz_cod_wait(tid, sem_name);

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
	log_msje_info("libSUSE SIGNAL EN EL SEMAFORO: [%s] PARA EL HILO DE ID: [ %i ]", sem_name, tid);

	return 0;
}

static struct hilolay_operations hiloops = {
		.suse_create = &libsuse_create,
		.suse_schedule_next = &libsuse_schedule_next,
		.suse_wait = &libsuse_wait,
		.suse_signal = &libsuse_signal,
		.suse_join = &libsuse_join,
		.suse_close = &libsuse_close
};

void hilolay_init(void)
{
	conectarse_a_suse();

	init_internal(&hiloops);

}

void set_server_suse(socket_t fd)
{
	suse_server = fd;
}


