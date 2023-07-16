#include "memoria.h"

// Sockets
int socket_cpu;
int socket_kernel;
int socket_memoria;
int socket_fs;

t_list* huecos_libres;
void* espacio_usuario;
t_list* tablas_de_segmentos_de_todos_los_procesos;
t_segmento* segmento_0;
uint32_t tam_memoria;


int main(void){

	//Declaracion variables para config
	char* puerto_escucha;
	int tam_segmento_0;
	int cant_segmentos;
	int retardo_memoria;
	int retardo_compactacion;
	char* algoritmo_asignacion;

	//Iniciar logger y config

	logger = iniciar_logger();
	t_config* config = iniciar_config();


	//Testeo config
	if(config == NULL){
		log_error(logger, "No se pudo iniciar el archivo de configuración !!");

		terminar_programa(logger, config);
	}

	//Levantar datos de config a variables
	puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
	tam_memoria = config_get_int_value(config, "TAM_MEMORIA");
	tam_segmento_0 = config_get_int_value(config, "TAM_SEGMENTO_0");
	cant_segmentos = config_get_int_value(config, "CANT_SEGMENTOS");
	retardo_memoria = config_get_int_value(config, "RETARDO_MEMORIA");
	retardo_compactacion = config_get_int_value(config, "RETARDO_COMPACTACION");
	algoritmo_asignacion = config_get_string_value(config, "ALGORITMO_ASIGNACION");


	//Testeo de carga de variables
	if(!puerto_escucha || !tam_memoria || !tam_segmento_0 || !cant_segmentos || !retardo_memoria || !retardo_compactacion || !algoritmo_asignacion){
		log_error(logger, "Falta una de las siguientes propiedades en el archivo de configuración: 'puerto_escucha', 'tam_memoria', 'tam_segmento_0', 'cant_segmentos', 'retardo_memoria', 'retardo_compactacion', 'algoritmo_asignacion'");

		terminar_programa(logger, config);
	}

	//inicializo estructuras administrativas

	espacio_usuario = malloc(tam_memoria);

	tablas_de_segmentos_de_todos_los_procesos = list_create();

	//inicio el segmento 0
	segmento_0 = malloc(sizeof(t_segmento));
	segmento_0->direccion_base = 0;
	segmento_0->id_segmento = 0;
	segmento_0->tamano = tam_segmento_0;

	// inicio lista de huecos libres inicial
	huecos_libres = list_create();

	t_segmento* hueco_libre_inicial = malloc(sizeof(t_segmento));

	hueco_libre_inicial->direccion_base = segmento_0->tamano;// arranca después del segmento 0
	hueco_libre_inicial->tamano = tam_memoria - segmento_0->tamano;
	hueco_libre_inicial->id_segmento = 1; // no importa, no se usa

	list_add(huecos_libres, hueco_libre_inicial);


	//Escucho conexiones del Kernel, CPU y File System
	socket_memoria = iniciar_servidor(puerto_escucha);

	log_info(logger, "Memoria lista para recibir peticiones");

	manejar_peticiones(algoritmo_asignacion, retardo_memoria, cant_segmentos);

	terminar_programa(logger, config);

} //FIN DEL MAIN


//Funciones de inicio de Config y Logger
t_log* iniciar_logger(void){

	t_log* nuevo_logger = log_create("memoria.log", "Memoria", true, LOG_LEVEL_INFO);

	return nuevo_logger;
}

t_config* iniciar_config(void){
	t_config* nueva_config = config_create("memoria.config");

	return nueva_config;
}



//Funcion para finalizar el programa
void terminar_programa(t_log* logger, t_config* config){
	log_destroy(logger);
	config_destroy(config);
}



//Funcion para manejo de peticiones tanto para kernel, CPU y filesystem
void manejar_peticiones(char* algoritmo_asignacion, int retardo_memoria, int cant_segmentos){

	while(1){
		pthread_t thread;
		uint64_t cliente_fd = (uint64_t) esperar_cliente(socket_memoria);

		t_arg_atender_cliente* argumentos_atender_cliente = malloc(sizeof(t_arg_atender_cliente));
		argumentos_atender_cliente->cliente_fd = cliente_fd;
		argumentos_atender_cliente->algoritmo_asignacion = algoritmo_asignacion;
		argumentos_atender_cliente->retardo_memoria = retardo_memoria;
		argumentos_atender_cliente->cant_segmentos = cant_segmentos;



		pthread_create(&thread, NULL, atender_cliente, (void*) argumentos_atender_cliente);

		pthread_detach(thread);
	}

}

