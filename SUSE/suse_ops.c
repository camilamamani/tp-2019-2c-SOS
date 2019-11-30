#include "suse_ops.h"

hilo_t *hilo_next;

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

void suse_schedule_next(int cliente_fd){
	log_msje_info("SUSE SCHUDULE_NEXT");
	package_t paquete;
	int res, err;

	res = proximo_hilo();

	if(res == -1){
		log_msje_error("schudule_next: [ %s ]", strerror(errno));
		err = errno;
		paquete = slz_res_error(err);
	}
	else
		paquete = slz_simple_res(SCHEDULE_NEXT);

	paquete_enviar(cliente_fd, paquete);
}


/* delegaciones de tareas  */
int create_hilo(int tid)
{
	int pid = getpid();
	hilo_t * hilo_new = crear_nuevo_hilo(tid,pid);

	agregar_hilo_a_new(hilo_new);

	hilo_t * hilo_v = devolver_primer_hilo(cola_new);

//	printf("hilo [%i] en cola_new", queue_size(cola_new));


	return 0;
}


//
int proximo_hilo()
{
	hilo_t *next;

		if(queue_is_empty(colas_exec) == 0 && queue_is_empty(colas_ready) != 0){
			next = (hilo_t *)queue_pop(colas_ready);
			next->time_enter_exec = clock();

			queue_push(colas_exec, next);
		}
	next = algoritmo_sjf();


	return 0;
}

hilo_t* algoritmo_sjf()
{

	hilo_t * next;
	hilo_t ** aux;
	int i;
	int cant_ready = queue_size(colas_ready);
	
	for(i = 0; i < cant_ready; i++)
	{
		(*aux) = queue_pop(colas_ready);

		estimacion_sjf(&aux);

		queue_push(colas_ready, (* aux));
	}

	next = (hilo_t*) hilo_prioritario();

	return next;
}

void estimacion_sjf(hilo_t ** hilo)
{
	float resultado;
	float estimacion_ant;
	float rafaga;

	estimacion_ant= (*hilo)->estimacion_anterior;
	rafaga = (*hilo)->rafaga_anterior;

	resultado = ALPHA_SJF*rafaga + (1 - ALPHA_SJF) * estimacion_ant;
	
	(*hilo)->estimacion_anterior = resultado;
	
}

hilo_t* hilo_prioritario()
{
	hilo_t * hilo_candidato;
	int cant_ready = queue_size(colas_ready);
	int i;
	int max= 0;

	t_queue* cola_aux = queue_create();
	cola_aux = colas_ready;
	
	for(i=0; i<cant_ready; i++)
	{
		hilo_candidato = queue_pop(cola_aux);

		if(hilo_candidato->estimacion_anterior > max)
		{
			max = hilo_candidato->estimacion_anterior;
		}

	}

	return hilo_candidato;
}





