#include "../../src/memoria.h"
#include <commons/collections/list.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cspecs/cspec.h>
#include <pthread.h>


void create_hueco(t_list* huecos, uint32_t direccion_base, uint32_t tamano, uint32_t id){
	t_segmento* segmento = malloc(sizeof(t_segmento));
	segmento->direccion_base = direccion_base;
	segmento->id_segmento=id;
	segmento->tamano = tamano;

	list_add(huecos, segmento);
}

void assert_hueco_from_list(t_list* huecos, int index, uint32_t direccion_base, uint32_t tamano, uint32_t id){
	t_segmento* segmento = list_get(huecos, index);

	should_ptr(segmento) not be null;
	should_int(segmento->direccion_base) be equal to (direccion_base);
	should_int(segmento->tamano) be equal to (tamano);
	should_int(segmento->id_segmento) be equal to (id);
}


void assert_hueco(t_segmento* segmento, uint32_t direccion_base, uint32_t tamano, uint32_t id){

	should_ptr(segmento) not be null;
	should_int(segmento->direccion_base) be equal to (direccion_base);
	should_int(segmento->tamano) be equal to (tamano);
	should_int(segmento->id_segmento) be equal to (id);
}

void agregar_tabla(t_list* tablas, uint32_t cantidad, uint32_t pid, t_list* segmentos){

	t_tabla_de_segmento* tabla = malloc(sizeof(t_tabla_de_segmento));

	tabla->cantidad_segmentos= cantidad;
	tabla->pid = pid;
	tabla->segmentos = list_duplicate(segmentos);

	list_add(tablas, tabla);
}

void assert_tabla(t_tabla_de_segmento* tabla, uint32_t cantidad, uint32_t pid){

	should_ptr(tabla) not be null;
	should_int(tabla->cantidad_segmentos) be equal to(cantidad);
	should_int(tabla->pid) be equal to(pid);
}

