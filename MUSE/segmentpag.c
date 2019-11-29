#include "segmentpag.h"

bool t = true;
bool f = false;

void init_structures(int m_size, int p_size) {
	int frames = m_size / p_size;
	tables = list_create();
	seg_logger = log_create("./logs/segmentpag.log", "segmentpag", 1, LOG_LEVEL_DEBUG);
	int bitmap_size_in_bytes = ceil((double) frames / 8);
	bitmap_pointer = malloc(bitmap_size_in_bytes);
	frame_usage_bitmap = bitarray_create_with_mode(bitmap_pointer, bitmap_size_in_bytes, LSB_FIRST);

	MEMORY = calloc(frames, p_size);
	int i;
	for(i = 0; i < frames; i++) {
		void* new_frame = malloc(p_size);
		bool is_free = true;
		uint32_t size = p_size - sizeof(bool) - sizeof(uint32_t);
		memcpy(new_frame, &is_free, sizeof(bool));
		memcpy(new_frame + sizeof(bool), &size, sizeof(uint32_t));
		*(MEMORY + i) = new_frame;
		// free(new_frame);
	}
}

t_page *create_page(int frame_number) {
	if (bitarray_test_bit(frame_usage_bitmap, frame_number)) {
		log_error(seg_logger, "Se está queriendo crear una página con el frame: %i que ya está en uso", frame_number);
		return NULL; // explotar acá!
	} else {
		t_page * page = malloc(sizeof(t_page));
		page->flag = true;
		page->frame_number = frame_number;
		page->in_use = 1;
		page->modified = 0;
		log_debug(seg_logger, "Nueva pagina creada relacionada con el frame %i", frame_number);
		bitarray_set_bit(frame_usage_bitmap, frame_number);
		return page;
	}
}

process_segment *create_segment(segment_type type, uint32_t base) {
	process_segment *segment = malloc(sizeof(process_segment));
	segment->type = type;
	segment->base = base;
	segment->size = 0;
	segment->pages= NULL;
	log_debug(seg_logger, "Nuevo segmento (%s) creado. base: %i, size: %i", type == HEAP ? "HEAP" : "MMAP", base, 0);
	return segment;
}

void add_page_to_segment(process_segment* segment, t_page* page) {
	void* pages = segment->pages;
	int number_of_pages = segment->size / PAGE_SIZE;
	if (pages == NULL) {
		segment->pages = malloc(sizeof(t_page));
	}
	int offset = number_of_pages * sizeof(t_page);
	realloc(segment->pages, pages + offset + sizeof(t_page));
	memcpy(segment->pages + offset, page, sizeof(t_page));
	segment->size += PAGE_SIZE;
	log_debug(seg_logger, "Se agrego una página al segmento con base: %i, nuevo size: %i", segment->base, segment->size);
}

process_segment* find_segment_with_space(process_table* table , int size) {
	int is_heap(process_segment* segment) {
		return segment->type == HEAP;
	};
	int has_space(process_segment* segment) {
		int free_dir = find_free_dir(segment, size);
		return free_dir != -1;
	}

	process_segment* segment = malloc(sizeof(process_segment));
	void* segments = table->segments;
	int i = 0;
	while (i < table->number_of_segments) {
		memcpy(segment, segments + i * sizeof(process_segment), sizeof(process_segment));
		if (is_heap(segment) && has_space(segment)) {
			return table->segments + i * sizeof(process_segment);
		}
		i++;
	}
	return NULL;
}

process_segment* segment_by_dir(process_table* table, int dir) {
	process_segment* segment = malloc(sizeof(process_segment));
	void* segments = table->segments;
	int i = 0;
	while (i < table->number_of_segments) {
		memcpy(segment, segments + i * sizeof(process_segment), sizeof(process_segment));
		if (segment->base < dir && (segment->base + segment->size) > dir) {
			return segment;
		}
		i++;
	}
	free(segment);
	return NULL;
}

process_segment* find_extensible_heap_segment(process_table* table) {
	process_segment* segment = malloc(sizeof(process_segment));
	void* segments = table->segments;
	int offset = (table->number_of_segments - 1) * sizeof(process_segment);
	memcpy(segment, segments + offset, sizeof(process_segment));

	log_info(
		seg_logger,
		"last seg, base: %i, fin: %i, type: %s",
		segment->base,
		segment->base + segment->size,
		segment->type == HEAP ? "HEAP" : "MMAP"
	);
	if (segment->type == HEAP) {
		return segment;
	} else {
		return NULL;
	}
}

