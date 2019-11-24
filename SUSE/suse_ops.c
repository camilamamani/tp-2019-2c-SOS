#include "suse_ops.h"

/* operaciones de suse*/

void suse_create(int tid, int cliente_fd){
	log_msje_info("SUSE CREATE tid = [ %i ]", tid);
	package_t paquete;
	int res, err;

	res = create_hilo(tid);

	if(res == -1){
		log_msje_error("create: [ %s ]", strerror(errno));
		err = errno;
		paquete = slz_res_error(err);
	}
	else
		paquete = slz_simple_res(COD_CREATE);

	paquete_enviar(cliente_fd, paquete);

}


/* delegaciones de tareas  */
int create_hilo(int tid)
{
	int pid = getpid();
	hilo_t * hilo_new = crear_nuevo_hilo(tid,pid);

	agregar_hilo_a_new(hilo_new);

	hilo_t * hilo_v = devolver_primer_hilo(cola_new);

	printf("hilo [%i] en cola_new", queue_size(cola_new));


	return 0;
}

