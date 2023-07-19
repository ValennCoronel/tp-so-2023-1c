#include "peticiones_kernel.h"


//Esta operación consistirá en verificar que exista el FCB correspondiente al
//archivo y en caso de que exista deberá completar las estructuras necesarias y abrir el archivo,
//caso contrario, deberá crear el archivo
void abrir_archivo(){

	//VARIABLES Y DATOS PARA LA FUNCION
	t_instruccion* instruccion = recibir_instruccion(socket_kernel);

	char* nombre_archivo = strdup(instruccion->parametros[0]);

	log_info(logger, "Abrir Archivo: %s", nombre_archivo);

	char* direccion_fcb = string_new();
	string_append(&direccion_fcb, path_fcb);
	string_append(&direccion_fcb, "/");
	string_append(&direccion_fcb, nombre_archivo);

	//CASO 1: La FCB esta en el diccionario (en memoria) => el archivo existe
	if(dictionary_has_key(fcb_por_archivo, nombre_archivo)){

		enviar_mensaje("OK", socket_kernel, ABRIR_ARCHIVO);

	//CASO 2: La FCB no esta en memoria pero si esta en el sistema
	}else {
		t_config* config_FCB = config_create(direccion_fcb);

		//CASO A: La FCB existe => el archivo tambien
		if(config_FCB != NULL){
			//Levanto la fcb
			t_fcb* fcb = iniciar_fcb(config_FCB);

			config_destroy(config_FCB);

			dictionary_put(fcb_por_archivo, nombre_archivo, fcb);

			enviar_mensaje("OK", socket_kernel, ABRIR_ARCHIVO);


		//CASO B: La FCB no existe => el archivo tampoco
		}else{

			log_info(logger, "El archivo %s no se encontro ", nombre_archivo);
			//Doy aviso a Kernel
			enviar_mensaje("ERROR", socket_kernel, ABRIR_ARCHIVO);

		}

	}

}


/*
	Para esta operación se deberá crear un archivo FCB correspondiente al nuevo archivo, con tamaño 0 y
	sin bloques asociados.
	Siempre será posible crear un archivo y por lo tanto esta operación deberá devolver OK.
*/
void crear_archivo(){
	//Cargamos las estructuras necesarias
	t_instruccion* instruccion = recibir_instruccion(socket_kernel);

	char* nombre_archivo = strdup(instruccion->parametros[0]);

	log_info(logger, "Crear Archivo: %s", nombre_archivo);

	char* direccion_fcb = string_new();
	string_append(&direccion_fcb, path_fcb);
	string_append(&direccion_fcb, "/");
	string_append(&direccion_fcb, nombre_archivo);

	log_info(logger, "Guardando el fcb del archivo %s en %s", nombre_archivo, direccion_fcb);

	//creamos la fcb
	t_fcb* fcb = crear_fcb(instruccion, direccion_fcb);

	//Cargo estructuras resantes
	dictionary_put(fcb_por_archivo, nombre_archivo, fcb);
	enviar_mensaje("OK", socket_kernel, CREAR_ARCHIVO);
}


