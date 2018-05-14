#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#define PNG_DEBUG 3
#include <png.h>

typedef struct queue{
	int ini, fim, tam;
	int *f;
} t_queue;

t_queue f1, f2;

/* variables used on read and write process */
int x, y;
int width, height;
png_byte color_type;
png_byte bit_depth;

png_structp png_ptr;
png_infop info_ptr;
int number_of_passes;
png_bytep *row_pointers;

void queue_init(t_queue *t, int tam);
void queue_destroy(t_queue *t);
void queue_add(t_queue *t, int v);
int queue_remove(t_queue *t);
int queue_empty(t_queue *t);
void abort_(const char *s, ...);
void read_png_file(char *file_name);
void write_png_file(char *file_name, int clear_memory);

/* Image processing */

int TAM_ARTEFATO = 35, LIMIAR = 230;
int R = 1, G = 0, B = 0;
int *v;

int ind(int y, int x) {
	return y * width + x;
}

int black(int y, int x) {
	png_byte *row = row_pointers[y];
	return row[x * 3] == 0;
}

int bfs(int ini) {
	int atual, x, y, cont = 0;
	png_byte *row;

	queue_add(&f1, ini);
	v[ini] = 1;

	while (!queue_empty(&f1))
	{
		atual = queue_remove(&f1);
		queue_add(&f2, atual);
		y = atual / width;
		x = atual % width;

		cont++;

		if (x > 0 && !v[ind(y, x - 1)] && black(y, x - 1)) {
			v[ind(y, x - 1)] = 1;
			queue_add(&f1, ind(y, x - 1));
		}
		if (x + 1 < width && !v[ind(y, x + 1)] && black(y, x + 1)) {
			v[ind(y, x + 1)] = 1;
			queue_add(&f1, ind(y, x + 1));
		}
		if (y > 0 && !v[ind(y - 1, x)] && black(y - 1, x)) {
			v[ind(y - 1, x)] = 1;
			queue_add(&f1, ind(y - 1, x));
		}
		if (y + 1 < height && !v[ind(y + 1, x)] && black(y + 1, x)) {
			v[ind(y + 1, x)] = 1;
			queue_add(&f1, ind(y + 1, x));
		}
	}

	if (cont < TAM_ARTEFATO) {
		while (!queue_empty(&f2)) {
			atual = queue_remove(&f2);
			y = atual / width;
			x = atual % width;

			row = row_pointers[y];
			row[x * 3 + 0] = 255;
			row[x * 3 + 1] = 255;
			row[x * 3 + 2] = 255;
		}

		return 0;
	}

	while (!queue_empty(&f2))
		queue_remove(&f2);

	return cont;
}

void process_file(void) {
	int v;

	for (y = 0; y < height; y++) {
		png_byte *row = row_pointers[y];
		for (x = 0; x < width; x++) {
			png_byte *ptr = &(row[x * 3]);

			if ((R * ptr[0] - G * ptr[1] - B * ptr[2]) / (R + G + B) > LIMIAR)
				v = 255;
			else
				v = 0;

			ptr[0] = ptr[1] = ptr[2] = v;
		}
	}
}

int postprocess_file() { 

	queue_init(&f1, height * width);
	queue_init(&f2, height * width);

	v = calloc(sizeof(int), height * width);
	if (!v)
		abort_("erro no calloc de v");

	int count_pixels = 0;
	int count_ovos = 0;
	int tam;
	double media;

	for (y = 0; y < height; y++) {

		png_byte *row = row_pointers[y];
		for (x = 0; x < width; x++) {
			png_byte *ptr = &(row[x * 3]);
			tam = 0;
			if (!v[ind(y, x)] && black(y, x))
				tam = bfs(ind(y, x));

			if (tam != 0)
			{
				printf("Cluster YX(%d,%d) = %d\n", y, x, tam);

				if (tam <= 61) {
					count_ovos++;
					count_pixels += tam;
				}
				else if (tam > 61 && tam <= 109) {
					count_ovos += 2;
					count_pixels += tam;
				}
				else if (tam > 109 && tam <= 217) {
					count_pixels += tam;
					count_ovos += 3;
				}
				else if (tam > 218 && tam <= 323) {
					count_pixels += tam;
					count_ovos += 3;
				}
			}
		}
	}

	printf("Pixels Counted: %d\n", count_pixels);
	printf("Counted Eggs: %d\n", count_ovos);

	free(v);
	queue_destroy(&f1);
	queue_destroy(&f2);
	return count_pixels;
}

