#include "utils.h"

int iniciar_servidor(char* IP, char* PORT, t_log* logger) {
	int socket_servidor;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; 

    getaddrinfo(IP, PORT, &hints, &servinfo);

    for (p = servinfo; p != NULL; p = p->ai_next) {
		socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

        if (socket_servidor == -1) {
            continue;
		}

		int bind_result = bind(socket_servidor, p->ai_addr, p->ai_addrlen);

        if (bind_result == -1) {
            close(socket_servidor);
			freeaddrinfo(servinfo);
			log_error(logger, "Fallo el bind.");
            return -1;
        }

        break;
    }

	listen(socket_servidor, SOMAXCONN);

	log_info(logger, "Servidor listo para recibir al cliente");

    freeaddrinfo(servinfo);

    return socket_servidor;
}

int esperar_cliente(int socket_servidor, void(*log)(char*)) {
	struct sockaddr_in dir_cliente;
	int tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

	log("Se conecto un cliente!");

	return socket_cliente;
}

int recibir_operacion(int socket_cliente) {
	int cod_op;
	int recv_result = recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL);

	if (recv_result != 0) {
		log_info(logger, "cod_op: %i", cod_op);
		return cod_op;
	} else {
		close(socket_cliente);
		return -1;
	}
}

char* recibir_buffer(int socket_cliente) {
	int size;
	void * buffer;

	recv(socket_cliente, &size, sizeof(int), MSG_WAITALL);
	log_info(logger, "size: %i", size);
	buffer = malloc(size);
	recv(socket_cliente, buffer, size, MSG_WAITALL);
	log_info(logger, "buffer: %s", buffer);
	return buffer;
}

int respond_to_client(int cliente_fd, t_log* logger) {
	char* mensaje_buffer;

	while(1) {
		int cod_op = recibir_operacion(cliente_fd);
		switch(cod_op) {
			case MENSAJE:
				recibir_buffer(cliente_fd);
				break;
			case LIST_FILES:
				send(cliente_fd, "1", 2 + 2 * sizeof(int), 0);
				break;
			case DESCONECTAR:
				log_info(logger, "El cliente se desconecto.");
				return EXIT_FAILURE;
				break;
			default:
				log_warning(logger, "Operacion desconocida. No quieras meter la pata");
				break;
		}
	}
}