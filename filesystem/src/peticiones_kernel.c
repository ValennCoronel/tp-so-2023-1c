#include "peticiones_kernel.h"



void abrir_archivo(){

//Esta operación consistirá en verificar que exista el FCB correspondiente al
//archivo y en caso de que exista deberá completar las estructuras necesarias y abrir el archivo,
//caso contrario, deberá crear el archivo


	//VARIABLES Y DATOS PARA LA FUNCION
	t_instruccion* instruccion = recibir_instruccion(socket_kernel);
	char* direccion_fcb = string_new();
	strcpy(direccion_fcb, path_fcb);
	strcpy(direccion_fcb, "/");
	strcpy(direccion_fcb, instruccion->parametros[0]);

	//CASO 1: La FCB esta en el diccionario (en memoria) => el archivo existe
	if(dictionary_has_key(fcb_por_archivo, instruccion->parametros[0])){

		enviar_mensaje("OK", socket_kernel, MENSAJE);

	//CASO 2: La FCB no esta en memoria pero si esta en el sistema
	}else {
		t_config* config_FCB = config_create(direccion_fcb);

		//CASO A: La FCB existe => el archivo tambien
		if(config_FCB != NULL){
			//Levanto la fcb
			t_fcb* fcb = iniciar_fcb(config_FCB);
			dictionary_put(fcb_por_archivo, instruccion->parametros[0], fcb);

			enviar_mensaje("OK", socket_kernel, MENSAJE);


		//CASO B: La FCB no existe => el archivo tampoco
		}else{
			//Doy aviso a Kernel
			enviar_mensaje("ERROR", socket_kernel, MENSAJE);

		}

	}

}


//levanta el archivo de fcb  y obtiene los datos del archivo para iniciar el FCB, si no existe lo crea
// el archivo de fcb usa el formato de config de las commons
t_fcb* crear_fcb(t_config* config, t_instruccion* instruccion, char* path){
	config_set_value(config, "NOMBRE_ARCHIVO", instruccion->parametros[0]);
	config_set_value(config, "TAMANIO_ARCHIVO", 0);
	config_set_value(config, "PUNTERO_DIRECTO", string_itoa(obtener_primer_bloque_libre()));
	config_set_value(config, "PUNTERO_INDIRECTO", string_itoa(-1)); // TODO reveer esto

	config_save_in_file(config, path);

	t_fcb* fcb = malloc(sizeof(t_fcb));

	fcb->nombre_archivo = config_get_string_value(config, "NOMBRE_ARCHIVO");
	fcb->tamanio_archivo = config_get_int_value(config, "TAMANIO_ARCHIVO");
	fcb->puntero_directo = config_get_int_value(config, "PUNTERO_DIRECTO");
	fcb->puntero_indirecto = config_get_int_value(config, "PUNTERO_INDIRECTO");

	return fcb;
}


t_fcb* iniciar_fcb(t_config* config){

	// si no existe la config no hace nada
	if(config == NULL){
		return NULL;
	}

	t_fcb* fcb = malloc(sizeof(t_fcb));

	fcb->nombre_archivo = config_get_string_value(config, "NOMBRE_ARCHIVO");
	fcb->tamanio_archivo = config_get_int_value(config, "TAMANIO_ARCHIVO");
	fcb->puntero_directo = config_get_int_value(config, "PUNTERO_DIRECTO");
	fcb->puntero_indirecto = config_get_int_value(config, "PUNTERO_INDIRECTO");




	if(!(fcb->nombre_archivo) || !(fcb->tamanio_archivo) || !(fcb->puntero_directo) || !(fcb->puntero_indirecto) ){
		log_error(logger, "El archivo ligado a la FCB que intentas abrir es erroneo");
		return NULL;
	}


	return fcb;
}


void crear_archivo(){
/*

Para esta operación se deberá crear un archivo FCB correspondiente al nuevo archivo, con tamaño 0 y
sin bloques asociados.
Siempre será posible crear un archivo y por lo tanto esta operación deberá devolver OK. */

	//Cargamos las estructuras necesarias
	t_instruccion* instruccion = recibir_instruccion(socket_kernel);
	char* direccion_fcb = string_new();
	strcpy(direccion_fcb, path_fcb);
	strcpy(direccion_fcb, "/");
	strcpy(direccion_fcb, instruccion->parametros[0]);
	t_config* config_FCB = malloc(sizeof(t_config));
	//creamos la fcb
	t_fcb* fcb = crear_fcb(config_FCB, instruccion, direccion_fcb);

	//Cargo estructuras resantes
	dictionary_put(fcb_por_archivo, instruccion->parametros[0], fcb);
	enviar_mensaje("OK", socket_kernel, MENSAJE);
}