int main(int argc, char **argv) {
	if (argc != 2)
		abort_("Usage: program_name <file_in>");

	clock_t tempos[6];
	char nome[256], nome2[256];

	tempos[0] = clock();

	strcpy(nome, argv[1]);
	int j = strlen(nome);
	nome[j - strlen(".png")] = 0;
	strcat(nome, "_proc.png");

	read_png_file(argv[1]);
	tempos[1] = clock();
	process_file();
	tempos[2] = clock();
	write_png_file(nome, 0);
	tempos[3] = clock();
	int pixel_count = postprocess_file();
	tempos[4] = clock();

	nome[j - strlen(".png")] = 0;
	sprintf(nome2, "%s-2-%d.png", nome, pixel_count);
	write_png_file(nome2, 1);

	return 0;
}

/*
 * Functions to manipulate queues
 */

void queue_init(t_queue *t, int tam) {
	t->ini = t->fim = 0;
	t->tam = tam;
	t->f = malloc(sizeof(int) * tam);

	if (!t->f)
		abort_("erro no malloc da queue");
}

void queue_destroy(t_queue *t) {
	free(t->f);
}

void queue_add(t_queue *t, int v) {
	t->f[t->fim] = v;
	t->fim = (t->fim + 1) % t->tam;
}

int queue_remove(t_queue *t) {
	int ret = t->f[t->ini];
	t->ini = (t->ini + 1) % t->tam;
	return ret;
}

int queue_empty(t_queue *t) {
	return t->ini == t->fim;
}

/* 
 *  functions to read and write png files
 */

void abort_(const char *s, ...) {
	va_list args;
	va_start(args, s);
	vfprintf(stderr, s, args);
	fprintf(stderr, "\n");
	va_end(args);
	abort();
}

void read_png_file(char *file_name) {
	char header[8]; // 8 is the maximum size that can be checked

	/* open file and test for it being a png */
	FILE *fp = fopen(file_name, "rb");
	if (!fp)
		abort_("[read_png_file] File %s could not be opened for reading", file_name);
	fread(header, 1, 8, fp);
	if (png_sig_cmp(header, 0, 8))
		abort_("[read_png_file] File %s is not recognized as a PNG file", file_name);

	/* initialize stuff */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr)
		abort_("[read_png_file] png_create_read_struct failed");

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		abort_("[read_png_file] png_create_info_struct failed");

	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[read_png_file] Error during init_io");

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	width = png_get_image_width(png_ptr, info_ptr);
	height = png_get_image_height(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);
	bit_depth = png_get_bit_depth(png_ptr, info_ptr);

	number_of_passes = png_set_interlace_handling(png_ptr);
	png_read_update_info(png_ptr, info_ptr);

	/* read file */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[read_png_file] Error during read_image");

	row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
	for (y = 0; y < height; y++)
		row_pointers[y] = (png_byte *)malloc(png_get_rowbytes(png_ptr, info_ptr));

	png_read_image(png_ptr, row_pointers);

	fclose(fp);
}

void write_png_file(char *file_name, int clear_memory) {
	/* create file */
	FILE *fp = fopen(file_name, "wb");
	if (!fp)
		abort_("[write_png_file] File %s could not be opened for writing", file_name);

	/* initialize stuff */
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr)
		abort_("[write_png_file] png_create_write_struct failed");

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		abort_("[write_png_file] png_create_info_struct failed");

	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[write_png_file] Error during init_io");

	png_init_io(png_ptr, fp);

	/* write header */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[write_png_file] Error during writing header");

	png_set_IHDR(png_ptr, info_ptr, width, height,
				 bit_depth, color_type, PNG_INTERLACE_NONE,
				 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);

	/* write bytes */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[write_png_file] Error during writing bytes");

	png_write_image(png_ptr, row_pointers);

	/* end write */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[write_png_file] Error during end of write");

	png_write_end(png_ptr, NULL);

	if (clear_memory) {
		/* cleanup heap allocation */
		for (y = 0; y < height; y++)
			free(row_pointers[y]);
		free(row_pointers);
	}

	fclose(fp);
}