void* atender_cliente(void *args){
	t_arg_atender_cliente* argumentos = (t_arg_atender_cliente*) args;

	char* algoritmo_asignacion = argumentos->algoritmo_asignacion;
	uint64_t cliente_fd = argumentos->cliente_fd;
	uint64_t retardo_memoria = argumentos->retardo_memoria;
	uint64_t cant_segmentos = argumentos->cant_segmentos;

	while(1){
		int cod_op = recibir_operacion(cliente_fd);

			switch (cod_op) {
				case MENSAJE:
					recibir_mensaje(cliente_fd);
					break;
				case HANDSHAKE:
					recibir_handshake(cliente_fd);
					break;
				case NUEVO_PROCESO_MEMORIA:
					crear_nuevo_proceso(cliente_fd, cant_segmentos);
					break;
				case FINALIZAR_PROCESO_MEMORIA:
					finalizar_proceso_memoria(cliente_fd);
					break;
				case CREAR_SEGMENTO:
					create_segment(algoritmo_asignacion,cliente_fd);
					break;
				case ELIMINAR_SEGMENTO:
					delete_segment(cliente_fd);
					break;
				case READ_MEMORY:
					acceder_espacio_usuario_lectura(cliente_fd, retardo_memoria);
					break;
				case WRITE_MEMORY:
					acceder_espacio_usuario_escritura(cliente_fd, retardo_memoria);
					break;
				case COMPACTAR_MEMORIA:
					compactar_memoria(cliente_fd);
					break;
				case -1:
					log_error(logger, "El cliente se desconecto. Terminando servidor");
					return NULL;
				default:
					log_warning(logger,"Operacion desconocida. No quieras meter la pata, code_op: %d", cod_op);
					break;
			}
	}

	return NULL;
}


