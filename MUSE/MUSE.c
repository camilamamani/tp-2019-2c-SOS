#include "MUSE.h"

void respond_init(muse_body* body, char* id, int socket_cliente) {
	log_debug(logger, "El cliente con id: %s hizo init", id);
	process_table* table = get_table_for_process(id);

	if (table == NULL) {
		create_process_table(id);
		log_debug(logger, "Se creo una process_table para %s", id);
		send_response_status(socket_cliente, SUCCESS);
	} else {
		log_error(logger, "Ya hay una tabla de segmentos para este proceso");
		send_response_status(socket_cliente, ERROR);
	}
}

void respond_alloc(muse_body* body, char* id, int socket_cliente) {
	uint32_t tam_pedido;
	memcpy(&tam_pedido, body->content, sizeof(uint32_t));

	log_debug(logger, "El cliente con id: %s hizo muse_malloc de %u", id, tam_pedido);

	process_table* table = get_table_for_process(id);
	process_segment *segment;

	muse_body* r_body = create_body();
	muse_response* response;
	void* dir;

	int pages_needed;

	if (table != NULL) {
		if (table->number_of_segments == 0) {
			pages_needed = ceil((double) (tam_pedido + metadata_size * 2) / PAGE_SIZE);
			int new_base = last_position(id);
			segment = create_segment(HEAP, new_base);

			for (int var = 0; var < pages_needed; var++) {
				t_page *new_page = create_page();
				asignar_frame(new_page);
				add_page_to_segment(segment, new_page);
			}
			dir = alloc_in_segment(segment, 0, tam_pedido);

			add_process_segment(id, segment);
		} else {
			segment = find_segment_with_space(table, tam_pedido);

			if (segment != NULL) {
				int free_dir = find_free_dir(segment, tam_pedido);
				dir = alloc_in_segment(segment, free_dir, tam_pedido);

			} else {
				segment = find_extensible_heap_segment(table);

				if (segment != NULL) {
					int free_space = free_space_at_the_end(segment);

					// tamaño pedido - el espacio que hay + metadata para lo que queda
					// (ya se tiene en cuenta el espacio para la metadata de este bloque porque ya existe)
					pages_needed = ceil((double) (tam_pedido - free_space + metadata_size) / PAGE_SIZE);

					// esto lo puse antes y con esa cuenta loca porque en si los frames no estan
					// inicializados (con metadata) no podemos saber cuanto espacio queda...
					// HAY QUE CAMBIARLO...
					int free_dir = find_free_dir(segment, free_space - metadata_size);

					for (int var = 0; var < pages_needed; var++) {
						t_page *new_page = create_page();
						asignar_frame(new_page);
						add_page_to_segment(segment, new_page);
					}
					dir = alloc_in_segment(segment, free_dir, tam_pedido);
				} else {
					pages_needed = ceil((double) (tam_pedido + metadata_size * 2) / PAGE_SIZE);

					int new_base = last_position(id);

					segment = create_segment(HEAP, new_base);
					for (int var = 0; var < pages_needed; var++) {
						t_page *new_page = create_page();
						asignar_frame(new_page);
						add_page_to_segment(segment, new_page);
					}
					int free_dir = find_free_dir(segment, tam_pedido);
					dir = alloc_in_segment(segment, free_dir, tam_pedido);
					add_process_segment(id, segment);
				}
			}
		}
		add_fixed_to_body(r_body, sizeof(uint32_t), segment->base + dir);
		response = create_response(SUCCESS, r_body);
	} else {
		log_error(logger, "El proceso %s no tiene tabla");
		response = create_response(ERROR, r_body);
	}
	send_response(response, socket_cliente);
}

void respond_get(muse_body* body, char* id, int socket_cliente) {
	uint32_t src;
	memcpy(&src, body->content, sizeof(uint32_t));
	size_t size;
	memcpy(&size, body->content + sizeof(uint32_t), sizeof(size_t));
	void* val = malloc(size);

	log_debug(logger, "El cliente con id: %s hizo get a la src: %u de %i bytes", id, src, size);

	process_table* table = get_table_for_process(id);
	muse_body* r_body = create_body();
	muse_response* response;

	if (table != NULL) {
		if (table->number_of_segments == 0) {
			log_error(logger, "El proceso %s no tiene segmentos");
			response = create_response(ERROR, r_body);
		} else {
			process_segment *segment = segment_by_dir(table, src);
			val = get_from_dir(segment, src, size);
			add_to_body(r_body, size, val);

			response = create_response(SUCCESS, r_body);
		}
	} else {
		log_error(logger, "El proceso %s no tiene tabla");
		response = create_response(ERROR, r_body);
	}
	send_response(response, socket_cliente);
}

void respond_free(muse_body* body, char* id, int socket_cliente) {
	uint32_t dir_to_free;
	memcpy(&dir_to_free, body->content, sizeof(uint32_t));

	log_debug(logger, "El cliente con id: %s hizo free a la dir: %u", id, dir_to_free);

	process_table* table = get_table_for_process(id);

	if (table != NULL) {
		if (table->number_of_segments == 0) {
			log_error(logger, "El proceso %s no tiene segmentos");
			send_response_status(socket_cliente, ERROR);
		} else {
			process_segment *segment = segment_by_dir(table, dir_to_free);
			free_dir(segment, dir_to_free);

			send_response_status(socket_cliente, SUCCESS);
		}
	} else {
		log_error(logger, "El proceso %s no tiene tabla");
		send_response_status(socket_cliente, ERROR);
	}
}