void truncar_archivo(t_superbloque* superbloque){

	t_instruccion* instruccion_peticion = (t_instruccion*) recibir_instruccion(socket_kernel);

	char* nombre_archivo = strdup(instruccion_peticion->parametros[0]);
	int nuevo_tamano_archivo = atoi(instruccion_peticion->parametros[1]);

	log_info(logger, "Truncar Archivo: %s - Tamaño: %d", nombre_archivo, nuevo_tamano_archivo);

	t_fcb* fcb_a_truncar = dictionary_get(fcb_por_archivo, nombre_archivo);


	  //calculo de cantidad de bloques a truncar
    int numero_de_bloques_nuevo = calcular_cantidad_de_bloques(nuevo_tamano_archivo, superbloque);

    //calculo de cantidad de bloques actuales (antes del truncado :D)
    int numero_de_bloques_actual = calcular_cantidad_de_bloques(fcb_a_truncar->tamanio_archivo, superbloque);



    if(numero_de_bloques_nuevo > numero_de_bloques_actual){
	   //ampliar tamanio
    	int bloques_a_agregar = numero_de_bloques_nuevo - numero_de_bloques_actual;

    	agregar_bloques(fcb_a_truncar, bloques_a_agregar, superbloque);


    } else if(numero_de_bloques_nuevo < numero_de_bloques_actual) {
	   //reducir tamanio

	  int bloques_a_sacar = numero_de_bloques_actual - numero_de_bloques_nuevo;
	  sacar_bloques(fcb_a_truncar, bloques_a_sacar, numero_de_bloques_actual, superbloque);

   }

    // actualizo tamanio_fcb
   fcb_a_truncar->tamanio_archivo = nuevo_tamano_archivo;

   log_info(logger, " puntero directo: %d",fcb_a_truncar->puntero_directo );
   log_info(logger, " puntero indirecto: %d",fcb_a_truncar->puntero_indirecto );

	

 	char* direccion_fcb = string_new();
	string_append(&direccion_fcb, path_fcb);
	string_append(&direccion_fcb, "/");
	string_append(&direccion_fcb, fcb_a_truncar->nombre_archivo);

   	t_config* archivo_fcb = config_create(direccion_fcb);

	if(archivo_fcb == NULL){
		log_error(logger, "No se encontro el fcb del archivo %s", fcb_a_truncar->nombre_archivo);
		enviar_mensaje("ERROR", socket_kernel, TRUNCAR_ARCHIVO);
		return;

	}


	config_set_value(archivo_fcb, "TAMANIO_ARCHIVO", string_itoa(fcb_a_truncar->tamanio_archivo));
	config_set_value(archivo_fcb, "PUNTERO_DIRECTO", string_itoa(fcb_a_truncar->puntero_directo));
	config_set_value(archivo_fcb, "PUNTERO_INDIRECTO", string_itoa(fcb_a_truncar->puntero_indirecto));

	config_save(archivo_fcb );


   // respondo a kernel un OK
	enviar_mensaje("OK", socket_kernel, TRUNCAR_ARCHIVO);

	free(nombre_archivo);
}


void leer_archivo( t_superbloque* superbloque){
	t_instruccion* instruccion = malloc(sizeof(t_instruccion));
	int puntero ;
	int pid ;

	recibir_instruccion_y_puntero_kernel_en(socket_kernel, instruccion, &puntero, &pid);

	int cantidad_de_bytes_a_leer = atoi(instruccion->parametros[2]);
	
	log_info(logger, "Leer Archivo: %s - Puntero: %d - Memoria: %s - Tamaño: %d", instruccion->parametros[0], puntero, instruccion->parametros[1], cantidad_de_bytes_a_leer );

	char* contenido_a_escribir = string_new();

	int cantidad_de_bolques_a_leer = calcular_cantidad_de_bloques(cantidad_de_bytes_a_leer, superbloque);

	log_info(logger, "Cantidad de bloques a leer: %d",cantidad_de_bolques_a_leer );

	for(int i = 0; i< cantidad_de_bolques_a_leer ; i ++){
		void* contenido_del_bloque = leer_en_bloque(instruccion->parametros[0], puntero +i ,superbloque);

		char* contenido_bloque_n_string = malloc(superbloque->block_size);
		memcpy(contenido_bloque_n_string, contenido_del_bloque  ,superbloque->block_size );

		log_info(logger, "lei: %s", contenido_bloque_n_string);
		string_append(&contenido_a_escribir, contenido_bloque_n_string);

		free(contenido_del_bloque);
	}

	log_info(logger, "Lei %s de los bloques",contenido_a_escribir );

	t_paquete* paquete = crear_paquete(WRITE_MEMORY);
	agregar_a_paquete_sin_agregar_tamanio(paquete, &pid, sizeof(int));
	agregar_a_paquete(paquete,  instruccion->opcode, sizeof(char)* instruccion->opcode_lenght );
	agregar_a_paquete(paquete,  instruccion->parametros[1],  instruccion->parametro2_lenght);
	agregar_a_paquete(paquete,   instruccion->parametros[2], instruccion->parametro3_lenght);
	agregar_a_paquete(paquete,  contenido_a_escribir,strlen(contenido_a_escribir)+1);
	
	char* nombre_modulo = string_new();
	string_append(&nombre_modulo, "Filesystem");

	agregar_a_paquete(paquete, nombre_modulo, strlen(nombre_modulo )+1);

	enviar_paquete(paquete, socket_memoria);

	free(nombre_modulo);
	eliminar_paquete(paquete);

	int cod_op = recibir_operacion(socket_memoria);


	if(cod_op != WRITE_MEMORY){
		return;
	}

	char* repuesta_memoria = recibir_mensaje(socket_memoria);

	enviar_mensaje(repuesta_memoria, socket_kernel,LEER_ARCHIVO);

	free(repuesta_memoria);
	instruccion_destroy(instruccion);
}


