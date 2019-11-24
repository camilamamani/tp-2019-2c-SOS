#ifndef SERVIDOR_H_
#define SERVIDOR_H_

#include <stdlib.h>
#include <pthread.h>
#include "protocol.h"
#include "log.h"
#include "utils.h"
#include "configuracion.h"
#include "socket.h"


#define COD_PROCESO 'S'

void servidor_iniciar();


#endif /* SERVIDOR_H_ */