context(memoria_tests){

	describe("t_segmento* determinar_hueco_a_ocupar(t_list* huecos_candidatos, char* algoritmo_asignacion)"){
		t_list* huecos_candidatos;
		before{
			huecos_candidatos = list_create();
		}end

		after{
			list_destroy(huecos_candidatos);
		}end

		it("en base al algoritmo de asignación que recibe por parámetro, elige el hueco de la lista de huecos y lo devuelve"){
			char* algoritmo_asingacion = malloc(6);
			strcpy(algoritmo_asingacion, "FIRST");

			create_hueco(huecos_candidatos, 0,100,1);
			create_hueco(huecos_candidatos, 100,10,2);

			t_segmento* segmento_a_ocupar =determinar_hueco_a_ocupar(huecos_candidatos, algoritmo_asingacion);

			assert_hueco(segmento_a_ocupar, 0,100,1);

		}end

		it("Si no hay ningun hueco no devuelve nada"){
			char* algoritmo_asingacion = malloc(6);
			strcpy(algoritmo_asingacion, "FIRST");


			t_segmento* segmento_a_ocupar =determinar_hueco_a_ocupar(huecos_candidatos, algoritmo_asingacion);

			should_ptr(segmento_a_ocupar) be null;
		}end

		it("debe funcionar con BEST FIT"){
			char* algoritmo_asingacion = malloc(6);
			strcpy(algoritmo_asingacion, "BEST");

			create_hueco(huecos_candidatos, 0,100,1);
			create_hueco(huecos_candidatos, 100,10,2);
			create_hueco(huecos_candidatos, 110,4,3);
			create_hueco(huecos_candidatos, 114,20,4);

			t_segmento* segmento_a_ocupar =determinar_hueco_a_ocupar(huecos_candidatos, algoritmo_asingacion);

			assert_hueco(segmento_a_ocupar, 110,4,3);

		}end

		it("debe funcionar con WORST FIT"){
			char* algoritmo_asingacion = malloc(6);
			strcpy(algoritmo_asingacion, "WORST");

			create_hueco(huecos_candidatos, 0,100,1);
			create_hueco(huecos_candidatos, 100,10,2);
			create_hueco(huecos_candidatos, 110,4,3);
			create_hueco(huecos_candidatos, 114,20,4);

			t_segmento* segmento_a_ocupar =determinar_hueco_a_ocupar(huecos_candidatos, algoritmo_asingacion);

			assert_hueco(segmento_a_ocupar, 0,100,1);

		}end

		it("debe funcionar con FIRST FIT"){
			char* algoritmo_asingacion = malloc(6);
			strcpy(algoritmo_asingacion, "FIRST");

			create_hueco(huecos_candidatos, 0,20,1);
			create_hueco(huecos_candidatos, 20,10,2);
			create_hueco(huecos_candidatos, 30,4,3);
			create_hueco(huecos_candidatos, 34,50,4);

			t_segmento* segmento_a_ocupar =determinar_hueco_a_ocupar(huecos_candidatos, algoritmo_asingacion);

			assert_hueco(segmento_a_ocupar, 0,20,1);

		}end
	}end

	describe("t_list* check_espacio_contiguo(uint32_t tamano_requerido)"){
		before{
			huecos_libres= list_create();
		}end

		it("debe checkear si hay espacio contiguo y devolver en lista los huecos candidatos"){

			create_hueco(huecos_libres, 0,20,1);
			create_hueco(huecos_libres, 20,10,2);
			create_hueco(huecos_libres, 30,4,3);
			create_hueco(huecos_libres, 34,50,4);

			t_list* candidatos = check_espacio_contiguo(12);

			assert_hueco_from_list(candidatos, 0, 0, 20,1);
			assert_hueco_from_list(candidatos, 1, 34, 50,4);

		}end

		it("si no hay huecos devuelve una lista vacia"){


			t_list* candidatos = check_espacio_contiguo(12);

			should_int(list_size(candidatos)) be equal to(0);

		}end

		it("si hay un hueco solo devuelve la lista con ese hueco si se ajusta"){

			create_hueco(huecos_libres, 0,20,1);

			t_list* candidatos = check_espacio_contiguo(12);

			should_int(list_size(candidatos)) be equal to(1);
			assert_hueco_from_list(candidatos, 0, 0, 20,1);

		}end


	}end

	describe("bool check_espacio_no_contiguo(uint32_t tamano_requerido)"){
		before{
			huecos_libres= list_create();
		}end

		it("debe indicar si hay espacio no contiguo disponible"){

			create_hueco(huecos_libres, 0,20,1);
			create_hueco(huecos_libres, 50,20,2);
			create_hueco(huecos_libres, 80,40,3);
			create_hueco(huecos_libres, 134,50,4);

			bool hay_espacio_no_contiguo = check_espacio_no_contiguo(100);

			should_bool(hay_espacio_no_contiguo) be equal to(true);
		}end

		it("si no lo hay devuelve false"){

			create_hueco(huecos_libres, 0,20,1);
			create_hueco(huecos_libres, 50,20,2);
			create_hueco(huecos_libres, 80,40,3);
			create_hueco(huecos_libres, 134,50,4);// 130 no contiguo en total

			bool hay_espacio_no_contiguo = check_espacio_no_contiguo(300);

			should_bool(hay_espacio_no_contiguo) be equal to(false);
		}end

		it("si no hay huecos libres devuelve false"){

			bool hay_espacio_no_contiguo = check_espacio_no_contiguo(4);

			should_bool(hay_espacio_no_contiguo) be equal to(false);
		}end
	}end

	describe("t_tabla_de_segmento* buscar_tabla_de(int pid)"){
		before{
			tablas_de_segmentos_de_todos_los_procesos= list_create();
		}end

		it("deve encontrar la tabla del proceso correspondiente"){
			int pid = 12;

			t_list* lista_segmentos = list_create();
			create_hueco(lista_segmentos, 0,20,1);

			agregar_tabla(tablas_de_segmentos_de_todos_los_procesos, 1, pid, lista_segmentos);
			agregar_tabla(tablas_de_segmentos_de_todos_los_procesos, 1, 120, lista_segmentos);


			t_tabla_de_segmento* tabla = buscar_tabla_de(pid);

			assert_tabla(tabla,1,pid);
			assert_hueco_from_list(tabla->segmentos, 0, 0, 20,1);

		}end
		it("si no lo encuentra devuelve null"){
			int pid = 12;

			t_list* lista_segmentos = list_create();
			create_hueco(lista_segmentos, 0,20,1);

			agregar_tabla(tablas_de_segmentos_de_todos_los_procesos, 1, 0, lista_segmentos);
			agregar_tabla(tablas_de_segmentos_de_todos_los_procesos, 1, 120, lista_segmentos);


			t_tabla_de_segmento* tabla = buscar_tabla_de(pid);

			should_ptr(tabla) be null;

		}end

		it("si no hay ninguna tabla también devuelve null"){
			int pid = 12;

			t_list* lista_segmentos = list_create();
			create_hueco(lista_segmentos, 0,20,1);



			t_tabla_de_segmento* tabla = buscar_tabla_de(pid);

			should_ptr(tabla) be null;

		}end


	}end

	describe("void agregar_nuevo_segmento_a(int pid, t_segmento* segmento)"){
		before{
			tablas_de_segmentos_de_todos_los_procesos= list_create();
		}end
		it("agrega el segmento recibido por parámetros a la tabla del proceso correspondiente"){
			int pid = 12;

			t_list* lista_segmentos = list_create();
			create_hueco(lista_segmentos, 0,20,1);
			create_hueco(lista_segmentos, -1,-1,1);
			create_hueco(lista_segmentos, -1,-1,1);// segmentos sin usarse

			agregar_tabla(tablas_de_segmentos_de_todos_los_procesos, 1, pid, lista_segmentos);
			agregar_tabla(tablas_de_segmentos_de_todos_los_procesos, 1, 120, lista_segmentos);


			t_segmento* segmento_a_agregar = malloc(sizeof(t_segmento));
			segmento_a_agregar->direccion_base = 12;
			segmento_a_agregar->tamano= 120;
			segmento_a_agregar->id_segmento = 2;


			agregar_nuevo_segmento_a(pid,segmento_a_agregar);

			t_tabla_de_segmento* tabla = buscar_tabla_de(pid);

			assert_tabla(tabla,2,pid);
			assert_hueco_from_list(tabla->segmentos, 0, 0, 20,1);
			assert_hueco_from_list(tabla->segmentos, 1, 12, 120,2);
			assert_hueco_from_list(tabla->segmentos, 2, -1, -1,1);

		}end

		it("si no hay segmentos para ocupar, no hace nada"){
			int pid = 12;

			t_list* lista_segmentos = list_create();
			create_hueco(lista_segmentos, 0,20,1);

			agregar_tabla(tablas_de_segmentos_de_todos_los_procesos, 1, pid, lista_segmentos);
			agregar_tabla(tablas_de_segmentos_de_todos_los_procesos, 1, 120, lista_segmentos);


			t_segmento* segmento_a_agregar = malloc(sizeof(t_segmento));
			segmento_a_agregar->direccion_base = 12;
			segmento_a_agregar->tamano= 120;
			segmento_a_agregar->id_segmento = 2;


			agregar_nuevo_segmento_a(pid,segmento_a_agregar);

			t_tabla_de_segmento* tabla = buscar_tabla_de(pid);

			assert_tabla(tabla,1,pid);
			assert_hueco_from_list(tabla->segmentos, 0, 0, 20,1);

		}end
	}end
}
