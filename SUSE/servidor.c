#include "servidor.h"

void atender_conexiones(int cliente_fd)
{
	package_t paquete;
	char *buffer;
	int tid;

	while(true){
		paquete = paquete_recibir(cliente_fd);
		switch(paquete.header.cod_operacion){
			case COD_HANDSHAKE:
				handshake_enviar(cliente_fd, 'S');
				break;
			case COD_CREATE:
				log_msje_info("Me llego operacion create");
				dslz_payload_with_tid(paquete.payload, &tid);
				suse_create(tid, cliente_fd);
				break;
			default:
				log_msje_error("Codigo de operacion desconocido");
				break;
		}
	}


}


void servidor_iniciar()
{
	int suse_fd = crear_servidor(LISTEN_PORT);
	log_msje_info("Se creo el socket servidor [ %d ] en el puerto [ %d ]", suse_fd, LISTEN_PORT);

	socket_t cliente;
	pthread_t thread_conexiones;

	while(true)
	{
		cliente.fd = aceptar_cliente(suse_fd);

		if(cliente.fd != -1)
		{
			log_msje_info("Se conecto socket [ %d ]", cliente.fd);

			pthread_create(&thread_conexiones,NULL, (void*)atender_conexiones, (void *)cliente.fd);
			pthread_join(thread_conexiones, NULL);
		}
		else
		{
			log_msje_error("No se conecto socket [ %d ]", cliente.fd);
		}
	}

	socket_liberar(suse_fd);
}