void respond_cpy(muse_body* body, char* id, int socket_cliente) {	
	uint32_t dst;
	memcpy(&dst, body->content, sizeof(uint32_t));	
	int size;
	memcpy(&size, body->content + sizeof(uint32_t), sizeof(int));
	void** val = malloc(size);
	memcpy(val, body->content + sizeof(uint32_t) + sizeof(int), size);

	log_debug(logger, "El cliente con id: %s hizo cpy a dst: %u, %i bytes", id, dst, size);
	process_table* table = get_table_for_process(id);

	if (table != NULL) {
		process_segment *segment;
		if (table->number_of_segments == 0) {
			send_response_status(socket_cliente, ERROR);
			log_error(logger, "El proceso %s no tiene segmentos");
		} else {
			segment = segment_by_dir(table, dst);
			int dir_de_pagina = dst - segment->base;

			cpy_to_dir(segment, dir_de_pagina, val, size);
			send_response_status(socket_cliente, SUCCESS);
		}
	} else {
		log_error(logger, "El proceso %s no tiene tabla");
		send_response_status(socket_cliente, ERROR);
	}
}

void respond_map(muse_body* body, char* id, int socket_cliente) {
	int path_size;
	memcpy(&path_size, body->content, sizeof(int));
	char* path = malloc(path_size * sizeof(char));
	memcpy(path, body->content + sizeof(int), path_size);
	
	size_t length;
	memcpy(&length, body->content + sizeof(int) + path_size, sizeof(size_t));
	map_flag flags;
	memcpy(&flags, body->content + sizeof(int) + path_size + sizeof(size_t), sizeof(int));

	log_debug(logger, "El cliente con id: %s hizo map a: %s, de %i bytes, flag: %i", id, path, length, flags);

	process_table* table = get_table_for_process(id);
	muse_body* r_body = create_body();
	muse_response* response;

	void* dir;
	// min amount of frames to ask for the segment
	int frames_to_ask = ceil((double) (length + metadata_size) / PAGE_SIZE);
	process_segment *segment;
	if (table != NULL) {
		log_info(logger, "Se crea un segmento para hacer un map de %i", length);
		int new_base = last_position(id);
		segment = create_segment(MMAP, new_base);
		for (int var = 0; var < frames_to_ask; var++) {
			t_page *new_page = create_page();
			asignar_frame(new_page);
			add_page_to_segment(segment, new_page);
		}
		int free_dir = find_free_dir(segment, length);
		dir = alloc_in_segment(segment, free_dir, length);
		add_process_segment(id, segment);

		add_fixed_to_body(r_body, sizeof(uint32_t), segment->base + dir);
		response = create_response(SUCCESS, r_body);
		send_response(response, socket_cliente);
	} else {
		log_info(logger, "El proceso %s no tiene tabla, falto hacer init?");
		send_response_status(socket_cliente, ERROR);
	}
	return;
}

void respond_sync(muse_body* body, char* id, int socket_cliente) {
	uint32_t addr;
	memcpy(&addr, body->content, sizeof(uint32_t));
	size_t len;
	memcpy(&len, body->content + sizeof(uint32_t), sizeof(size_t));

	// log_info(logger, "El cliente con id: %s hizo sync a la addr: %u de %i bytes", id, addr, len);

	return;
}

void respond_unmap(muse_body* body, char* id, int socket_cliente) {
	uint32_t addr;
	memcpy(&addr, body->content, sizeof(uint32_t));

	// log_info(logger, "El cliente con id: %s hizo unmap a la addr: %u", id, addr);

	return;
}

int respond_to_client(int cliente_fd) {
	while(1) {
		int cod_op = recv_muse_op_code(cliente_fd);
		char* id = recv_muse_id(cliente_fd);
		muse_body* body = recv_body(cliente_fd);
		switch(cod_op) {
			case INIT_MUSE:
				respond_init(body, id, cliente_fd);
				break;
			case ALLOC:
				respond_alloc(body, id, cliente_fd);
				break;
			case FREE:
				respond_free(body, id, cliente_fd);
				break;
			case GET:
				respond_get(body, id, cliente_fd);
				break;
			case CPY:
				respond_cpy(body, id, cliente_fd);
				break;
			case MAP:
				respond_map(body, id, cliente_fd);
				break;
			case SYNC:
				respond_sync(body, id, cliente_fd);
				break;
			case UNMAP:
				respond_unmap(body, id, cliente_fd);
				break;
			case DISCONNECT_MUSE:
				return EXIT_FAILURE;
				break;
			default:
				break;
		}
	}
}

int main(void) {
	logger = log_create("../logs/MUSE.log", "Servidor", 1, LOG_LEVEL_DEBUG);
	config = config_create("../program.config");

	IP = "127.0.0.1";
	PORT = config_get_string_value(config, "LISTEN_PORT");
	MEMORY_SIZE = config_get_int_value(config, "MEMORY_SIZE");
	PAGE_SIZE = config_get_int_value(config, "PAGE_SIZE");
	SWAP_SIZE = config_get_int_value(config, "SWAP_SIZE");

	log_info(logger, "Port: %s", PORT);
	log_info(logger, "Memory size: %i", MEMORY_SIZE);
	log_info(logger, "Page size: %i", PAGE_SIZE);
	log_info(logger, "Number of pages: %i", MEMORY_SIZE / PAGE_SIZE);

	int server_fd = init_server(IP, PORT);
	
	if (server_fd == -1) {
		return EXIT_FAILURE;
	}

	init_structures();

	pthread_t hilo;
	int r1;
	while (1) {
		int cliente_fd = recibir_cliente(server_fd);
	    r1 = pthread_create(&hilo, NULL, (void*) respond_to_client, (void*) cliente_fd);
	}

	fclose(swap_file);
	return EXIT_SUCCESS;
}
