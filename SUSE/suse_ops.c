#include "suse_ops.h"

hilo_t *hilo_next;
int multiprogramacion = 0;

/* operaciones de suse*/

void suse_create(int tid, int cliente_fd){
	log_msje_info("SUSE CREATE tid = [ %i ]", tid);
	package_t paquete;
	int res, err;


	hilo_t *hilo;
	hilo = malloc(sizeof(hilo_t));

	res = create_hilo(tid, &hilo);

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
int create_hilo(int tid, hilo_t *hilo)
{
	int pid = getpid();

	crear_nuevo_hilo(tid, pid, &hilo);

	agregar_hilo_a_new(&hilo);


	return 0;
}


//
int proximo_hilo()
{
	hilo_t *next;

		if(queue_is_empty(colas_exec) == 0 && queue_is_empty(colas_ready) != 0){

			next = (hilo_t *)queue_pop(colas_ready);
			next->time_enter_exec = clock();

			queue_push(colas_exec, &next);
		}
	alg_planificacion_sjf(&next);


	return 0;
}

void alg_planificacion_sjf(hilo_t * next)
{


	hilo_t * aux;
	int i;
	int cant_ready = queue_size(colas_ready);

	for(i = 0; i < cant_ready; i++)
	{
		aux = (hilo_t *) queue_pop(colas_ready);

		estimacion_sjf(&aux);

		queue_push(colas_ready, aux);
	}

	hilo_prioritario(&next);


}

void estimacion_sjf(hilo_t * hilo)
{
	float resultado;
	float estimacion_ant;
	float rafaga;

	estimacion_ant= hilo->estimacion_anterior;
	rafaga = hilo->rafaga_anterior;

	resultado = ALPHA_SJF*rafaga + (1 - ALPHA_SJF) * estimacion_ant;
	
	hilo->estimacion_anterior = resultado;
	
}

void hilo_prioritario(hilo_t * hilo_candidato){

	int cant_ready = (int) queue_size(colas_ready);
	int i;
	int max= 0;

	t_queue* cola_aux = queue_create();
	cola_aux = (t_queue*) colas_ready;

	for(i=0; i<cant_ready; i++)
	{
		hilo_candidato = (hilo_t *)queue_pop(cola_aux);

		if(hilo_candidato->estimacion_anterior > max)
		{
			max = hilo_candidato->estimacion_anterior;
		}

	}


}





