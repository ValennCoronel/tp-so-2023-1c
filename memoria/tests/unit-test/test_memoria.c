#include "../../src/memoria.h"
#include <commons/collections/list.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cspecs/cspec.h>
#include <pthread.h>


context(memoria_tests){

	describe("t_segmento* determinar_hueco_a_ocupar(t_list* huecos_candidatos, char* algoritmo_asignacion)"){

	}end

	describe("t_list* check_espacio_contiguo(uint32_t tamano_requerido)"){

	}end

	describe("bool check_espacio_no_contiguo(uint32_t tamano_requerido)"){

	}end

	describe("t_tabla_de_segmento* buscar_tabla_de(int pid)"){

	}end

	describe("void agregar_nuevo_segmento_a(int pid, t_segmento* segmento)"){

	}end
}
