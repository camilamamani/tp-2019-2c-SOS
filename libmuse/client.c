#include "client.h"
#include "unistd.h"

int main(void) {
	config = config_create("./program.config");

	char* IP = config_get_string_value(config, "IP");
	char* PORT = config_get_string_value(config, "PORT");

	int init_result = muse_init((int) getpid(), IP, PORT);

	if (init_result != -1) {
		uint32_t my_memory = muse_alloc(65);
		uint32_t map_dir = muse_map("/Downloads", 80, MAP_PRIVATE);
		uint32_t another_memory = muse_alloc(35);

		log_info(logger, "my_memory: %u", my_memory);
		log_info(logger, "map_dir: %u", map_dir);
		log_info(logger, "another_memory: %u", another_memory);

		int x = 1998;
		int* y = malloc(4);

		muse_cpy(another_memory, &x, 4);
		muse_get(y, another_memory, 4);
		printf("y: %d \n", *y);

		muse_free(my_memory);

		muse_close();

		return 0;
	} else {
		return EXIT_FAILURE;
	}
}

// PRUEBA 1
//uint32_t my_memory = muse_alloc(15);
//uint32_t mem3 = muse_alloc(10);
//
//log_info(logger, "my_memory: %u", my_memory);
//log_info(logger, "mem3: %u", mem3);
//char* x = "Hola juan";
//char** y = malloc(strlen(x) + 1);
//
//muse_cpy(mem3, &x, 10);
//muse_get(y, mem3, 10);
//printf("y: %s \n", *y);
//
//muse_free(mem3);

// PRUEBA 2
//uint32_t my_memory = muse_alloc(30);
//uint32_t mem3 = muse_alloc(15);
//
//log_info(logger, "my_memory: %u", my_memory);
//log_info(logger, "mem3: %u", mem3);
//char* x = "Hola, como va?";
//char** y = malloc(strlen(x) + 1);
//
//muse_cpy(mem3, &x, 15);
//muse_get(y, mem3, 15);
//printf("y: %s \n", *y);
//
//muse_free(mem3);

// PRUEBA 3
//uint32_t my_memory = muse_alloc(30);
//uint32_t my_memory2 = muse_alloc(65);
//uint32_t another_memory = muse_alloc(95);
//
//log_info(logger, "my_memory: %u", my_memory);
//log_info(logger, "my_memory2: %u", my_memory2);
//log_info(logger, "another_memory: %u", another_memory);
//
//char* x = "Un texto cualquiera";
//uint32_t x_size = strlen(x) + 1;
//char** y = malloc(x_size);
//
//muse_cpy(my_memory, &x, x_size);
//muse_get(y, my_memory, x_size);
//printf("y: %s \n", *y);
//
//muse_cpy(my_memory2, &x, x_size);
//muse_get(y, my_memory2, x_size);
//printf("y: %s \n", *y);