void truncar_archivo(int socket_kernel, int socket_memoria,t_superbloque* superbloque){

	t_instruccion* instruccion_peticion = (t_instruccion*) recibir_instruccion(socket_kernel);

	char* nombre_archivo = instruccion_peticion->parametros[0];
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
	  sacar_bloques(fcb_a_truncar, bloques_a_sacar, superbloque);

   }

    // actualizo tamanio_fcb
   fcb_a_truncar->tamanio_archivo = nuevo_tamano_archivo;

   // respondo a kernel un OK
	enviar_mensaje("OK", socket_kernel, TRUNCAR_ARCHIVO);

}


void leer_archivo(int socket_kernel, int socket_memoria, t_superbloque* superbloque){
	t_instruccion_y_puntero* instruccion_peticion = (t_instruccion_y_puntero*) recibir_instruccion_y_puntero_kernel(socket_kernel);

	int cantidad_de_bytes_a_leer = atoi(instruccion_peticion->instruccion->parametros[2]);

	char* contenido_a_escribir = string_new();

	int cantidad_de_bolques_a_leer = calcular_cantidad_de_bloques(cantidad_de_bytes_a_leer, superbloque);

	for(int i = 0; i< cantidad_de_bolques_a_leer ; i ++){
		string_append(&contenido_a_escribir, leer_en_bloque(instruccion_peticion->puntero +i ,superbloque));
	}

	t_paquete* paquete = crear_paquete(WRITE_MEMORY);
	agregar_a_paquete(paquete, instruccion_peticion->instruccion->opcode, sizeof(char)*instruccion_peticion->instruccion->opcode_lenght );
	agregar_a_paquete(paquete, instruccion_peticion->instruccion->parametros[1], instruccion_peticion->instruccion->parametro2_lenght);
	agregar_a_paquete(paquete,  instruccion_peticion->instruccion->parametros[2],instruccion_peticion->instruccion->parametro3_lenght);
	agregar_a_paquete(paquete,  contenido_a_escribir,strlen(contenido_a_escribir)+1);
	enviar_paquete(paquete, socket_memoria);

	int cod_op = recibir_operacion(socket_memoria);


	if(cod_op != WRITE_MEMORY){
		return;
	}

	int size;
	char*  buffer = recibir_buffer(&size, socket_memoria);


	enviar_mensaje(buffer, socket_kernel,LEER_ARCHIVO);
}


void escribir_archivo(int socket_kernel,int socket_memoria, t_superbloque* superbloque){

	t_instruccion_y_puntero* instruccion_peticion = recibir_instruccion_y_puntero_kernel(socket_kernel);


	t_paquete* paquete = crear_paquete(READ_MEMORY);
	t_instruccion* instruccion = instruccion_peticion->instruccion;

	agregar_a_paquete(paquete, instruccion->opcode, sizeof(char)*instruccion->opcode_lenght );

	agregar_a_paquete(paquete, instruccion->parametros[1], instruccion->parametro2_lenght);
	agregar_a_paquete(paquete, instruccion->parametros[2], instruccion->parametro3_lenght);
	enviar_paquete(paquete, socket_memoria);

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
		char* contenido_bloque_n = malloc(superbloque->block_size + 1);
		memcpy(contenido_bloque_n, buffer + desplazamiento, superbloque->block_size);
		desplazamiento += superbloque->block_size;

		guardar_en_bloque(instruccion_peticion->puntero + i, contenido_bloque_n, superbloque);
	}

	enviar_mensaje("OK", socket_kernel, ESCRIBIR_ARCHIVO);

}


//----------UTILS------------

