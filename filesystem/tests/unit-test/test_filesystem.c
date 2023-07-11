#include "../../src/filesystem.h"
#include <commons/collections/list.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cspecs/cspec.h>
#include <pthread.h>
#include<sys/stat.h>
#include <semaphore.h>

sem_t semaforo;

context(test_filesystem){
	describe("t_superbloque* iniciar_superbloque(char* path_superbloque)"){

		it("si existe el archivo lee su contenido y setea los valores el superbloque"){
			char* path_superbloque = malloc(sizeof(char)*29);
			strcpy(path_superbloque, "fs/superbloque.dat");
			t_superbloque* superbloque;

			superbloque = iniciar_superbloque(path_superbloque);

			should_ptr(superbloque) not be null;

			should_int(superbloque->block_size) be equal to(64);
			should_int(superbloque->block_count) be equal to(65536);


			free(path_superbloque);
			free(superbloque);

		}end
		it("si no existe el archivo devuelve un NULL y no hace nada"){
			char* path_superbloque = malloc(sizeof(char)*30);
			strcpy(path_superbloque, "fs/superbloque.noexiste");
			t_superbloque* superbloque;

			superbloque = iniciar_superbloque(path_superbloque);

			should_ptr(superbloque) be null;

			free(path_superbloque);
			free(superbloque);
		}end

	}end

	describe("t_fcb* iniciar_fcb(t_config* config)"){

			it("si existe el archivo lee su contenido y setea los valores del fcb"){
				char* path_fcb = malloc(sizeof(char)*29);
				strcpy(path_fcb, "fs/fcb");
				t_fcb* fcb;

				t_config* config_FCB = config_create(path_fcb);

				fcb = iniciar_fcb(config_FCB);

				should_ptr(fcb) not be null;

				should_string(fcb->nombre_archivo) be equal to("Notas1erParcialK9999");
				should_int(fcb->tamanio_archivo) be equal to(256);
				should_int(fcb->puntero_directo) be equal to(12);
				should_int(fcb->puntero_indirecto) be equal to(45);

				free(path_fcb);
				free(fcb->nombre_archivo);
				free(fcb);

			}end
			it("si no existe el archivo devuelve un NULL y no hace nada"){
				char* path_fcb = malloc(sizeof(char)*29);
				strcpy(path_fcb, "fs/noexistente");
				t_fcb* fcb;

				t_config* config_FCB = config_create(path_fcb);

				fcb = iniciar_fcb(config_FCB);

				should_ptr(fcb) be null;

				free(path_fcb);
			}end

		}end


		describe("int calcular_cantidad_de_bloques(int tamanio_en_bytes ,t_superbloque* superbloque)"){
			t_superbloque* superbloque ;
			before{
				superbloque = malloc(sizeof(t_superbloque));
				superbloque->block_size = 4;
				superbloque->block_count = 255;
			}end

			it("debe poder calcular la cantidad de bloques correspondientes a ciertos bytes pares correctamente"){
				int bytes = 16;

				should_int(calcular_cantidad_de_bloques(bytes, superbloque)) be equal to(4);

			}end
			it("debe poder calcular la cantidad de bloques correspondientes a ciertos bytes inpares correctamente"){
				int bytes = 17;

				should_int(calcular_cantidad_de_bloques(bytes, superbloque)) be equal to(5);

			}end


			it("si son 0 bytes, devuelve 0 bloques"){
				should_int(calcular_cantidad_de_bloques(0, superbloque)) be equal to(0);
			}end
		}end

		describe("int obtener_primer_bloque_libre()"){
			before{
				int tamanio_bitmap = 255;
				char* bits = malloc(tamanio_bitmap);
				bitarray_bloques_libres = bitarray_create_with_mode(bits, tamanio_bitmap, MSB_FIRST);

				bitmap = fopen("fs/bitmap.dat", "wb");

				sem_init(&semaforo, 0, 1);
			}end

			after{
//				fclose(bitmap);
				remove("fs/bitmap.dat");
//				bitarray_destroy(bitarray_bloques_libres);
			}end

			it("debe obtener el primero que encuentra libre"){
				sem_wait(&semaforo);
				should_int(obtener_primer_bloque_libre()) be equal to(0);
				sem_post(&semaforo);
			}end
			it("si esta ocupado busca el siguiente bloque libre"){

				bitarray_set_bit(bitarray_bloques_libres, 0);
				bitarray_set_bit(bitarray_bloques_libres, 2);

				sem_wait(&semaforo);
				should_int(obtener_primer_bloque_libre()) be equal to(1);
				sem_post(&semaforo);
			}end

		}end

		describe("void marcar_bloques_libres_directo(uint32_t numero_de_bloque_directo)"){
			int tamanio_bitmap;

			before{
				tamanio_bitmap = 255;
				char* bits = malloc(tamanio_bitmap);
				bitarray_bloques_libres = bitarray_create_with_mode(bits, tamanio_bitmap, MSB_FIRST);

				bitmap = fopen("fs/bitmap.dat", "wb");
				sem_init(&semaforo, 0, 1);
			}end
			after{
//				fclose(bitmap);
				remove("fs/bitmap.dat");
//				bitarray_destroy(bitarray_bloques_libres);
			}end

			it("debe marcar con 0 al bloque ocupado y actualizar el archivo del bitmap correctamente"){
				bitarray_set_bit(bitarray_bloques_libres, 12);
				should_bool(bitarray_test_bit(bitarray_bloques_libres, 12)) be equal to(true);

				sem_wait(&semaforo);
				marcar_bloques_libres_directo(12);
				sem_post(&semaforo);

				should_bool(bitarray_test_bit(bitarray_bloques_libres, 12)) be equal to(false);


				bitmap = freopen("fs/bitmap.dat", "rb", bitmap);

				fseek(bitmap, 0, SEEK_SET);
				char* buffer = malloc(bitarray_bloques_libres->size);
				int leido = fread(buffer,bitarray_bloques_libres->size, 1,bitmap);

				should_int(leido) be equal to(1);
				t_bitarray* bitarray_actualizado= bitarray_create_with_mode(buffer, tamanio_bitmap, MSB_FIRST);

				should_bool(bitarray_test_bit(bitarray_actualizado, 12)) be equal to(bitarray_test_bit(bitarray_bloques_libres, 12));

				free(buffer);
				bitarray_destroy(bitarray_actualizado);
			}end

		}end
		describe("void marcar_bloques_libres_indirecto(uint32_t puntero_indirecto, t_superbloque* superbloque, int punteros_x_bloque)"){
			int tamanio_bitmap;
			t_superbloque* superbloque;
			before{
				tamanio_bitmap = 255;
				char* bits = malloc(tamanio_bitmap);
				bitarray_bloques_libres = bitarray_create_with_mode(bits, tamanio_bitmap, MSB_FIRST);

				bitmap= fopen("fs/bitmap.dat", "wb");
				bloques = fopen("fs/bloques.dat","w+");

				superbloque = malloc(sizeof(t_superbloque));
				superbloque->block_size=4;
				superbloque->block_count=tamanio_bitmap;

				sem_init(&semaforo, 0, 1);
			}end
			after{
//				fclose(bloques);
//				fclose(bitmap);
//				bitarray_destroy(bitarray_bloques_libres);
				remove("fs/bitmap.dat");
				remove("fs/bloques.dat");
			}end

			it("debe marcar con 0 a todos los bloques que esten en el bloque de indices apuntado y actualizar el archivo del bitmap correctamente"){
				bitarray_set_bit(bitarray_bloques_libres, 12);
				bitarray_set_bit(bitarray_bloques_libres, 1);

				int punteros_x_bloque = superbloque->block_size/4;

				fseek(bloques, 0, SEEK_SET);
				fwrite("0001",strlen("0001"),1,bloques);

				should_bool(bitarray_test_bit(bitarray_bloques_libres, 1)) be equal to(true);
				should_bool(bitarray_test_bit(bitarray_bloques_libres, 12)) be equal to(true);

				sem_wait(&semaforo);
				marcar_bloques_libres_indirecto(12, superbloque, punteros_x_bloque);
				sem_post(&semaforo);

				should_bool(bitarray_test_bit(bitarray_bloques_libres, 1)) be equal to(false);
				should_bool(bitarray_test_bit(bitarray_bloques_libres, 12)) be equal to(false);

				struct stat stat_file;
				stat("fs/bitmap.dat", &stat_file);


				bitmap = freopen("fs/bitmap.dat", "rb", bitmap);
				fseek(bitmap, 0, SEEK_SET);
				char* buffer = malloc(stat_file.st_size);
				fread(buffer, stat_file.st_size, 1,bitmap);

				t_bitarray* bitarray_actualizado= bitarray_create_with_mode(buffer, tamanio_bitmap, MSB_FIRST);

				should_bool(bitarray_test_bit(bitarray_actualizado, 1)) be equal to(bitarray_test_bit(bitarray_bloques_libres, 1));
				should_bool(bitarray_test_bit(bitarray_actualizado, 12)) be equal to(bitarray_test_bit(bitarray_bloques_libres, 12));


				free(buffer);
				bitarray_destroy(bitarray_actualizado);
			}end

		}end
		describe("void marcar_bloques_libres_indirecto_hasta(uint32_t puntero_indirecto, int numeros_de_bloques_a_sacar, t_superbloque* superbloque, int punteros_x_bloque)"){
			int tamanio_bitmap;
			t_superbloque* superbloque;
			before{
				tamanio_bitmap = 255;
				char* bits = malloc(tamanio_bitmap);
				bitarray_bloques_libres = bitarray_create_with_mode(bits, tamanio_bitmap, MSB_FIRST);

				bitmap= fopen("fs/bitmap.dat", "wb");
				bloques = fopen("fs/bloques.dat","w+");

				superbloque = malloc(sizeof(t_superbloque));
				superbloque->block_size=8;
				superbloque->block_count=tamanio_bitmap;

				sem_init(&semaforo, 0, 1);
			}end
			after{
//				fclose(bloques);
//				fclose(bitmap);
				remove("fs/bitmap.dat");
				remove("fs/bloques.dat");
//				bitarray_destroy(bitarray_bloques_libres);
			}end

			it("debe marcar con 0 a todos los bloques que esten en el bloque de indices apuntado y actualizar el archivo del bitmap correctamente"){
				bitarray_set_bit(bitarray_bloques_libres, 12);
				bitarray_set_bit(bitarray_bloques_libres, 1);
				bitarray_set_bit(bitarray_bloques_libres, 2);


				int punteros_x_bloque = superbloque->block_size/4;

				should_int(punteros_x_bloque) be equal to(2);

				char* punteros_directos_guardados = string_new();
				string_append(&punteros_directos_guardados, "0001");
				string_append(&punteros_directos_guardados, "0002");

				fseek(bloques, (superbloque->block_size) *12, SEEK_SET);
				fwrite(punteros_directos_guardados,superbloque->block_size,1,bloques);

				free(punteros_directos_guardados);

				should_bool(bitarray_test_bit(bitarray_bloques_libres, 2)) be equal to(true);
				should_bool(bitarray_test_bit(bitarray_bloques_libres, 1)) be equal to(true);
				should_bool(bitarray_test_bit(bitarray_bloques_libres, 12)) be equal to(true);

				sem_wait(&semaforo);
				marcar_bloques_libres_indirecto_hasta(12, 1, superbloque, punteros_x_bloque);
				sem_post(&semaforo);

				should_bool(bitarray_test_bit(bitarray_bloques_libres, 1)) be equal to(true);
				should_bool(bitarray_test_bit(bitarray_bloques_libres, 2)) be equal to(false);
				should_bool(bitarray_test_bit(bitarray_bloques_libres, 12)) be equal to(false);


				bitmap = freopen("fs/bitmap.dat", "rb", bitmap);
				fseek(bitmap, 0, SEEK_SET);
				char* buffer = malloc(bitarray_bloques_libres->size);
				fread(buffer, bitarray_bloques_libres->size, 1,bitmap);

				t_bitarray* bitarray_actualizado= bitarray_create_with_mode(buffer, tamanio_bitmap, MSB_FIRST);

				should_bool(bitarray_test_bit(bitarray_actualizado, 2)) be equal to(bitarray_test_bit(bitarray_bloques_libres, 2));
				should_bool(bitarray_test_bit(bitarray_actualizado, 1)) be equal to(bitarray_test_bit(bitarray_bloques_libres, 1));
				should_bool(bitarray_test_bit(bitarray_actualizado, 12)) be equal to(bitarray_test_bit(bitarray_bloques_libres, 12));

				free(buffer);
				bitarray_destroy(bitarray_actualizado);
			}end
		}end
		describe("void ocupar_bloque_libre_directo(t_fcb* fcb)"){
			int tamanio_bitmap;
			before{
				tamanio_bitmap = 255;
				char* bits = malloc(tamanio_bitmap);
				bitarray_bloques_libres = bitarray_create_with_mode(bits, tamanio_bitmap, MSB_FIRST);
				bitmap= fopen("fs/bitmap.dat", "wb");

				sem_init(&semaforo, 0, 1);
			}end

			after{
//				fclose(bitmap);
				remove("fs/bitmap.dat");
//				bitarray_destroy(bitarray_bloques_libres);
			}end
			it("debe ocupar un bloque directo, actualizandolo en el fcb y actualizar el bit array como bloque ocupado correctamente"){
				t_fcb* fcb = malloc(sizeof(t_fcb));

				bitarray_set_bit(bitarray_bloques_libres, 0);
				bitarray_clean_bit(bitarray_bloques_libres, 0);

				should_bool(bitarray_test_bit(bitarray_bloques_libres,0)) be equal to(false);

				sem_wait(&semaforo);
				ocupar_bloque_libre_directo(fcb);
				sem_post(&semaforo);

				should_int(fcb->puntero_directo) be equal to(0);
				should_bool(bitarray_test_bit(bitarray_bloques_libres,0)) be equal to(true);

				bitmap = freopen("fs/bitmap.dat", "rb", bitmap);
				fseek(bitmap, 0, SEEK_SET);
				char* buffer = malloc(bitarray_bloques_libres->size +1);
				fread(buffer, bitarray_bloques_libres->size, 1,bitmap);

				t_bitarray* bitarray_actualizado= bitarray_create_with_mode(buffer, tamanio_bitmap, MSB_FIRST);

				should_bool(bitarray_test_bit(bitarray_actualizado, 0)) be equal to(bitarray_test_bit(bitarray_bloques_libres, 0));

				free(buffer);
				bitarray_destroy(bitarray_actualizado);
				free(fcb);
			}end

		}end

		describe("void ocupar_bloque_libre_indirecto(t_fcb* fcb, int bloques_a_agregar, int punteros_x_bloque, t_superbloque* superbloque)"){
			int tamanio_bitmap;
			t_superbloque* superbloque;

			before{
				tamanio_bitmap = 255;
				char* bits = malloc(tamanio_bitmap);
				bitarray_bloques_libres = bitarray_create_with_mode(bits, tamanio_bitmap, MSB_FIRST);

				bitmap= fopen("fs/bitmap.dat", "wb");
				bloques = fopen("fs/bloques.dat","w+");

				superbloque = malloc(sizeof(t_superbloque));
				superbloque->block_size=8;
				superbloque->block_count=tamanio_bitmap;

				sem_init(&semaforo, 0, 1);
			}end

			after{
				//bitarray_destroy(bitarray_bloques_libres);
//				fclose(bloques);
				remove("fs/bloques.dat");

//				fclose(bitmap);
				remove("fs/bitmap.dat");

				free(superbloque);
			}end
			it("debe ocupar un bloque indirecto, actualizandolo en el fcb correctamente"){
				t_fcb* fcb = malloc(sizeof(t_fcb));
				fcb->puntero_indirecto = -1;

				int punteros_x_bloque = (superbloque->block_size)/4;

				bitarray_destroy(bitarray_bloques_libres);
				char* bits = malloc(tamanio_bitmap);
				bitarray_bloques_libres = bitarray_create_with_mode(bits, tamanio_bitmap, MSB_FIRST);

				sem_wait(&semaforo);
				ocupar_bloque_libre_indirecto(fcb, 1, punteros_x_bloque, superbloque);
				sem_post(&semaforo);

				should_int(fcb->puntero_indirecto) be equal to(0);

				free(fcb);
			}end
			it("debe tambien actualizar el bit array y el archivo bitmap como bloque ocupado"){
				t_fcb* fcb = malloc(sizeof(t_fcb));
				fcb->puntero_indirecto = -1;

				int punteros_x_bloque = superbloque->block_size/4;

				sem_wait(&semaforo);
				ocupar_bloque_libre_indirecto(fcb, 1, punteros_x_bloque, superbloque);
				sem_post(&semaforo);

				should_int(fcb->puntero_indirecto) be equal to(0);
				should_bool(bitarray_test_bit(bitarray_bloques_libres,0)) be equal to(true);

				bitmap = freopen("fs/bitmap.dat", "rb", bitmap);
				fseek(bitmap, 0, SEEK_SET);
				char* buffer = malloc(bitarray_bloques_libres->size);
				int datos_leidos = fread(buffer, bitarray_bloques_libres->size, 1,bitmap);

				should_int(datos_leidos) be equal to(1);

				t_bitarray* bitarray_actualizado= bitarray_create_with_mode(buffer, tamanio_bitmap, MSB_FIRST);

				should_bool(bitarray_test_bit(bitarray_actualizado, 0)) be equal to(bitarray_test_bit(bitarray_bloques_libres, 0));
				should_bool(bitarray_test_bit(bitarray_actualizado, 1)) be equal to(bitarray_test_bit(bitarray_bloques_libres, 1));

				free(buffer);
				bitarray_destroy(bitarray_actualizado);
				free(fcb);
			}end
		}end

		describe("void ocupar_bloque_libre_indirecto_fatlantes(t_fcb* fcb, int bloques_a_agregar, t_superbloque* superbloque)"){
			int tamanio_bitmap;
			t_superbloque* superbloque;

			before{
				tamanio_bitmap = 255;
				char* bits = malloc(tamanio_bitmap);
				bitarray_bloques_libres = bitarray_create_with_mode(bits, tamanio_bitmap, MSB_FIRST);

				bitmap= fopen("fs/bitmap.dat", "wb");
				bloques = fopen("fs/bloques.dat","w+");

				superbloque = malloc(sizeof(t_superbloque));
				superbloque->block_size=12;// caben 3 punteros en total
				superbloque->block_count=tamanio_bitmap;

				sem_init(&semaforo, 0, 1);
			}end

			after{
//				fclose(bloques);
				remove("fs/bloques.dat");

//				fclose(bitmap);
				remove("fs/bitmap.dat");

//				bitarray_destroy(bitarray_bloques_libres);
			}end

			it("se debe poder ocupar bloques necesarios de forma existosa en el puntero indirecto existente"){
				t_fcb* fcb = malloc(sizeof(t_fcb));

				bitarray_destroy(bitarray_bloques_libres);
				char* bits = malloc(tamanio_bitmap);
				bitarray_bloques_libres = bitarray_create_with_mode(bits, tamanio_bitmap, MSB_FIRST);

				sem_wait(&semaforo);
				fcb->puntero_indirecto = obtener_primer_bloque_libre();
				sem_post(&semaforo);

				sem_wait(&semaforo);
				int puntero_directo_1 = obtener_primer_bloque_libre();
				sem_post(&semaforo);

				char* puntero_directo_1_inverso = string_itoa(puntero_directo_1);
				string_append(&puntero_directo_1_inverso, "000");
				char* puntero_directo_completo = string_reverse(puntero_directo_1_inverso);

				should_string(puntero_directo_completo) be equal to("0001");

				sem_wait(&semaforo);
				guardar_en_bloque(fcb->puntero_indirecto ,puntero_directo_completo, superbloque);

				ocupar_bloque_libre_indirecto_fatlantes(fcb,2, superbloque);
				sem_post(&semaforo);

				fseek(bloques, (superbloque->block_size)*(fcb->puntero_indirecto), SEEK_SET);
				char* contenido_leido = malloc(superbloque->block_size);
				int datos_leidos = fread(contenido_leido, superbloque->block_size, 1,bloques);

				should_int(datos_leidos) be equal to(1);

				should_string(contenido_leido) be equal to("000100020003");
				should_bool(string_contains(contenido_leido, "0001")) be equal to(true);
				should_bool(string_contains(contenido_leido, "0002")) be equal to(true);
				should_bool(string_contains(contenido_leido, "0003")) be equal to(true);

				free(fcb);
			}end

			it("no debe crear punteros si se pasa de la cantidad maxima de punteros por bloque, en el bloque indirecto"){
				t_fcb* fcb = malloc(sizeof(t_fcb));

				fcb->puntero_indirecto = obtener_primer_bloque_libre();

				int puntero_directo_1 = obtener_primer_bloque_libre();


				char* puntero_directo_1_inverso = string_itoa(puntero_directo_1);
				string_append(&puntero_directo_1_inverso, "000");
				char* puntero_directo_completo = string_reverse(puntero_directo_1_inverso);

				should_string(puntero_directo_completo) be equal to("0001");

				sem_wait(&semaforo);
				guardar_en_bloque(fcb->puntero_indirecto ,puntero_directo_completo, superbloque);

				ocupar_bloque_libre_indirecto_fatlantes(fcb,3, superbloque);
				sem_post(&semaforo);

				fseek(bloques, (superbloque->block_size)*(fcb->puntero_indirecto), SEEK_SET);
				char* contenido_leido = malloc(superbloque->block_size);
				int datos_leidos = fread(contenido_leido, superbloque->block_size, 1,bloques);

				should_int(datos_leidos) be equal to(1);

				should_string(contenido_leido) not be equal to("000100020003");
				should_bool(string_contains(contenido_leido, "0001")) be equal to(true);
				should_bool(string_contains(contenido_leido, "0002")) be equal to(false);
				should_bool(string_contains(contenido_leido, "0003")) be equal to(false);
				should_bool(string_contains(contenido_leido, "0004")) be equal to(false);

				free(fcb);
			}end

			it("no debe crear punteros si no tiene un puntero indirecto"){
				t_fcb* fcb = malloc(sizeof(t_fcb));
				fcb->puntero_indirecto = -1;

				sem_wait(&semaforo);
				ocupar_bloque_libre_indirecto_fatlantes(fcb,3, superbloque);
				sem_post(&semaforo);

				fseek(bloques, (superbloque->block_size)*(fcb->puntero_indirecto), SEEK_SET);
				char* contenido_leido = malloc(superbloque->block_size);
				int datos_leidos = fread(contenido_leido, superbloque->block_size, 1,bloques);

				should_int(datos_leidos) be equal to(0);
				should_string(contenido_leido) not be equal to("000100020003");

				free(fcb);
			}end
		}end

		describe("char* leer_en_bloque(uint32_t bloque_a_leer, t_superbloque* superbloque)"){
			t_superbloque* superbloque;
			before{
				superbloque = malloc(sizeof(t_superbloque));
				superbloque->block_size=4;
				superbloque->block_count=255;

				bloques = fopen("fs/bloques.dat","w+");

				sem_init(&semaforo, 0, 1);
			}end

			after{
				//free(superbloque);

//				fclose(bloques);
				remove("fs/bloques.dat");
			}end

			it("debe devolver el string que encontro en el bloque indicado por parametros"){
				char* palabra_secreta = string_new();
				string_append(&palabra_secreta, "hola");

				int numero_de_bloque = 1;

				sem_wait(&semaforo);
				guardar_en_bloque(numero_de_bloque, palabra_secreta, superbloque);
				sem_post(&semaforo);

				char* contenido = leer_en_bloque(numero_de_bloque, superbloque);

				should_string(contenido) be equal to(palabra_secreta);
				free(palabra_secreta);
			}end

			it("si en el bloque no hay nada, entonces devuelve un string vacio"){
				char* palabra_secreta = string_new();
				string_append(&palabra_secreta, "hola");

				int numero_de_bloque = 1;

				sem_wait(&semaforo);
				guardar_en_bloque(numero_de_bloque, palabra_secreta, superbloque);
				sem_post(&semaforo);

				// leo en otro bloque
				char* contenido = leer_en_bloque(2, superbloque);

				should_string(contenido) not be equal to(palabra_secreta);
				should_string(contenido) be equal to("");

			}end
		}end

		describe("void guardar_en_bloque(int numero_de_bloque, char* contenido_a_guardar, t_superbloque* superbloque)"){
			t_superbloque* superbloque;

			before{
				bloques = fopen("fs/bloques.dat","w+");

				superbloque = malloc(sizeof(t_superbloque));
				superbloque->block_size=8;
				superbloque->block_count=255;

				sem_init(&semaforo, 0, 1);

			}end

			after{
//				fclose(bloques);
				remove("fs/bloques.dat");
			}end

			it("debe poder guardar un contenido en el numero de bloque correspondiente, en el archivo de bloques de forma exitosa"){

				int numero_de_bloque = 10;
				char* contenido_a_guardar_en_bloque = string_new();
				string_append(&contenido_a_guardar_en_bloque, "hola si");

				sem_wait(&semaforo);
				guardar_en_bloque(numero_de_bloque, contenido_a_guardar_en_bloque, superbloque);
				sem_post(&semaforo);

				char* buffer = malloc(superbloque->block_size +1);
				fseek(bloques, (superbloque->block_size)*numero_de_bloque , SEEK_SET);
				int leido = fread(buffer,superbloque->block_size, 1, bloques);


				should_int(leido) be equal to(1);
				should_string(buffer) be equal to(contenido_a_guardar_en_bloque);

			}end

			it("no se debe guardar en un bloque inexistente"){

				int numero_de_bloque = -100;
				char* contenido_a_guardar_en_bloque = string_new();
				string_append(&contenido_a_guardar_en_bloque, "hola si");

				sem_wait(&semaforo);
				guardar_en_bloque(numero_de_bloque, contenido_a_guardar_en_bloque, superbloque);
				sem_post(&semaforo);

				char* buffer = malloc(superbloque->block_size +1);
				fseek(bloques, (superbloque->block_size)*numero_de_bloque , SEEK_SET);
				int leido = fread(buffer,superbloque->block_size, 1, bloques);


				should_int(leido) be equal to(0);

			}end

			it("no se debe guardar si el contenido se pasa del block size"){

				int numero_de_bloque = 0;
				char* contenido_a_guardar_en_bloque = string_new();
				string_append(&contenido_a_guardar_en_bloque, "hola si ahrre");

				sem_wait(&semaforo);
				guardar_en_bloque(numero_de_bloque, contenido_a_guardar_en_bloque, superbloque);
				sem_post(&semaforo);

				char* buffer = malloc(superbloque->block_size +1);
				fseek(bloques, (superbloque->block_size)*numero_de_bloque , SEEK_SET);
				int leido = fread(buffer,superbloque->block_size, 1, bloques);


				should_int(leido) be equal to(0);

			}end
			it("debe poder sobreescribir sobre el contenido viejo de un bloque"){

				int numero_de_bloque = 0;
				char* contenido_a_guardar_en_bloque = string_new();
				string_append(&contenido_a_guardar_en_bloque, "hola si");

				sem_wait(&semaforo);
				guardar_en_bloque(numero_de_bloque, contenido_a_guardar_en_bloque, superbloque);
				sem_post(&semaforo);

				char* buffer = malloc(superbloque->block_size +1);
				fseek(bloques, (superbloque->block_size)*numero_de_bloque , SEEK_SET);
				int datos_leidos = fread(buffer,superbloque->block_size, 1, bloques);


				should_int(datos_leidos) be equal to(1);
				should_string(buffer) be equal to(contenido_a_guardar_en_bloque);

				char* contenido_a_sobreescribir = string_new();
				string_append(&contenido_a_sobreescribir, "hola no");

				sem_wait(&semaforo);
				guardar_en_bloque(numero_de_bloque, contenido_a_sobreescribir, superbloque);
				sem_post(&semaforo);

				char* contenido_leido = malloc(superbloque->block_size +1);
				fseek(bloques, (superbloque->block_size)*numero_de_bloque , SEEK_SET);
				int datos_nuevos_leidos = fread(contenido_leido,superbloque->block_size, 1, bloques);


				should_int(datos_nuevos_leidos) be equal to(1);
				should_string(contenido_leido) not be equal to(contenido_a_guardar_en_bloque);
				should_string(contenido_leido) be equal to(contenido_a_sobreescribir);

			}end

		}end

}