void compactar_memoria(int cliente_fd)
{
	 // se debe recibir para evitar errores en los sockets, por mas que no se use
	char* mensaje = recibir_mensaje(cliente_fd);

	log_info(logger, "Solicitud de Compactación");


	int cantidad_segmentos = tablas_de_segmentos_de_todos_los_procesos->elements_count;

	for (int i = 0; i < tablas_de_segmentos_de_todos_los_procesos->elements_count; i++)
	{
		t_segmento* segmento_actual = list_get(tablas_de_segmentos_de_todos_los_procesos,i);


        // Obtener el siguiente segmento
		t_segmento* segmento_siguiente = NULL;
		if (i < cantidad_segmentos - 1)
		{
			segmento_siguiente = list_get(tablas_de_segmentos_de_todos_los_procesos, i + 1);
		}

        // Calcular el desplazamiento necesario para compactar el segmento actual
		int desplazamiento = 0;
		if (segmento_siguiente != NULL) {
			desplazamiento = segmento_siguiente->direccion_base - (segmento_actual ->direccion_base + segmento_actual ->tamano);
		}

		// Mover los datos del actual al nuevo espacio contiguo
		if (desplazamiento > 0)
		{
			void* origen = espacio_usuario + segmento_siguiente->direccion_base;
			void* destino = espacio_usuario + segmento_actual->direccion_base + segmento_actual->tamano;
			size_t bytes_a_mover = segmento_siguiente->tamano;
			memmove(destino, origen, bytes_a_mover);

			// Actualizar la dirección base del hueco actual
			segmento_siguiente->direccion_base += segmento_actual->direccion_base + segmento_actual->tamano;
		}

	}

	// actualizo la lista de huecos libres para que haya un solo hueco libre de lo que falta por ocupar de memoria


	void _hueco_destroyer(void* hueco){
		t_segmento* hueco_n = (t_segmento*) hueco;

		free(hueco_n);
	}


	list_destroy_and_destroy_elements(huecos_libres, _hueco_destroyer);

	huecos_libres  = list_create();

	t_segmento* ultimo_segmento = obtener_ultimo_hueco_de_la_tabla();

	t_segmento* unico_hueco_libre = malloc(sizeof(t_segmento));
	unico_hueco_libre->id_segmento = -1;
	unico_hueco_libre->direccion_base = ultimo_segmento->direccion_base + ultimo_segmento->tamano;

	// tamano con el resto de la memoria que falta por ocupar
	unico_hueco_libre->tamano = tam_memoria - unico_hueco_libre->direccion_base;

	list_add(huecos_libres, unico_hueco_libre);


	// responde con las tablas de todos los procesos acutalizada a kernel
	t_paquete* paquete_respuesta = crear_paquete(COMPACTAR_MEMORIA);
	int lista_length = list_size(tablas_de_segmentos_de_todos_los_procesos);
	agregar_a_paquete_sin_agregar_tamanio(paquete_respuesta, &lista_length, sizeof(int));

	for(int i = 0; i < lista_length ; i++){
		t_tabla_de_segmento* tabla_n = list_get(tablas_de_segmentos_de_todos_los_procesos, i);

		agregar_a_paquete_sin_agregar_tamanio(paquete_respuesta, &(tabla_n->cantidad_segmentos), sizeof(uint32_t));
		agregar_a_paquete_sin_agregar_tamanio(paquete_respuesta, &(tabla_n->pid), sizeof(uint32_t));

		int cantidad_segmentos = list_size(tabla_n->segmentos);
		agregar_a_paquete_sin_agregar_tamanio(paquete_respuesta, &cantidad_segmentos, sizeof(int));

		for(int j = 0; j< cantidad_segmentos ; j++){
			t_segmento* segmento_n = list_get(tabla_n->segmentos, j);

			log_info(logger, "PID: %d - Segmento: %d - Base: %d - Tamaño %d", tabla_n->pid, segmento_n->id_segmento, segmento_n->direccion_base, segmento_n->tamano);

			agregar_a_paquete_sin_agregar_tamanio(paquete_respuesta, &(segmento_n->id_segmento),sizeof(uint32_t));
			agregar_a_paquete_sin_agregar_tamanio(paquete_respuesta, &(segmento_n->direccion_base),sizeof(uint32_t));
			agregar_a_paquete_sin_agregar_tamanio(paquete_respuesta, &(segmento_n->tamano),sizeof(uint32_t));
		}

	}

	enviar_paquete(paquete_respuesta, socket_kernel);

}