t_instruccion_y_puntero* recibir_instruccion_y_puntero_kernel(int socket_kernel){

	t_instruccion_y_puntero* instruccion_y_puntero = malloc(sizeof(t_instruccion_y_puntero));


	int size;
	void *  buffer = recibir_buffer(&size, socket_kernel);
	int desplazamiento=0;

	t_instruccion* instruccion = malloc(sizeof(t_instruccion));


	while (desplazamiento<size){


				memcpy(&(instruccion->opcode_lenght), buffer + desplazamiento, sizeof(int));
				desplazamiento+=sizeof(int);
				instruccion->opcode = malloc(instruccion->opcode_lenght);
				memcpy(instruccion->opcode, buffer+desplazamiento, instruccion->opcode_lenght);
				desplazamiento+=instruccion->opcode_lenght;

				memcpy(&(instruccion->parametro1_lenght), buffer+desplazamiento, sizeof(int));
				desplazamiento+=sizeof(int);
				instruccion->parametros[0] = malloc(instruccion->parametro1_lenght);
				memcpy(instruccion->parametros[0], buffer + desplazamiento, instruccion->parametro1_lenght);
				desplazamiento += instruccion->parametro1_lenght;

				memcpy(&(instruccion->parametro2_lenght), buffer+desplazamiento, sizeof(int));
				desplazamiento+=sizeof(int);
				instruccion->parametros[1] = malloc(instruccion->parametro2_lenght);
				memcpy(instruccion->parametros[1], buffer + desplazamiento, instruccion->parametro2_lenght);
				desplazamiento += instruccion->parametro2_lenght;

				memcpy(&(instruccion->parametro3_lenght), buffer+desplazamiento, sizeof(int));
				desplazamiento+=sizeof(int);
				instruccion->parametros[2] = malloc(instruccion->parametro3_lenght);
				memcpy(instruccion->parametros[2], buffer + desplazamiento, instruccion->parametro3_lenght);
				desplazamiento += instruccion->parametro3_lenght;

				memcpy(&(instruccion_y_puntero->puntero), buffer + desplazamiento, sizeof(int));
				desplazamiento += sizeof(int);

	}

 return instruccion_y_puntero;
}


