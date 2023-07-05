#include "../../src/filesystem.h"
#include <commons/collections/list.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cspecs/cspec.h>
#include <pthread.h>
#include<sys/stat.h>


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
			//free(superbloque);
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
			}end

			it("debe obtener el primero que encuentra libre"){
				should_int(obtener_primer_bloque_libre()) be equal to(0);
			}end
			it("si esta ocupado busca el siguiente bloque libre"){

				bitarray_set_bit(bitarray_bloques_libres, 0);
				bitarray_set_bit(bitarray_bloques_libres, 2);

				should_int(obtener_primer_bloque_libre()) be equal to(1);
			}end

		}end

		describe("void marcar_bloques_libres_directo(uint32_t numero_de_bloque_directo)"){
			int tamanio_bitmap;
			before{
				tamanio_bitmap = 255;
				char* bits = malloc(tamanio_bitmap);
				bitarray_bloques_libres = bitarray_create_with_mode(bits, tamanio_bitmap, MSB_FIRST);
				bitmap = fopen("fs/bitmap.dat", "wb");
			}end
			after{
				fclose(bitmap);
				remove("fs/bitmap.dat");
				bitarray_destroy(bitarray_bloques_libres);
			}end

			it("debe marcar con 0 al bloque ocupado y actualizar el archivo del bitmap correctamente"){
				bitarray_set_bit(bitarray_bloques_libres, 12);
				should_bool(bitarray_test_bit(bitarray_bloques_libres, 12)) be equal to(true);

				marcar_bloques_libres_directo(12);

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
			}end
			after{
				fclose(bloques);
				fclose(bitmap);
				bitarray_destroy(bitarray_bloques_libres);
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

				marcar_bloques_libres_indirecto(12, superbloque, punteros_x_bloque);

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
			}end
			after{
				fclose(bloques);
				fclose(bitmap);
				remove("fs/bitmap.dat");
				remove("fs/bloques.dat");
				bitarray_destroy(bitarray_bloques_libres);
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

				marcar_bloques_libres_indirecto_hasta(12, 1, superbloque, punteros_x_bloque);

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
			}end

			after{
				fclose(bitmap);
				remove("fs/bitmap.dat");
				bitarray_destroy(bitarray_bloques_libres);
			}end
			it("debe ocupar un bloque directo, actualizandolo en el fcb y actualizar el bit array como bloque ocupado correctamente"){
				t_fcb* fcb = malloc(sizeof(t_fcb));

				bitarray_set_bit(bitarray_bloques_libres, 0);
				bitarray_clean_bit(bitarray_bloques_libres, 0);

				should_bool(bitarray_test_bit(bitarray_bloques_libres,0)) be equal to(false);

				ocupar_bloque_libre_directo(fcb);

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
				superbloque = malloc(sizeof(t_superbloque));
				superbloque->block_size=8;
				superbloque->block_count=tamanio_bitmap;
			}end

			after{
				fclose(bitmap);
				remove("fs/bitmap.dat");
				bitarray_destroy(bitarray_bloques_libres);
			}end
//			it("debe ocupar un bloque indirecto, actualizandolo en el fcb correctamente"){
//				t_fcb* fcb = malloc(sizeof(t_fcb));
//
//				int punteros_x_bloque = superbloque->block_size/4;
//
//				ocupar_bloque_libre_indirecto(fcb, 1, punteros_x_bloque, superbloque);
//
//				should_int(fcb->puntero_indirecto) be equal to(2);
//
//				free(fcb);
//			}end
//			it("debe tambien actualizar el bit array como bloque ocupado"){
//				t_fcb* fcb = malloc(sizeof(t_fcb));
//
//				int punteros_x_bloque = superbloque->block_size/4;
//
//				ocupar_bloque_libre_indirecto(fcb, 1, punteros_x_bloque, superbloque);
//
//				should_int(fcb->puntero_indirecto) be equal to(1);
//				should_bool(bitarray_test_bit(bitarray_bloques_libres,1)) be equal to(true);
//
//				bitmap = freopen("fs/bitmap.dat", "rb", bitmap);
//				fseek(bitmap, 0, SEEK_SET);
//				char* buffer = malloc(bitarray_bloques_libres->size);
//				fread(buffer, bitarray_bloques_libres->size, 1,bitmap);
//
//				t_bitarray* bitarray_actualizado= bitarray_create_with_mode(buffer, tamanio_bitmap, MSB_FIRST);
//
//				should_bool(bitarray_test_bit(bitarray_actualizado, 0)) be equal to(bitarray_test_bit(bitarray_bloques_libres, 0));
//				should_bool(bitarray_test_bit(bitarray_actualizado, 1)) be equal to(bitarray_test_bit(bitarray_bloques_libres, 1));
//
//				free(buffer);
//				bitarray_destroy(bitarray_actualizado);
//				free(fcb);
//			}end
		}end

		describe("void ocupar_bloque_libre_indirecto_fatlantes(t_fcb* fcb, int bloques_a_agregar, t_superbloque* superbloque)"){

		}end

		describe("char* leer_en_bloque(uint32_t bloque_a_leer, t_superbloque* superbloque)"){

		}end

		describe("void guardar_en_bloque(int numero_de_bloque, char* contenido_a_guardar, t_superbloque* superbloque)"){
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

			}end

			after{
				fclose(bitmap);
				fclose(bloques);
				remove("fs/bloques.dat");
				remove("fs/bitmap.dat");
				bitarray_destroy(bitarray_bloques_libres);
			}end

			it("debe poder guardar un contenido en el numero de bloque correspondiente, en el archivo de bloques de forma exitosa"){

				guardar_en_bloque(0, "hola si", superbloque);
				char* buffer = malloc(superbloque->block_size +1);
				fseek(bloques, (superbloque->block_size)*0, SEEK_SET);
				int leido = fread(buffer,superbloque->block_size, 1, bloques);

				should_int(leido) be equal to(1);
				should_string(buffer) be equal to("hola si");

			}end
		}end
}
