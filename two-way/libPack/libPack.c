#include "client.h"

void enviar_algo(char* IP, char* PORT, char* algo) {
	int conexion = crear_conexion(IP, PORT);

	enviar_mensaje(algo, conexion);
	int cod_op;
	int size;

	// recibir codigo de operacion, un entero que tenemos como enum
	int recv_result = recv(
		conexion,
		&cod_op,
		sizeof(int),
		MSG_WAITALL
	);

	if (recv_result != 0) {
		log_info(logger, "cod_op: %i", cod_op);
	} else {
		close(conexion);
	}

	void * buffer;

	// recibir tamaño del buffer y ponerlo en "size"
	recv(conexion, &size, sizeof(int), MSG_WAITALL);
	log_info(logger, "size: %i", size);

	buffer = malloc(size);

	// recibir buffer
	recv(conexion, buffer, size, MSG_WAITALL);
	log_info(logger, "buffer: %s", buffer);
	liberar_conexion(conexion);
}

int main(void) {
	logger = log_create("../logs/client.log", "Cliente", 1, LOG_LEVEL_INFO);
	config = config_create("../program.config");

	char* IP = config_get_string_value(config, "IP");
	char* PORT = config_get_string_value(config, "PORT");

	log_info(logger, "Conectandome a %s:%s", IP, PORT);

	enviar_algo(IP, PORT, "Hola, me conecte!");
	enviar_algo(IP, PORT, "Hola, estoy acá de nuevo!");

	log_destroy(logger);
	config_destroy(config);
	return 0;
}