void create_segment(char* algoritmo_asignacion,uint64_t cliente_fd){
	t_arg_segmento_parametro* valores_recibidos = recibir_segmento_parametro(cliente_fd);

	t_segmento_parametro* peticion_segmento = valores_recibidos->segmento_parametro_recibido;

	int pid = valores_recibidos->pid;


	t_list* huecos_posibles = check_espacio_contiguo(peticion_segmento->tamano_segmento);

	if(list_size(huecos_posibles) == 0){

		bool requiere_compactar = check_espacio_no_contiguo(peticion_segmento->tamano_segmento);

		if(requiere_compactar){
		// si no hay huecos contiguos se debe compactar
			enviar_mensaje("COMPACTA !!", cliente_fd,COMPACTAR_MEMORIA);
			return;
		}

		// sino OUT OF MEMORY

		// aviso de un out of memory a kernel
		enviar_mensaje("OUT OF MEMORY !!", cliente_fd,OUT_OF_MEMORY);

		return;
	}

	// si hay espacio contiguo

	// si es uno solo, no es necesario correr un algoritmo de asingnación
	if(list_size(huecos_posibles) == 1){

		t_segmento* hueco_a_usar = list_get(huecos_posibles, 0);

		usar_hueco(hueco_a_usar, peticion_segmento->tamano_segmento);

		t_segmento* nuevo_segmento = malloc(sizeof(t_segmento));

		nuevo_segmento->direccion_base = hueco_a_usar->direccion_base;
		nuevo_segmento->id_segmento = peticion_segmento->id_segmento;
		nuevo_segmento->tamano = peticion_segmento->tamano_segmento;

		agregar_nuevo_segmento_a(pid, nuevo_segmento);

		log_info(logger, "PID: %d - Crear Segmento: %d - Base: %d - TAMAÑO: %d",pid,peticion_segmento->id_segmento, nuevo_segmento->direccion_base, nuevo_segmento->tamano);

		t_paquete* paquete = crear_paquete(CREAR_SEGMENTO);

		agregar_a_paquete_sin_agregar_tamanio(paquete, &(nuevo_segmento->direccion_base), sizeof(uint32_t));

		enviar_paquete(paquete, cliente_fd);
		return;
	}

	// si son varios es necesario correr el algoritmo de asingnación corespondiente

	t_segmento* hueco_a_ocupar = determinar_hueco_a_ocupar(huecos_posibles, algoritmo_asignacion);

	usar_hueco(hueco_a_ocupar, peticion_segmento->tamano_segmento);

	t_segmento* nuevo_segmento = malloc(sizeof(t_segmento));

	nuevo_segmento->direccion_base = hueco_a_ocupar->direccion_base;
	nuevo_segmento->id_segmento = peticion_segmento->id_segmento;
	nuevo_segmento->tamano = peticion_segmento->tamano_segmento;

	// acutalizo la tabla del proceso correspondiente
	agregar_nuevo_segmento_a(pid, nuevo_segmento);

	log_info(logger, "PID: %d - Crear Segmento: %d - Base: %d - TAMAÑO: %d",pid,peticion_segmento->id_segmento, nuevo_segmento->direccion_base, nuevo_segmento->tamano);


	// envio la direccion base del nuevo segmento a kernel

	t_paquete* paquete = crear_paquete(CREAR_SEGMENTO);

	agregar_a_paquete_sin_agregar_tamanio(paquete, &(nuevo_segmento->direccion_base), sizeof(uint32_t));

	enviar_paquete(paquete, cliente_fd);
}


void delete_segment(int cliente_fd){
//TODO delete segment
}

void acceder_espacio_usuario_lectura(int cliente_fd, int retardo_memoria){
	t_instruccion* instruccion = malloc(sizeof(t_instruccion));
	int pid = 0;
	char* nombre_modulo;

	recibir_instruccion_con_dos_parametros_en(instruccion, &nombre_modulo, &pid, cliente_fd);

	 int direccion_fisica = atoi(instruccion->parametros[0]);
	 int bytes_a_leer = atoi(instruccion->parametros[1]);


	log_info(logger, "PID: %d - Acción: LEER - Dirección física: %d - Tamaño: %d - Origen: %s", pid, direccion_fisica, bytes_a_leer, nombre_modulo);

	 char* contenido_leido = malloc(bytes_a_leer + 1);

	 esperar_por(retardo_memoria);

	 memcpy(contenido_leido, espacio_usuario+direccion_fisica, bytes_a_leer);

	 instruccion_destroy(instruccion);

	 enviar_mensaje(contenido_leido, cliente_fd, READ_MEMORY);

}

void acceder_espacio_usuario_escritura(int cliente_fd, int retardo_memoria){
	 t_instruccion* instruccion = malloc(sizeof(t_instruccion));
	 char* contenido_a_escribir;
	 char* nombre_modulo;
	 int pid = 0;

	 //ignorar el warning porque contenido_a_escribir se inicializa dentro de esta funcion recibir
	 recibir_instruccion_con_dos_parametros_y_contenido_en(instruccion, &contenido_a_escribir, &nombre_modulo, &pid, cliente_fd);

	 // instruccion->parametros[0] --> direccion_fisica
	 // instruccion->parametros[1] --> cantidad de bytes a escribir

	 int direccion_fisica = atoi(instruccion->parametros[0]);
	 int bytes_a_escribir = atoi(instruccion->parametros[1]);


	log_info(logger, "PID: %d - Acción: ESCRIBIR - Dirección física: %d - Tamaño: %d - Origen: %s", pid, direccion_fisica, bytes_a_escribir, nombre_modulo);


	 esperar_por(retardo_memoria);

	 memcpy(espacio_usuario+direccion_fisica,contenido_a_escribir, bytes_a_escribir);

	 free(contenido_a_escribir);
	 instruccion_destroy(instruccion);

	 enviar_mensaje("OK", cliente_fd, WRITE_MEMORY);

}