int last_position(char* process) {
	process_table* process_table = get_table_for_process(process);
	void* segments = process_table->segments;
	int i = 0;
	int last = 0;
	process_segment* segment = malloc(sizeof(process_segment));
	while (i < process_table->number_of_segments) {
		memcpy(segment, segments + i * sizeof(process_segment), sizeof(process_segment));
		last += segment->size;
		i++;
	}
	free(segment);
	return last;
}

void* find_free_dir(process_segment* segment, int size) {
	void* pages = segment->pages;
	int number_of_pages = segment->size / PAGE_SIZE;

	int free_dir = -1;
	int read_pages = 0;

	int offset = 0;
	int is_free = false;
	int data_size = 0;

	int free_space = 0;

	int first_run = 1;
	t_page* page = malloc(sizeof(t_page));
	while (read_pages < number_of_pages) {
		memcpy(page, pages + read_pages * sizeof(t_page), sizeof(t_page));
		void* frame = MEMORY[page->frame_number];

		while ((offset != 0 || first_run == 1) && (offset % PAGE_SIZE) < (PAGE_SIZE - sizeof(bool) - sizeof(uint32_t))) {
			first_run = 0;
			memcpy(&is_free, frame + offset % PAGE_SIZE, sizeof(bool));
			offset += sizeof(bool);
			memcpy(&data_size, frame + offset % PAGE_SIZE, sizeof(uint32_t));
			offset += sizeof(uint32_t);

			if (is_free) {
				if (free_dir == -1) {
					free_dir = offset - sizeof(bool) - sizeof(uint32_t);
					free_space = data_size;
				} else {
					free_space += data_size + sizeof(bool) + sizeof(uint32_t);
				}
				if (free_space >= size) {
					// ver cuanto espacio tiene
					// si el espacio alcanza esa es la dir
					return free_dir;
				} else {
					// si no alcanza hay que ver si el prox bloque de frame esta libre
					offset += data_size;
				}
			} else {
				// si no está libre no puede entrar acá...
				offset += data_size;
				data_size = 0;
				free_dir = -1;
				free_space = 0;
				log_debug(seg_logger, "offset: %i", offset);
				break;
			}
			if (offset == (read_pages + 1) * PAGE_SIZE) {
				break;
			}
		}
		read_pages++;
	}
	return -1;
}

t_page* find_page_with_space(process_segment* segment, int size) {
	void* pages = segment->pages;
	int number_of_pages = segment->size / PAGE_SIZE;
	log_debug(seg_logger, "find_page_with_space, number_of_pages: %i", number_of_pages);

	int has_free_frame(t_page* page) {
		void* frame = MEMORY[page->frame_number];
		int space = get_frame_free_size(frame);
		return space > size;
	}

	t_page* page = malloc(sizeof(t_page));
	int i = 0;
	while (i < number_of_pages) {
		log_info(seg_logger, "i: %i", i);
		memcpy(page, pages + i * sizeof(t_page), sizeof(t_page));
		if (has_free_frame(page)) {
			return page;
		}
		i++;
	}
	free(page);
	return NULL;
}

void create_process_table(char* process) {
	process_table* new_table = malloc(sizeof(process_table));
	new_table->process = process;
	new_table->segments = NULL;
	new_table->number_of_segments = 0;
	list_add(tables, new_table);
	log_info(seg_logger, "Se creo una process_table para el proceso: %s", process);
}

void add_process_segment(char* process, process_segment* segment) {
	process_table* process_table = get_table_for_process(process);
	void* segments = process_table->segments;
	int offset = process_table->number_of_segments * sizeof(process_segment);
	if (segments == NULL) {
		process_table->segments = malloc(sizeof(process_segment));
	}
	realloc(process_table->segments, segments + offset + sizeof(process_segment));
	memcpy(process_table->segments + offset, segment, sizeof(process_segment));
	process_table->number_of_segments += 1;
	log_debug(seg_logger, "Se agrego un segmento con base: %i y size: %i al process: %s", segment->base, segment->size, process);
}

process_table* get_table_for_process(char* process) {
	int is_this(process_table* segment_table) {
		return string_equals_ignore_case(segment_table->process, process);
	}
	return list_find(tables, (void*) is_this);
}

