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
			before{
				int tamanio_bitmap = 255;
				char* bits = malloc(tamanio_bitmap);
				bitarray_bloques_libres = bitarray_create_with_mode(bits, tamanio_bitmap, MSB_FIRST);
			}end


		}end
		describe("void marcar_bloques_libres_indirecto(uint32_t puntero_indirecto, t_superbloque* superbloque, int punteros_x_bloque)"){

		}end
		describe("void marcar_bloques_libres_indirecto_hasta(uint32_t puntero_indirecto, int numeros_de_bloques_a_sacar, t_superbloque* superbloque, int punteros_x_bloque)"){

		}end
		describe("void ocupar_bloque_libre_directo(t_fcb* fcb)"){
			before{
				int tamanio_bitmap = 255;
				char* bits = malloc(tamanio_bitmap);
				bitarray_bloques_libres = bitarray_create_with_mode(bits, tamanio_bitmap, MSB_FIRST);
			}end
			it("debe ocupar un bloque directo, actualizandolo en el fcb correctamente"){
				t_fcb* fcb = malloc(sizeof(t_fcb));

				ocupar_bloque_libre_directo(fcb);

				should_int(fcb->puntero_directo) be equal to(0);
			}end
			it("debe tambien actualizar el bit array como bloque ocupado"){
				t_fcb* fcb = malloc(sizeof(t_fcb));

				ocupar_bloque_libre_directo(fcb);

				should_int(fcb->puntero_directo) be equal to(0);
				should_bool(bitarray_test_bit(bitarray_bloques_libres,0)) be equal to(true);

			}end
		}end
		describe("void ocupar_bloque_libre_indirecto(t_fcb* fcb, int bloques_a_agregar, int punteros_x_bloque, t_superbloque* superbloque)"){

		}end

		describe("void ocupar_bloque_libre_indirecto_fatlantes(t_fcb* fcb, int bloques_a_agregar, t_superbloque* superbloque)"){

		}end

		describe("char* leer_en_bloque(uint32_t bloque_a_leer, t_superbloque* superbloque)"){

		}end

		describe("void guardar_en_bloque(int numero_de_bloque, char* contenido_a_guardar, t_superbloque* superbloque)"){

		}end
}