//crea las estructuras administrativas y enviar tabla de segmentos a Kernel
void crear_nuevo_proceso(int socket_cliente, int cant_segmentos){

	//recibe el pid para el LOG
	int size;
	void* buffer = recibir_buffer(&size, socket_cliente);

	int pid;
	memcpy(&pid, buffer, sizeof(int));

	free(buffer);

	t_tabla_de_segmento* tabla = malloc(sizeof(t_tabla_de_segmento));
	t_paquete* paquete = crear_paquete(NUEVO_PROCESO_MEMORIA);

	log_info(logger, "Creación de Proceso PID: %d", pid);

	tabla->cantidad_segmentos = cant_segmentos;
	tabla->pid = pid;
	tabla->segmentos = list_create();

	list_add(tabla->segmentos, segmento_0);

	for(int i = 0; i< (cant_segmentos-1); i++){
		list_add(tabla->segmentos, crear_segmento_sin_usar());
	}

	agregar_a_paquete_sin_agregar_tamanio(paquete, &(tabla->pid), sizeof(uint32_t));
	agregar_a_paquete_sin_agregar_tamanio(paquete, &(tabla->cantidad_segmentos), sizeof(uint32_t));

	agregar_a_paquete_sin_agregar_tamanio(paquete, &cant_segmentos, sizeof(int));

	for(int i = 0; i<cant_segmentos; i++){
		t_segmento* segmento_n = list_get(tabla->segmentos, i);

		agregar_a_paquete_sin_agregar_tamanio(paquete, &(segmento_n->direccion_base), sizeof(uint32_t));
		agregar_a_paquete_sin_agregar_tamanio(paquete, &(segmento_n->id_segmento), sizeof(uint32_t));
		agregar_a_paquete_sin_agregar_tamanio(paquete, &(segmento_n->tamano), sizeof(uint32_t));
	}

	enviar_paquete(paquete, socket_cliente);

	list_add(tablas_de_segmentos_de_todos_los_procesos, tabla);

}


void finalizar_proceso_memoria(int cliente_fd){

	// recibe pid del kernel
	int size;
	void* buffer = recibir_buffer(&size, cliente_fd);

	int pid;
	memcpy(&pid, buffer, sizeof(int));

	free(buffer);

	// usando el pid, busca la tabla de segmentos del proceso
	t_tabla_de_segmento* tabla_buscada = buscar_tabla_de(pid);

	if(tabla_buscada != NULL){
		// despues de eliminar los segmentos, eliminamos la tabla de segmentos
		destroy_tabla_de_segmentos(tabla_buscada);

		log_info(logger, "Eliminación de Proceso PID: %d", pid);
	} else {
		log_error(logger,"No se encontro la tabla del Proceso PID: %d", pid);
	}

}


// -------- UTILS -------

/*
 * en base al segmento que recibe, busca por dirección base a ese hueco y lo elimina de la lista de huecos libres
 * 	crea otro hueco si el segmento no ocupa el tamaño total del hueco,
 * 	dicho hueco tendrá el tamaño restante que no ocupa el segmento y su dirección base va a arrancar después de la finalización del segmento (osea direccion_base_segmento + tamano_segmento
 * 	si no encuentra el hueco solicitado, no hace nada (porque no va a ocurrir nunca)
 *
 */
