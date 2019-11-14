#include "libmuse.h"

#include <commons/config.h>
#include <commons/log.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int conexion;

int muse_init(int id, char* ip, int puerto){
	logger = log_create("./logs/libmuse.log", "libmuse", 1, LOG_LEVEL_INFO);

	log_info(logger, "Conectandome a %s:%s", ip, puerto);

	conexion = create_connection(ip, puerto);

	if (conexion != -1) {
		send_code(conexion, INIT_MUSE);
		return 0;
	} else {
		return -1;
	}
}

void muse_close(){
	log_info(logger, "muse_close");

	log_destroy(logger);
	config_destroy(config);
	send_code(conexion, DISCONNECT_MUSE);

	free_connection(conexion);
}

uint32_t muse_alloc(uint32_t tam){
	log_info(logger, "muse_alloc: tam = %i", tam);
	send_int(conexion, ALLOC, tam);

	int status = recv_response_status(conexion);

	int size;
	char* buffer = recv_buffer(&size, conexion);

	log_info(logger, "dir: %s", buffer);

	char* x;
	return strtoul(buffer, &x, 10);
}

void muse_free(uint32_t dir) {
	log_info(logger, "muse_free a: %u", dir);
	send_int(conexion, FREE, dir);
}

int muse_get(void* dst, uint32_t src, size_t n){
	log_info(logger, "muse_get a: %u, de %i bytes", src, n);
	send_int(conexion, GET, src);

	int status = recv_response_status(conexion);

	int size;
	char* data = recv_buffer(&size, conexion);

	log_info(logger, "data: %s", data);

	memcpy(dst, data, 5);
    return 0;
}

int muse_cpy(uint32_t dst, void* src, int n){
    memcpy((void*) dst, src, n);
    return 0;
}

uint32_t muse_map(char *path, size_t length, int flags){
    return 0;
}

int muse_sync(uint32_t addr, size_t len){
    return 0;
}

int muse_unmap(uint32_t dir){
    return 0;
}