void* alloc_in_frame(int frame_number, uint32_t size) {
	void* frame = MEMORY[frame_number];
	void* pointer = NULL;
	bool x = false;
	bool y = true;

	bool is_free = false;
	uint32_t data_size = 0;
    uint32_t offset = 0;

    // Me muevo hasta encontrar un espacio libre o que se termine el frame...
	while (offset < PAGE_SIZE) {
		log_info(seg_logger, "is_free: %i, data_size: %u, offset: %i", is_free, data_size, offset);
		memcpy(&is_free, frame + offset, sizeof(bool));
		log_info(seg_logger, "is_free: %i", is_free);
		if (is_free) {
			break;
		}
		offset += sizeof(bool);
		memcpy(&data_size, frame + offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		log_info(seg_logger, "data_size: %i", data_size);
		offset += data_size;
	}

	// Si queda espacio libre entonces lo ocupo
	// y al final agrego mas metadata para lo que queda free
	if (is_free) {
		memcpy(frame + offset, &x, sizeof(bool));
		offset += sizeof(bool);
		memcpy(frame + offset, &(size), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		pointer = offset; // acá empieza la dirección de datos de lo que se pidio
		offset += size;
		memcpy(frame + offset, &y, sizeof(bool));
		offset += sizeof(bool);
		uint32_t remaining_size = PAGE_SIZE - offset - sizeof(uint32_t);
		log_info(seg_logger, "remaining_size %i", remaining_size);
		memcpy(frame + offset, &(remaining_size), sizeof(uint32_t));
	}
	return pointer;
}

int get_frame_free_size(void* frame) {
	log_debug(seg_logger, "get_frame_free_size");
	bool is_free = 0;
	uint32_t data_size = 0;
	uint32_t offset = 0;

	// Me muevo hasta encontrar un espacio libre o que se termine el frame...
	while (offset < PAGE_SIZE) {
		memcpy(&is_free, frame + offset, sizeof(bool));
		offset += sizeof(bool);
		memcpy(&data_size, frame + offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		offset += data_size;
		if (is_free) {
			log_debug(seg_logger, "get_frame_free_size, retorna %i", data_size);
			return data_size;
		}
		data_size = 0;
	}
	log_debug(seg_logger, "get_frame_free_size, retorna %i", 0);
	return 0;
}

int find_free_frame(t_bitarray* bitmap) {
	int var;
	for (var = 0; var < (MEMORY_SIZE / PAGE_SIZE); ++var) {
		bool is_used = bitarray_test_bit(bitmap, var);
		if (!is_used) {
			return var;
		}
	}
	return -1;
}

int min(int x, int y) {
	return x < y ? x : y;
}

int max(int x, int y) {
	return x > y ? x : y;
}

void* alloc_in_segment(process_segment* segment, int dir, uint32_t size) {
	void* pages = segment->pages;
	int base = dir % PAGE_SIZE;
	int dir_in_segment = dir - base;
	int allocd = 0;
	int last_allocd = 0;

	int offset = base;
	int pages_read = 0;
	t_page* page = malloc(sizeof(t_page));
	void* frame;
	while (allocd < size) {
//		log_debug(seg_logger,
//				"allocd: %i, size: %i, offset: %i, read_pages: %i, segment offset %i",
//				allocd,
//				size,
//				offset,
//				pages_read,
//				dir_in_segment + (pages_read * sizeof(t_page))
//		);
		memcpy(page, pages + dir_in_segment + pages_read * sizeof(t_page), sizeof(t_page));
		// falta chequear que la pagina este en memoria

		// get frame
		frame = MEMORY[page->frame_number];
		// falta chequear que el frame esta libre en esa base

		if (allocd == 0) {
			memcpy(frame + offset, &f, sizeof(bool));
			offset += sizeof(bool);
			last_allocd = min(size, PAGE_SIZE - base - sizeof(bool) - sizeof(uint32_t));
			memcpy(frame + offset, &(size), sizeof(uint32_t));
			offset += sizeof(uint32_t);
		} else {
			last_allocd = min(size - allocd, PAGE_SIZE);
		}
		allocd += last_allocd;
		pages_read += 1;
	}
	log_debug(seg_logger, "frame_number: %i, last_allocd: %i", page->frame_number, last_allocd);
	memcpy(frame + last_allocd, &t, sizeof(bool));
	// hay que revisar el free_space_after para seguir viendo si queda algo en las próximas páginas
	int free_space_after = PAGE_SIZE - last_allocd - sizeof(bool)- sizeof(uint32_t);
	log_debug(seg_logger, "free_space_after: %i", free_space_after);
	if (pages_read == 1) {
		// se leyó una sola, con lo cual hay que restar la base
		// al espacio libre
		free_space_after -= base;
	}
	memcpy(frame + last_allocd + sizeof(bool), &(free_space_after), sizeof(uint32_t));
	return dir_in_segment + offset;
}