void usar_hueco(t_segmento* segmento_a_asignar, int tamano_segmento){

	bool _encontrar_hueco_a_usar(void* hueco){
		t_segmento* hueco_libre = (t_segmento*) hueco;

		return hueco_libre->direccion_base == segmento_a_asignar->direccion_base;
	}

	t_segmento* hueco_encontrado = list_find(huecos_libres, _encontrar_hueco_a_usar);

	if(hueco_encontrado == NULL){
		return;
	}

	if(hueco_encontrado->tamano > tamano_segmento){
		// creo un nuevo segmento por el espacio que no usa el nuevo segmento
		t_segmento* nuevo_hueco = malloc(sizeof(t_segmento));

		// lo que no ocupa el segmento
		nuevo_hueco->tamano = hueco_encontrado->tamano - tamano_segmento;
		nuevo_hueco->direccion_base = tamano_segmento + segmento_a_asignar->direccion_base;//direccion base despues del segmento
		nuevo_hueco->id_segmento = segmento_a_asignar->id_segmento+1; // esto no importa, no se va usar

		// agrego el nuevo hueco a la lista, reemplazando el anterior hueco

		list_replace_by_condition(huecos_libres, _encontrar_hueco_a_usar, (void*) nuevo_hueco);

		return;
	}

	// si el tamaño es igual o si es mayor

	// borro el hueco a usar

	void _hueco_destroyer(void* hueco){
		t_segmento* hueco_libre = (t_segmento*) hueco;

		free(hueco_libre);
	}

	list_remove_and_destroy_by_condition(huecos_libres, _encontrar_hueco_a_usar, _hueco_destroyer);

}

/*
 * agrega el segmento recibido por parámetros a la tabla del proceso correspondiente
 */
void agregar_nuevo_segmento_a(int pid, t_segmento* segmento){

	t_tabla_de_segmento* tabla_del_proceso = buscar_tabla_de(pid);

	if(tabla_del_proceso == NULL || segmento == NULL){
		return;
	}

	 bool _encontrar_segmento_vacio(void* segmento){
		 t_segmento* segmento_posiblemente_vacio = (t_segmento*) segmento;

		 return segmento_posiblemente_vacio->direccion_base == -1 && segmento_posiblemente_vacio->tamano == -1;
	 }

	 // busco un segmento con valores por default
	 	 // osea un segmento no usado
	t_segmento* segmento_a_actualizar = list_find(tabla_del_proceso->segmentos, _encontrar_segmento_vacio);

	if(segmento_a_actualizar == NULL){
		// si un proceso utilizo todos sus segmentos no hace nada
		//	esto es algo que no va a suceder y va a estar contemplado en el psuedocódigo
		return;
	}

	// actualizo la tabla del proceso

	tabla_del_proceso->cantidad_segmentos +=1;

	//uso el segmento
	segmento_a_actualizar->direccion_base = segmento->direccion_base;
	segmento_a_actualizar->id_segmento = segmento->id_segmento;
	segmento_a_actualizar->tamano = segmento->tamano;


	free(segmento);
}

/*
 * en base al algoritmo de asignación que recibe por parámetro, elige el hueco de la lista de huecos y lo devuelve
 */
t_segmento* determinar_hueco_a_ocupar(t_list* huecos_candidatos, char* algoritmo_asignacion){
	t_segmento* hueco_a_ocupar;

	//si no hay ningun hueco devuelve null
	if(list_size(huecos_candidatos) == 0){
		return NULL;
	}

	if(strcmp(algoritmo_asignacion,"FIRST")==0){
		hueco_a_ocupar = first_fit(huecos_candidatos);
	}
	else if(strcmp(algoritmo_asignacion,"WORST")==0){
		hueco_a_ocupar = worst_fit(huecos_candidatos);
	}
	else if(strcmp(algoritmo_asignacion,"BEST")==0){

		hueco_a_ocupar = best_fit(huecos_candidatos);
	}

	return hueco_a_ocupar;
}


/*
 * busca si hay espacio contiguo en la lista de huecos libres
 * 	si no lo hay, entonces devuelve una lista vacia
 * 	Si lo hay, devuelve una lista de los posibles huecos libres (o segmentos libres)
 * 		que se pueden llegar a usar para asignar el segmento.(lista de t_segmento)
 *
 */
t_list* check_espacio_contiguo(uint32_t tamano_requerido){
	t_list* huecos_contiguos ;

	bool _encontrar_espacio_contiguo(void* hueco){
		t_segmento* hueco_libre = (t_segmento*) hueco;

		return hueco_libre->tamano >= tamano_requerido;
	}

	huecos_contiguos = list_filter(huecos_libres, _encontrar_espacio_contiguo);

	return huecos_contiguos;
}

