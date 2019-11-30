#ifndef SUSE_OPS_H_
#define SUSE_OPS_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <commons/collections/queue.h>
#include "protocol.h"
#include "struct_suse.h"
#include "log.h"
#include "utils.h"



/* operaciones de suse */
void suse_create(int, int);
void suse_schedule_next(int cliente_fd);


#endif /* SUSE_OPS_H_ */
