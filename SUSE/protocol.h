#ifndef COMMON_PROTOCOL_H_
#define COMMON_PROTOCOL_H_

#include <stdlib.h>
#include "log.h"
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include "socket.h"
#include <dirent.h>

#include <commons/collections/list.h>

#define TAM_HEADER 9

typedef enum{
    COD_HANDSHAKE,
	COD_DESC,
	COD_ERROR,
	COD_CREATE,
	SCHEDULE_NEXT,
	JOIN,
	CLOSE,
	WAIT,
	SIGNAL
}op_code;

typedef struct{
    char cod_proceso;
    int cod_operacion;
    int tam_payload;
}__attribute__((packed))
header_t;

typedef struct{
	header_t header;
    void * payload;
}package_t;

typedef struct{
    int socket;
}pthread_data;

void handshake_recibir(int socket);
void handshake_enviar(int socket, char cod_proc);

package_t paquete_recibir(int socket);
bool paquete_enviar(int socket, package_t paquete);


/* serializacion en comun*/
package_t slz_res_error(int errnum);
package_t slz_simple_res(op_code cod);

void dslz_payload_with_tid(void *buffer, int *tid);
void dslz_payload_with_tid_semaforo(void *buffer, int * tid, char** semaforo);



//operaciones libsuse (cliente)
package_t slz_cod_create(int tid);

void dslz_res_create();

//operacione suse (servidor)




#endif /* COMMON_PROTOCOL_H_ */