/*
 * busca si hay espacio no contiguo suficiente en la lista de huecos libres
 * 	(osea, suma el tamaño de todos los huecos libres, por más de que no sean contiguos y valida si es menor o igual al tamaño del segmento a crear)
 * devuelve un true si hay espacio no contiguo suficiente o false si no lo hay
 */
bool check_espacio_no_contiguo(uint32_t tamano_requerido){

	int tamanio_huecos_no_contiguos = 0;

	void _sumar_todos_los_tamanios_no_contiguos(void* hueco){
		t_segmento* hueco_libre = (t_segmento*) hueco;

		tamanio_huecos_no_contiguos += hueco_libre->tamano;
	}

	list_iterate(huecos_libres, _sumar_todos_los_tamanios_no_contiguos);

	return tamanio_huecos_no_contiguos >= tamano_requerido;
}

t_arg_segmento_parametro* recibir_segmento_parametro(int cliente_fd){
	t_arg_segmento_parametro* valores_recibidos = malloc(sizeof(t_arg_segmento_parametro));
	t_segmento_parametro* segmento_parametro_recibido = malloc(sizeof(t_segmento_parametro));

	valores_recibidos->segmento_parametro_recibido = segmento_parametro_recibido;

	int size;
	int desplazamiento = 0;
	void* buffer =  recibir_buffer(&size, cliente_fd);


	while(desplazamiento<size){
		memcpy(&(segmento_parametro_recibido->id_segmento), buffer+desplazamiento, sizeof(uint32_t));
		desplazamiento+=sizeof(uint32_t);

		memcpy(&(segmento_parametro_recibido->tamano_segmento), buffer+desplazamiento, sizeof(uint32_t));
		desplazamiento+=sizeof(uint32_t);

		memcpy(&(valores_recibidos->pid), buffer+desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
	}

	return valores_recibidos;
}

/*
 * busca de la lista de t_tabla_segmento la tabla del proceso en base al pid que se recibe por parámetros y lo devuelve
 * si no lo encuentra devuelve NULL
 *
 */
t_tabla_de_segmento* buscar_tabla_de(int pid){

	bool _encontrar_por_pid(void* tabla ){
		t_tabla_de_segmento* tabla_de_segmentos = (t_tabla_de_segmento*) tabla;

		return tabla_de_segmentos->pid == pid;
	}

	return list_find(tablas_de_segmentos_de_todos_los_procesos, _encontrar_por_pid);
}

// --- ALGORITMOS DE ASIGNACION ----
t_segmento* first_fit(t_list* huecos_candidatos){

	return list_get(huecos_candidatos,0);
}

t_segmento* worst_fit(t_list* huecos_candidatos){
	void* _calcular_maximo(void* hueco1, void* hueco2){
		t_segmento* hueco_libre_1 = (t_segmento*) hueco1;
		t_segmento* hueco_libre_2 = (t_segmento*) hueco2;

		return hueco_libre_1->tamano > hueco_libre_2->tamano ? hueco_libre_1 : hueco_libre_2 ;
	}

	return list_get_maximum(huecos_candidatos, _calcular_maximo);
}

t_segmento* best_fit(t_list* huecos_candidatos){

	void* _calcular_minimo(void* hueco1, void* hueco2){
		t_segmento* hueco_libre_1 = (t_segmento*) hueco1;
		t_segmento* hueco_libre_2 = (t_segmento*) hueco2;

		return hueco_libre_1->tamano < hueco_libre_2->tamano ? hueco_libre_1 : hueco_libre_2;
	}

	return list_get_minimum(huecos_candidatos, _calcular_minimo);
}

t_segmento* obtener_ultimo_hueco_de_la_tabla(){
	int cantidad_de_segmentos = tablas_de_segmentos_de_todos_los_procesos->elements_count;

	return (t_segmento*) list_get(tablas_de_segmentos_de_todos_los_procesos,cantidad_de_segmentos-1);
}

t_segmento* crear_segmento_sin_usar(){
	t_segmento* segmento_nuevo = malloc(sizeof(t_segmento));

	segmento_nuevo->direccion_base = -1;
	segmento_nuevo->tamano = -1;
	segmento_nuevo->id_segmento = -1;

	return segmento_nuevo;
}
