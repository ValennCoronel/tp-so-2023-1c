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

	describe("t_superbloque* iniciar_fcb(char* path_fcb)"){

			it("si existe el archivo lee su contenido y setea los valores del fcb"){
				char* path_fcb = malloc(sizeof(char)*29);
				strcpy(path_fcb, "fs/fcb");
				t_fcb* fcb;

				fcb = iniciar_fcb(path_fcb);

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
				strcpy(path_fcb, "fs/fcb.noexiste");
				t_fcb* fcb;

				fcb = iniciar_fcb(path_fcb);

				should_ptr(fcb) be null;

				free(path_fcb);
				//free(superbloque);
			}end

		}end

}