t_instruccion* recibir_instruccion(int socket_cliente){

	int size;
	void *  buffer = recibir_buffer(&size, socket_cliente);
	int desplazamiento=0;

	t_instruccion* instruccion = malloc(sizeof(t_instruccion));
	while (desplazamiento<size){


				memcpy(&(instruccion->opcode_lenght), buffer + desplazamiento, sizeof(int));
				desplazamiento+=sizeof(int);
				instruccion->opcode = malloc(instruccion->opcode_lenght);
				memcpy(instruccion->opcode, buffer+desplazamiento, instruccion->opcode_lenght);
				desplazamiento+=instruccion->opcode_lenght;

				memcpy(&(instruccion->parametro1_lenght), buffer+desplazamiento, sizeof(int));
				desplazamiento+=sizeof(int);
				instruccion->parametros[0] = malloc(instruccion->parametro1_lenght);
				memcpy(instruccion->parametros[0], buffer + desplazamiento, instruccion->parametro1_lenght);
				desplazamiento += instruccion->parametro1_lenght;

				memcpy(&(instruccion->parametro2_lenght), buffer+desplazamiento, sizeof(int));
				desplazamiento+=sizeof(int);
				instruccion->parametros[1] = malloc(instruccion->parametro2_lenght);
				memcpy(instruccion->parametros[1], buffer + desplazamiento, instruccion->parametro2_lenght);
				desplazamiento += instruccion->parametro2_lenght;

				memcpy(&(instruccion->parametro3_lenght), buffer+desplazamiento, sizeof(int));
				desplazamiento+=sizeof(int);
				instruccion->parametros[2] = malloc(instruccion->parametro3_lenght);
				memcpy(instruccion->parametros[2], buffer + desplazamiento, instruccion->parametro3_lenght);
				desplazamiento += instruccion->parametro3_lenght;
	}

	return instruccion;
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
void sacar_bloques(t_fcb* fcb_a_actualizar, int bloques_a_sacar, t_superbloque* superbloque){
	// 1 puntero directo
	// 1 puntero indirecto con x bloques

	int punteros_x_bloque =superbloque->block_size/4; //4 bytes ocupa un puntero

	 if(punteros_x_bloque > bloques_a_sacar){
		  // saco tambien del directo

		 // marco los bloques como libres
		 marcar_bloques_libres_directo( fcb_a_actualizar->puntero_directo);
		 marcar_bloques_libres_indirecto(fcb_a_actualizar->puntero_indirecto, superbloque, punteros_x_bloque);

		  fcb_a_actualizar->puntero_directo = -1;
		  fcb_a_actualizar->puntero_indirecto = -1;

	  } else if(punteros_x_bloque < bloques_a_sacar){
		  // sino saco solo del indirecto
		  int bloques_a_sacar_del_indirecto = bloques_a_sacar - punteros_x_bloque;
		  marcar_bloques_libres_indirecto_hasta(fcb_a_actualizar->puntero_indirecto, bloques_a_sacar_del_indirecto, superbloque, punteros_x_bloque);

	  } else {
		  // sino solo le saco todos los indirectos

		  marcar_bloques_libres_indirecto(fcb_a_actualizar->puntero_indirecto, superbloque, punteros_x_bloque);

		  fcb_a_actualizar->puntero_indirecto = -1;
	  }
}

// actualiza el bitarray de bloques libres
// coloca un 0 al numero de bloque que apunta
void marcar_bloques_libres_directo(uint32_t numero_de_bloque_directo){

	bitarray_clean_bit(bitarray_bloques_libres, numero_de_bloque_directo);

	// actualizo el archivo del bitmap con los nuevos valores
	fseek(bitmap, 0, SEEK_SET);
	fwrite(bitarray_bloques_libres->bitarray,bitarray_bloques_libres->size,1, bitmap);
}

// actualiza el bitarray de bloques libres
// coloca un 0 a todos los bloques del puntero indirecto
void marcar_bloques_libres_indirecto(uint32_t puntero_indirecto, t_superbloque* superbloque, int punteros_x_bloque){

	//leo_archivo_de_bloques
	char* contenido_blouque_indirecto = malloc(sizeof(superbloque->block_size) + 1);
	fseek(bloques, 0, SEEK_SET); // arranco a leer desde el inicio
	fread(contenido_blouque_indirecto,superbloque->block_size,1,bloques);

	for(int i = 0; i < punteros_x_bloque; i++){
		char* puntero_n_string = string_substring(contenido_blouque_indirecto, i*4, 4);
		marcar_bloques_libres_directo(atoi(puntero_n_string));

		free(puntero_n_string);
	}
	marcar_bloques_libres_directo(puntero_indirecto);

	free(contenido_blouque_indirecto);
}

// actualiza el bitarray de bloques libres
// coloca un 0 hasta una cantidad especificada de bloques que hay dento del bloque que apunta el puntero indirecto
void marcar_bloques_libres_indirecto_hasta(uint32_t puntero_indirecto, int numeros_de_bloques_a_sacar, t_superbloque* superbloque, int punteros_x_bloque){
	//leo_archivo_de_bloques

	if(numeros_de_bloques_a_sacar == 0){
		// si no debo sacar nada, no hago nada
		return;
	}

	char* contenido_blouque_indirecto = malloc(sizeof(superbloque->block_size) + 1);
	fseek(bloques, (superbloque->block_size)* puntero_indirecto, SEEK_SET);
	fread(contenido_blouque_indirecto,superbloque->block_size,1,bloques);

	for(int i = punteros_x_bloque ; i != (punteros_x_bloque - numeros_de_bloques_a_sacar) ; i--){
		char* puntero_n_string = string_substring(contenido_blouque_indirecto, (i-1)*4, 4);
		marcar_bloques_libres_directo(atoi(puntero_n_string));

		free(puntero_n_string);
	}
	marcar_bloques_libres_directo(puntero_indirecto);

	free(contenido_blouque_indirecto);
}

// actualiza el fcb y el bitarray de bloques libres, agregando cierta cantidad de bloques especificada
void agregar_bloques(t_fcb* fcb_a_actualizar, int bloques_a_agregar, t_superbloque* superbloque){
	// 1 puntero directo
	// 1 puntero indirecto con x bloques

	int punteros_x_bloque =superbloque->block_size/4; //4 bytes ocupa un puntero

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

		if(!esta_ocupado)
				break;
	}

	colocar_en_ocupado_bitarray_en(puntero_primer_bloque_libre);

	return puntero_primer_bloque_libre;
}

// coloca en true a la posicion indicada y actualiza el archivo del bitmap
void colocar_en_ocupado_bitarray_en(int posicion){
	bitarray_set_bit(bitarray_bloques_libres, posicion);

	fseek(bitmap, 0, SEEK_SET);
	fwrite(bitarray_bloques_libres->bitarray, bitarray_bloques_libres->size, 1, bitmap);

}