void escribir_archivo(t_superbloque* superbloque){

	t_instruccion* instruccion = malloc(sizeof(t_instruccion));
	int puntero;
	int pid;

	recibir_instruccion_y_puntero_kernel_en(socket_kernel, instruccion, &puntero, &pid);

	log_info(logger, "“Escribir Archivo: %s - Puntero: %d - Memoria: %s - Tamaño: %s", instruccion->parametros[0], puntero, instruccion->parametros[1], instruccion->parametros[2]);

	t_paquete* paquete = crear_paquete(READ_MEMORY);

	agregar_a_paquete_sin_agregar_tamanio(paquete, &pid, sizeof(int));

	agregar_a_paquete(paquete, instruccion->opcode, sizeof(char)*instruccion->opcode_lenght );

	agregar_a_paquete(paquete, instruccion->parametros[1], instruccion->parametro2_lenght);
	agregar_a_paquete(paquete, instruccion->parametros[2], instruccion->parametro3_lenght);

	char* nombre_modulo = string_new();
	string_append(&nombre_modulo, "Filesystem");

	agregar_a_paquete(paquete, nombre_modulo, strlen(nombre_modulo) + 1);

	enviar_paquete(paquete, socket_memoria);

	free(nombre_modulo);
	eliminar_paquete(paquete);

	int cod_op = recibir_operacion(socket_memoria);

	if(cod_op != READ_MEMORY){
		return;
	}

	int size;
	char* buffer = recibir_buffer(&size, socket_memoria);

	// size == cantidad de bytes a escribir
	int cantidad_de_bolques_a_escribir = calcular_cantidad_de_bloques(size, superbloque);

	int desplazamiento = 0;

	for(int i = 0; i< cantidad_de_bolques_a_escribir ; i ++){
		void* contenido_bloque_n = malloc(superbloque->block_size);
		memcpy(contenido_bloque_n, buffer + desplazamiento, superbloque->block_size);
		desplazamiento += superbloque->block_size;

		guardar_en_bloque(instruccion->parametros[0], puntero + i, contenido_bloque_n, superbloque);
	}

	enviar_mensaje("OK", socket_kernel, ESCRIBIR_ARCHIVO);

	instruccion_destroy(instruccion);
}


//----------UTILS------------