//obteiene un bloque libre y actualiza el fcb como puntero directo
void ocupar_bloque_libre_directo(t_fcb* fcb){
	//busca un solo bloque libre y lo setea como puntero directo

	fcb->puntero_directo = obtener_primer_bloque_libre();

}

//agrega un puntero indirecto al fcb y le agrega adentro todos los bloques que necesite sin pasarse de los punteros por bloque
void ocupar_bloque_libre_indirecto(t_fcb* fcb, int bloques_a_agregar, int punteros_x_bloque, t_superbloque* superbloque){
	int puntero_indirecto = obtener_primer_bloque_libre();
	char* punteros_directos = string_new();

	for(int i = 0; i< bloques_a_agregar; i++){
		int puntero_directo_n = obtener_primer_bloque_libre();
		char* puntero_directo_n_string = string_itoa(puntero_directo_n);

		int cantidad_de_digitos = strlen(puntero_directo_n_string);

		if(cantidad_de_digitos < 4){
			// le agrego ceros para que ocupe 4 chars
			while(cantidad_de_digitos != 4){
				string_append(&puntero_directo_n_string, "0");
				cantidad_de_digitos ++;
			}

			char* puntero_directo_n_string_completo = string_reverse(puntero_directo_n_string);


			string_append(&punteros_directos, puntero_directo_n_string_completo);

//			free(puntero_directo_n_string_completo);
//			free(puntero_directo_n_string);
		} else if(cantidad_de_digitos == 4){
			string_append(&punteros_directos, puntero_directo_n_string);
		} else {
			// si pasa que es hueco libre es mayor a 4 digitos, significa que no existe y no hace nada
			return;
		}

		//string_append(&punteros_directos,string_itoa(puntero_directo_n) );
	}

	guardar_en_bloque(puntero_indirecto, punteros_directos, superbloque);

	fcb->puntero_indirecto = puntero_indirecto;

	//free(punteros_directos);
}


// retorna el contenido del bloque pasado por parámetros
char* leer_en_bloque(uint32_t bloque_a_leer, t_superbloque* superbloque){

	int posicion_en_archivo_a_leer = (superbloque->block_size)*bloque_a_leer;

	fseek(bloques, posicion_en_archivo_a_leer, SEEK_SET);
	char* contenido_del_bloque = malloc(superbloque->block_size);
	int datos_leidos = fread(contenido_del_bloque,superbloque->block_size, 1, bloques);

	if(datos_leidos == 0){
		return "";
	}

	return contenido_del_bloque;
}

// En base a un numero de bloque lo guarda en el archivo
// 	se tiene en cuenta de que el contenido_a_guardar tenga menor a igual a los bytes que puede se ocupar en un bloque
void guardar_en_bloque(int numero_de_bloque, char* contenido_a_guardar, t_superbloque* superbloque){

	if(strlen(contenido_a_guardar) > superbloque->block_size){
		return;
	}

	int posicion_en_archivo_a_guardar = (superbloque->block_size)*numero_de_bloque;

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

	char* punteros_directos_leidos = leer_en_bloque(puntero_indirecto, superbloque);

	for(int i = 0; i< bloques_a_agregar; i++){
		int puntero_directo_n = obtener_primer_bloque_libre();
		char* puntero_directo_n_string = string_itoa(puntero_directo_n);

		int cantidad_de_digitos = strlen(puntero_directo_n_string);

		if(cantidad_de_digitos < 4){
			// le agrego ceros para que ocupe 4 chars
			while(cantidad_de_digitos != 4){
				string_append(&puntero_directo_n_string, "0");
				cantidad_de_digitos ++;
			}

			char* puntero_directo_n_string_completo = string_reverse(puntero_directo_n_string);


			string_append(&punteros_directos_leidos, puntero_directo_n_string_completo);

			//free(puntero_directo_n_string_completo);
		} else if(cantidad_de_digitos == 4){
			string_append(&punteros_directos_leidos, puntero_directo_n_string);
		} else {
			// si pasa que es hueco libre es mayor a 4 digitos, significa que no existe y no hace nada
			return;
		}

		//free(puntero_directo_n_string);

	}

	guardar_en_bloque(puntero_indirecto, punteros_directos_leidos, superbloque);
}

/*
 * Persistencia
Todas las operaciones que se realicen sobre los FCBs, Bitmap y Bloques deberán mantenerse actualizadas
en disco a medida que ocurren.
En caso de utilizar la función mmap(), se recomienda investigar el uso de la función msync() para tal fin
 * */