void recibir_instruccion_y_puntero_kernel_en(int socket_kernel, t_instruccion* instruccion, int* puntero, int* pid){

	int size;
	void *  buffer = recibir_buffer(&size, socket_kernel);
	int desplazamiento=0;

	while (desplazamiento<size){

		memcpy(pid, buffer+desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);

		deserializar_instruccion_con_dos_parametros_de(buffer, instruccion, &desplazamiento);

		memcpy(&(instruccion->parametro3_lenght), buffer+desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		instruccion->parametros[2] = malloc(instruccion->parametro3_lenght);
		memcpy(instruccion->parametros[2], buffer + desplazamiento, instruccion->parametro3_lenght);
		desplazamiento += instruccion->parametro3_lenght;

		memcpy(puntero, buffer + desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);

	}

}


t_instruccion* recibir_instruccion(int socket_cliente){

	int size;
	void *  buffer = recibir_buffer(&size, socket_cliente);
	int desplazamiento=0;

	t_instruccion* instruccion = malloc(sizeof(t_instruccion));
	while (desplazamiento<size){


		deserializar_instruccion_con_dos_parametros_de(buffer, instruccion, &desplazamiento);

		memcpy(&(instruccion->parametro3_lenght), buffer+desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		instruccion->parametros[2] = malloc(instruccion->parametro3_lenght);
		memcpy(instruccion->parametros[2], buffer + desplazamiento, instruccion->parametro3_lenght);
		desplazamiento += instruccion->parametro3_lenght;
	}

	return instruccion;
}

//levanta el archivo de fcb  y obtiene los datos del archivo para iniciar el FCB, si no existe lo crea
// el archivo de fcb usa el formato de config de las commons
t_fcb* crear_fcb( t_instruccion* instruccion, char* path){
	t_config* config = malloc(sizeof(t_config));

	config->path = strdup(path);
	config->properties = dictionary_create();

	config_set_value(config, "NOMBRE_ARCHIVO", instruccion->parametros[0]);
	config_set_value(config, "TAMANIO_ARCHIVO", string_itoa(0));
	config_set_value(config, "PUNTERO_DIRECTO", string_itoa(-1));
	config_set_value(config, "PUNTERO_INDIRECTO", string_itoa(-1));

	config_save_in_file(config, path);

	t_fcb* fcb = malloc(sizeof(t_fcb));

	fcb->nombre_archivo = config_get_string_value(config, "NOMBRE_ARCHIVO");
	fcb->tamanio_archivo = config_get_int_value(config, "TAMANIO_ARCHIVO");
	fcb->puntero_directo = config_get_int_value(config, "PUNTERO_DIRECTO");
	fcb->puntero_indirecto = config_get_int_value(config, "PUNTERO_INDIRECTO");

	config_destroy(config);
	return fcb;
}


t_fcb* iniciar_fcb(t_config* config){
	// si no existe la config no hace nada
	if(config == NULL){
		log_error(logger, "El archivo ligado a la FCB que intentas abrir es erroneo o no existe");
		return NULL;
	}

	t_fcb* fcb = malloc(sizeof(t_fcb));

	fcb->nombre_archivo = config_get_string_value(config, "NOMBRE_ARCHIVO");
	fcb->tamanio_archivo = config_get_int_value(config, "TAMANIO_ARCHIVO");
	fcb->puntero_directo = config_get_int_value(config, "PUNTERO_DIRECTO");
	fcb->puntero_indirecto = config_get_int_value(config, "PUNTERO_INDIRECTO");


	if(fcb->nombre_archivo==NULL ){
		log_error(logger, "El archivo ligado a la FCB que intentas abrir es erroneo o no existe");
		return NULL;
	}


	return fcb;
}


//dado un tamanio en bytes, calcula cuantos bloques son
int calcular_cantidad_de_bloques(int tamanio_en_bytes ,t_superbloque* superbloque){
	float numero_bloques_nuevo_aux =tamanio_en_bytes/(float)superbloque->block_size;

	double parte_fraccional, numero_bloques_nuevo;
	parte_fraccional = modf(numero_bloques_nuevo_aux, &numero_bloques_nuevo);


	if( parte_fraccional != 0)
		numero_bloques_nuevo=numero_bloques_nuevo+1;

	return (int)numero_bloques_nuevo;
}

// actualiza el fcb y el bitarray de bloques libres, sacando cierta cantidad de bloques especificada
void sacar_bloques(t_fcb* fcb_a_actualizar, int bloques_a_sacar, int bloques_actuales, t_superbloque* superbloque){
	// 1 puntero directo
	// 1 puntero indirecto con x bloques

	int punteros_x_bloque =superbloque->block_size/4; //4 bytes ocupa un puntero

	int cantidad_de_bloques_maximo = punteros_x_bloque +1;
	int bloques_actuales_en_indirecto = bloques_actuales -1;


	if(fcb_a_actualizar->puntero_directo != -1 && fcb_a_actualizar->puntero_indirecto != -1){
		// CASOS:
		// si bloques_en_puntero_directo=1 bloques_en_indirecto=20 && punteros_x_bloque == 20 ==>
			 // caso 1: bloques_a_sacar ==  bloques_maximo
			 // caso 2: bloques_a_sacar == bloques_en_indirecto
			 // caso 3: bloques_a_sacar < bloques_en_indirecto
		// si bloques_en_puntero_directo=1 bloques_en_indirecto=12 && punteros_x_bloque == 20 ==>
			 // caso 4: bloques_a_sacar ==  bloques_en_indirecto + bloques_en_puntero_directo
			 // caso 5: bloques_a_sacar == bloques_en_indirecto
			 // caso 6: bloques_a_sacar < bloques_en_indirecto

		if(bloques_a_sacar == cantidad_de_bloques_maximo && bloques_actuales == cantidad_de_bloques_maximo){
			 // solo pasa si bloques_a_sacar == bloques actuales
			 marcar_bloques_libres_indirecto(fcb_a_actualizar->nombre_archivo,fcb_a_actualizar->puntero_indirecto, superbloque, punteros_x_bloque);

			 fcb_a_actualizar->puntero_directo = -1;
			 fcb_a_actualizar->puntero_indirecto = -1;

		  } else if(bloques_a_sacar  == punteros_x_bloque && bloques_actuales == cantidad_de_bloques_maximo){
			  // sino solo le saco todos los indirectos

			  marcar_bloques_libres_indirecto(fcb_a_actualizar->nombre_archivo, fcb_a_actualizar->puntero_indirecto, superbloque, punteros_x_bloque);

			  fcb_a_actualizar->puntero_indirecto = -1;


		  } else if(bloques_a_sacar  < punteros_x_bloque && bloques_a_sacar < bloques_actuales_en_indirecto && bloques_actuales == cantidad_de_bloques_maximo){

			  marcar_bloques_libres_indirecto_sin_liberar_puntero_indirecto_hasta(fcb_a_actualizar->nombre_archivo, fcb_a_actualizar->puntero_indirecto, bloques_a_sacar, superbloque, punteros_x_bloque);

		  } else if(bloques_a_sacar == (bloques_actuales_en_indirecto + 1) && bloques_actuales_en_indirecto < punteros_x_bloque){
			  int bloques_indirectos_a_sacar = bloques_a_sacar - 1;
			  marcar_bloques_libres_indirecto_hasta(fcb_a_actualizar->nombre_archivo, fcb_a_actualizar->puntero_indirecto, bloques_indirectos_a_sacar, superbloque, punteros_x_bloque);
			  marcar_bloques_libres_directo( fcb_a_actualizar->puntero_directo);

		  }else if(bloques_a_sacar == bloques_actuales_en_indirecto && bloques_actuales_en_indirecto < punteros_x_bloque){
			  marcar_bloques_libres_indirecto_hasta(fcb_a_actualizar->nombre_archivo, fcb_a_actualizar->puntero_indirecto, bloques_a_sacar, superbloque, punteros_x_bloque);

		  }else if(bloques_a_sacar < bloques_actuales_en_indirecto && bloques_actuales_en_indirecto < punteros_x_bloque){
			  marcar_bloques_libres_indirecto_sin_liberar_puntero_indirecto_hasta(fcb_a_actualizar->nombre_archivo, fcb_a_actualizar->puntero_indirecto, bloques_a_sacar, superbloque, punteros_x_bloque);

		  } else {
			  // otro caso no debería suceder nunca
			  return;
		  }
	} else if(fcb_a_actualizar->puntero_directo != -1 && fcb_a_actualizar->puntero_indirecto == -1 && bloques_a_sacar == 1){

		marcar_bloques_libres_directo( fcb_a_actualizar->puntero_directo);

	} else {
		//cualquier otro caso no deberia suceder nunca
		return;
	}

}

// actualiza el bitarray de bloques libres
// coloca un 0 al numero de bloque que apunta
void marcar_bloques_libres_directo(uint32_t numero_de_bloque_directo){

	bitarray_clean_bit(bitarray_bloques_libres, numero_de_bloque_directo);

	// actualizo el archivo del bitmap con los nuevos valores
	//fseek(bitmap, 0, SEEK_SET);
	//fwrite(bitarray_bloques_libres->bitarray,bitarray_bloques_libres->size,1, bitmap);
}

// actualiza el bitarray de bloques libres
// coloca un 0 a todos los bloques del puntero indirecto
void marcar_bloques_libres_indirecto(char* nombre_archivo, uint32_t puntero_indirecto, t_superbloque* superbloque, int punteros_x_bloque){

	//leo_archivo_de_bloques
	void* contenido_bloque_indirecto = leer_en_bloque(nombre_archivo, puntero_indirecto, superbloque);
	int desplazamiento = 0;

	for(int i = 0; i < punteros_x_bloque; i++){
		uint32_t puntero_n;

		memcpy(&puntero_n, contenido_bloque_indirecto +desplazamiento, sizeof(uint32_t));
		desplazamiento+=  sizeof(uint32_t);

		marcar_bloques_libres_directo(puntero_n);

	}
	marcar_bloques_libres_directo(puntero_indirecto);

	free(contenido_bloque_indirecto);
}

// actualiza el bitarray de bloques libres
// coloca un 0 hasta una cantidad especificada de bloques que hay dento del bloque que apunta el puntero indirecto
void marcar_bloques_libres_indirecto_hasta(char* nombre_archivo, uint32_t puntero_indirecto, int numeros_de_bloques_a_sacar, t_superbloque* superbloque, int punteros_x_bloque){
	//leo_archivo_de_bloques

	if(numeros_de_bloques_a_sacar == 0){
		// si no debo sacar nada, no hago nada
		return;
	}

	void* contenido_bloque_indirecto = leer_en_bloque(nombre_archivo, puntero_indirecto, superbloque);
	int desplazamiento = 0;

	for(int i = 0 ; i < numeros_de_bloques_a_sacar ; i++){
		uint32_t puntero_n;

		memcpy(&puntero_n, contenido_bloque_indirecto +desplazamiento, sizeof(uint32_t));
		desplazamiento+=  sizeof(uint32_t);


		marcar_bloques_libres_directo(puntero_n);
	}
	marcar_bloques_libres_directo(puntero_indirecto);

	free(contenido_bloque_indirecto);
}

void marcar_bloques_libres_indirecto_sin_liberar_puntero_indirecto_hasta(char* nombre_archivo, uint32_t puntero_indirecto, int numeros_de_bloques_a_sacar, t_superbloque* superbloque, int punteros_x_bloque){
	//leo_archivo_de_bloques

	if(numeros_de_bloques_a_sacar == 0){
		// si no debo sacar nada, no hago nada
		return;
	}

	void* contenido_bloque_indirecto = leer_en_bloque(nombre_archivo, puntero_indirecto, superbloque);
	int desplazamiento = 0;

	for(int i = 0 ; i < numeros_de_bloques_a_sacar ; i++){
		uint32_t puntero_n;

		memcpy(&puntero_n, contenido_bloque_indirecto +desplazamiento, sizeof(uint32_t));
		desplazamiento+=  sizeof(uint32_t);


		marcar_bloques_libres_directo(puntero_n);
	}

	free(contenido_bloque_indirecto);
}

// actualiza el fcb y el bitarray de bloques libres, agregando cierta cantidad de bloques especificada
void agregar_bloques(t_fcb* fcb_a_actualizar, int bloques_a_agregar, t_superbloque* superbloque){
	// 1 puntero directo
	// 1 puntero indirecto con x bloques

	int punteros_x_bloque =superbloque->block_size/sizeof(uint32_t); //4 bytes ocupa un puntero

	if(fcb_a_actualizar->puntero_directo == -1 && bloques_a_agregar == 1){
		ocupar_bloque_libre_directo(fcb_a_actualizar);
	} else if(fcb_a_actualizar->puntero_directo == -1 && bloques_a_agregar > 1){
		//ocupa priermo el directo
		ocupar_bloque_libre_directo(fcb_a_actualizar);
		bloques_a_agregar -= 1;
		// luego ocupa el indirecto para el resto de bloques
		ocupar_bloque_libre_indirecto(fcb_a_actualizar, bloques_a_agregar, punteros_x_bloque, superbloque);

	} else if(fcb_a_actualizar->puntero_indirecto == -1){
		//solo ocupa bloques indirectos
		ocupar_bloque_libre_indirecto(fcb_a_actualizar, bloques_a_agregar, punteros_x_bloque, superbloque);

	} else if(fcb_a_actualizar->puntero_indirecto != -1 && fcb_a_actualizar->puntero_directo != -1 ){
		// no va a pasar que se intenten agregar bloques de mas
		ocupar_bloque_libre_indirecto_fatlantes(fcb_a_actualizar, bloques_a_agregar, superbloque);
	}

}

// obtiene el numero del primer bloque libre según el bit array y lo pone como ocupado
int obtener_primer_bloque_libre(){
	int puntero_primer_bloque_libre = 0;

	int bits_bitarray = bitarray_get_max_bit(bitarray_bloques_libres);

	for(puntero_primer_bloque_libre = 0; puntero_primer_bloque_libre< bits_bitarray; puntero_primer_bloque_libre++){
		bool esta_ocupado = bitarray_test_bit(bitarray_bloques_libres, puntero_primer_bloque_libre);

		log_info(logger, "Acceso a Bitmap - Bloque: %d - Estado: %d", puntero_primer_bloque_libre, esta_ocupado);

		if(!esta_ocupado)
				break;
	}

	colocar_en_ocupado_bitarray_en(puntero_primer_bloque_libre);

	return puntero_primer_bloque_libre;
}

// coloca en true a la posicion indicada y actualiza el archivo del bitmap
void colocar_en_ocupado_bitarray_en(int posicion){
	bitarray_set_bit(bitarray_bloques_libres, posicion);

	//fseek(bitmap, 0, SEEK_SET);
	//fwrite(bitarray_bloques_libres->bitarray, bitarray_bloques_libres->size, 1, bitmap);

}

//obteiene un bloque libre y actualiza el fcb como puntero directo
void ocupar_bloque_libre_directo(t_fcb* fcb){
	//busca un solo bloque libre y lo setea como puntero directo

	fcb->puntero_directo = obtener_primer_bloque_libre();

}

//agrega un puntero indirecto al fcb y le agrega adentro todos los bloques que necesite sin pasarse de los punteros por bloque
void ocupar_bloque_libre_indirecto(t_fcb* fcb, int bloques_a_agregar, int punteros_x_bloque, t_superbloque* superbloque){
	int puntero_indirecto = obtener_primer_bloque_libre();
	void* punteros_directos = malloc(bloques_a_agregar*sizeof(uint32_t));

	int desplazamiento = 0;
	for(int i = 0; i< bloques_a_agregar; i++){
		uint32_t puntero_directo_n = (uint32_t) obtener_primer_bloque_libre();

		memcpy(punteros_directos+desplazamiento,&puntero_directo_n, sizeof(uint32_t));
		desplazamiento += sizeof(uint32_t);
	}

	guardar_en_bloque(fcb->nombre_archivo, puntero_indirecto, punteros_directos, superbloque);

	fcb->puntero_indirecto = (uint32_t) puntero_indirecto;

	free(punteros_directos);
}


// retorna el contenido del bloque pasado por parámetros
void* leer_en_bloque(char* nombre_archivo, uint32_t bloque_a_leer, t_superbloque* superbloque){

	int posicion_en_archivo_a_leer = (superbloque->block_size)*bloque_a_leer;

	//TODO VER QUE ES BLOQUE ARCHIVO VS BLOQUE FILESYSTEM
	log_info(logger, "Acceso Bloque - Archivo: %s - Bloque Archivo: %d - Bloque File System %d",nombre_archivo, posicion_en_archivo_a_leer, bloque_a_leer);


	fseek(bloques, posicion_en_archivo_a_leer, SEEK_SET);
	void* contenido_del_bloque = malloc(superbloque->block_size);
	int datos_leidos = fread(contenido_del_bloque,superbloque->block_size, 1, bloques);

	if(datos_leidos == 0){
		log_info(logger, "No se leyo nada del archivo de bloques");
		return NULL;
	}

	return contenido_del_bloque;
}

// En base a un numero de bloque lo guarda en el archivo
// 	se tiene en cuenta de que el contenido_a_guardar tenga menor a igual a los bytes que puede se ocupar en un bloque
void guardar_en_bloque(char* nombre_archivo, int numero_de_bloque, void* contenido_a_guardar, t_superbloque* superbloque){
	

	int posicion_en_archivo_a_guardar = (superbloque->block_size)*numero_de_bloque;

	//TODO VER QUE ES BLOQUE ARCHIVO VS BLOQUE FILESYSTEM
	log_info(logger, "Acceso Bloque - Archivo: %s - Bloque Archivo: %d - Bloque File System %d",nombre_archivo, posicion_en_archivo_a_guardar, numero_de_bloque);



	fseek(bloques, posicion_en_archivo_a_guardar, SEEK_SET);
	fwrite(contenido_a_guardar,superbloque->block_size, 1, bloques);
}

//ocupa los bloques faltantes en el bloque indirecto
// asume que ya hay un par de punteros pero no todos y llena con los que necesita agregar
void ocupar_bloque_libre_indirecto_fatlantes(t_fcb* fcb, int bloques_a_agregar, t_superbloque* superbloque){

	uint32_t puntero_indirecto = fcb->puntero_indirecto;

	if(puntero_indirecto == -1){
		return;
	}
	

	void* punteros_directos_leidos = leer_en_bloque(fcb->nombre_archivo, puntero_indirecto, superbloque);

	int cantiadad_de_punteros_bloque_indirecto= 0;
	int punteros_x_bloque = superbloque->block_size / sizeof(uint32_t);
	int desplazamiento = 0;
	for(int i = 0; i<punteros_x_bloque ; i++){
		uint32_t puntero_posible_n ;

		memcpy(&puntero_posible_n, punteros_directos_leidos+ desplazamiento, sizeof(uint32_t));
		desplazamiento+=  sizeof(uint32_t);

		if(es_puntero_valido(puntero_posible_n, superbloque)){
			cantiadad_de_punteros_bloque_indirecto ++;
		}
	}


	desplazamiento = sizeof(uint32_t)*(cantiadad_de_punteros_bloque_indirecto + 1); // el +1 por el puntero directo

	for(int i = 0; i< bloques_a_agregar; i++){
		uint32_t puntero_directo_n = (uint32_t) obtener_primer_bloque_libre();

		memcpy(punteros_directos_leidos+desplazamiento,&puntero_directo_n, sizeof(uint32_t));
		desplazamiento += sizeof(uint32_t);

	}

	guardar_en_bloque(fcb->nombre_archivo, puntero_indirecto, punteros_directos_leidos, superbloque);
	free(punteros_directos_leidos);
}


bool es_puntero_valido(uint32_t posible_puntero, t_superbloque* superbloque){
	// si se pasa de la cantiadad de bloques existentes, no es un puntero válido
	return posible_puntero < superbloque->block_count;
}
/*
 * Persistencia
Todas las operaciones que se realicen sobre los FCBs, Bitmap y Bloques deberán mantenerse actualizadas
en disco a medida que ocurren.
En caso de utilizar la función mmap(), se recomienda investigar el uso de la función msync() para tal fin
 * */
