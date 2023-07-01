#include "peticiones_kernel.h"


/*
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

		//Cargo estructuras restantes
		t_tabla_global_de_archivos_abiertos* archivo = malloc(sizeof(t_tabla_global_de_archivos_abiertos));
		t_fcb* fcb = dictionary_get(fcb_por_archivo, instruccion->parametros[0]);
		archivo->fileDescriptor = fcb->puntero_directo;
		archivo->file = instruccion->parametros[0];
		archivo->open = 1;
		dictionary_put(tabla_global_de_archivos_abiertos, instruccion->parametros[0], archivo);

		//abro el archivo
		fopen(instruccion->parametros[0]);

	//CASO 2: La FCB no esta en memoria
	}else {
		t_config* config_FCB = config_create(direccion_fcb);

		//CASO A: La FCB existe => el archivo tambien
		if(config_FCB != NULL){
			//Levanto la fcb
			t_fcb* fcb = iniciar_fcb(config_FCB);

			//Cargo estructuras resantes
			t_tabla_global_de_archivos_abiertos* archivo;
			archivo->fileDescriptor = fcb->puntero_directo;
			archivo->file = instruccion->parametros[0];
			archivo->open = 1;
			dictionary_put(tabla_global_de_archivos_abiertos, instruccion->parametros[0], archivo);
			dictionary_put(fcb_por_archivo, instruccion->parametros[0], fcb);

			//abro el archivo
			fopen(instruccion->parametros[0]);

		//CASO B: La FCB no existe => el archivo tampoco
		}else{
			//creamos la fcb
			t_fcb* fcb = crear_fcb(config_FCB, instruccion, direccion_fcb);

			//Cargo estructuras resantes
			t_tabla_global_de_archivos_abiertos* archivo;
			archivo->fileDescriptor = fcb->puntero_directo;
			archivo->file = instruccion->parametros[0];
			archivo->open = 1;
			dictionary_put(tabla_global_de_archivos_abiertos, instruccion->parametros[0], archivo);
			dictionary_put(fcb_por_archivo, instruccion->parametros[0], fcb);

			//TODO creo y abro el archivo

		}

	}

}
*/

//levanta el archivo de fcb  y obtiene los datos del archivo para iniciar el FCB, si no existe lo crea
// el archivo de fcb usa el formato de config de las commons
t_fcb* crear_fcb(t_config* config, t_instruccion* instruccion, char* path){
	config_set_value(config, "NOMBRE_ARCHIVO", instruccion->parametros[0]);
	config_set_value(config, "TAMANIO_ARCHIVO", 0);
	config_set_value(config, "PUNTERO_DIRECTO", encontrar_bloque_libre());
	config_set_value(config, "PUNTERO_INDIRECTO", -1); // TODO reveer esto

	config_save_in_file(config, path);

	t_fcb* fcb = malloc(sizeof(t_fcb));

	fcb->nombre_archivo = config_get_string_value(config, "NOMBRE_ARCHIVO");
	fcb->tamanio_archivo = config_get_int_value(config, "TAMANIO_ARCHIVO");
	fcb->puntero_directo = config_get_int_value(config, "PUNTERO_DIRECTO");
	fcb->puntero_indirecto = config_get_int_value(config, "PUNTERO_INDIRECTO");

	return fcb;
}


t_fcb* iniciar_fcb(t_config* config){

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

int encontrar_bloque_libre(){
	int a=0;
	//TODO Funcion que devuelve el primer bloque libre del bitmap y cambia su estado
	return a ;
}

void crear_archivo(int socket_kernel){
/*

Para esta operación se deberá crear un archivo FCB correspondiente al nuevo archivo, con tamaño 0 y
sin bloques asociados.
Siempre será posible crear un archivo y por lo tanto esta operación deberá devolver OK.

*///enviar_mensaje("OK",socket_kernel);
}



void truncar_archivo(int socket_kernel, int socket_memoria,FILE* bloques,t_superbloque* superbloque){

	t_instruccion* instruccion_peticion = (t_instruccion*) recibir_instruccion(socket_kernel);

	t_fcb* fcb_a_truncar = dictionary_get(fcb_por_archivo,instruccion_peticion->parametros[0]);
    int nuevo_tamano_archivo = atoi(instruccion_peticion->parametros[1]);

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
	fwrite(&bitarray_bloques_libres->bitarray,bitarray_bloques_libres->size,1, bitmap);
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
	char* contenido_blouque_indirecto = malloc(sizeof(superbloque->block_size) + 1);
	fseek(bloques, 0, SEEK_SET); // arranco a leer desde el inicio
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
		ocupar_bloque_libre_indirecto_fatlantes(fcb_a_actualizar, bloques_a_agregar, superbloque);
	}

}

// obtiene el numero del primer bloque libre según el bit array
int obtener_primer_bloque_libre(){
	int puntero_primer_bloque_libre = 0;
	int bits_bitarray = bitarray_get_max_bit(bitarray_bloques_libres);
	for(puntero_primer_bloque_libre = 0; puntero_primer_bloque_libre< bits_bitarray; puntero_primer_bloque_libre++){
		bool esta_ocupado = bitarray_test_bit(bitarray_bloques_libres, puntero_primer_bloque_libre);

		if(!esta_ocupado)
				break;
	}
	return puntero_primer_bloque_libre;
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

	for(int i = 0; i< punteros_x_bloque; i++){
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

			free(puntero_directo_n_string_completo);
		} else if(cantidad_de_digitos == 4){
			string_append(&punteros_directos, puntero_directo_n_string);
		} else {
			// si pasa que es hueco libre es mayor a 4 digitos, significa que no existe y no hace nada
			return;
		}


		free(puntero_directo_n_string);

		string_append(&punteros_directos,string_itoa(puntero_directo_n) );
	}

	guardar_en_bloque(puntero_indirecto, punteros_directos, superbloque);

	//free(punteros_directos);
}

// en base a un array de strings, lo pasa a un string
char* pasar_a_string(char** string_array){
	char* string = string_new();
	void _crear_string(char *contenido_n){
	    string_append(&string, contenido_n);
	}

	string_iterate_lines(string_array,_crear_string);

	return string;
}

// En base a un numero de bloque lo guarda en el archivo
// 	se tiene en cuenta de que el bloque esta vacio  y que el contenido_a_guardar tenga todos los bytes segun el block size
void guardar_en_bloque(int numero_de_bloque, char* contenido_a_guardar, t_superbloque* superbloque){
	int tamanio_archivo = superbloque->block_count * superbloque->block_size;

	fseek(bloques, 0, SEEK_SET);
	char *leido_buffer = calloc(1, tamanio_archivo +1);
	fread(leido_buffer, tamanio_archivo, 1, bloques);


	char** contenido_archivo_de_bloques = string_array_new();

	for(int i = 0; i< (superbloque->block_count); i++){
		char* contenido_bloque_n = string_substring(leido_buffer, i* (superbloque->block_size), superbloque->block_size);
		string_array_push(&contenido_archivo_de_bloques,contenido_bloque_n );
	}

	string_array_replace(contenido_archivo_de_bloques, numero_de_bloque, contenido_a_guardar);

	char* contenido_bloques_nuevo = pasar_a_string(contenido_archivo_de_bloques);

	fseek(bloques, 0, SEEK_SET);
	fwrite(contenido_bloques_nuevo,tamanio_archivo, 1, bloques);
}

void ocupar_bloque_libre_indirecto_fatlantes(t_fcb* fcb, int bloques_a_agregar, t_superbloque* superbloque){
// TODO FRANCO
}

void leer_archivo(int socket_kernel, int socket_memoria, FILE* bloques,int tamanio_bloque){



	t_instruccion_y_puntero* instruccion_peticion = (t_instruccion_y_puntero*) recibir_instruccion_y_puntero_kernel(socket_kernel);

	void * contenido_del_bloque=malloc(atoi(instruccion_peticion->instruccion->parametros[2]));
	fseek(bloques,0,(instruccion_peticion->puntero*tamanio_bloque));
    fread(contenido_del_bloque,instruccion_peticion->instruccion->parametros[2],1,bloques);

	//t_fcb* fcb = dictionary_get(fcb_por_archivo,instruccion_peticion->instruccion->parametros[0]);

	t_paquete* paquete = crear_paquete(WRITE_MEMORY);
	agregar_a_paquete(paquete, instruccion_peticion->instruccion->opcode, sizeof(char)*instruccion_peticion->instruccion->opcode_lenght );
	agregar_a_paquete(paquete, instruccion_peticion->instruccion->parametros[1], instruccion_peticion->instruccion->parametro2_lenght);
	agregar_a_paquete(paquete,  instruccion_peticion->instruccion->parametros[2],instruccion_peticion->instruccion->parametro3_lenght);
	agregar_a_paquete(paquete,  contenido_del_bloque,atoi(instruccion_peticion->instruccion->parametros[2]));
	enviar_paquete(paquete, socket_memoria);

	int cod_op = recibir_operacion(socket_memoria);


	if(cod_op != WRITE_MEMORY){
		return;
	}

	int size;
	char*  buffer = recibir_buffer(&size, socket_memoria);


	enviar_mensaje(buffer, socket_kernel,LEER_ARCHIVO);
}




void enviar_peticion_memoria(op_code code,t_instruccion* instruccion ){
	t_paquete* paquete = crear_paquete(code);

		agregar_a_paquete(paquete, instruccion->opcode, sizeof(char)*instruccion->opcode_lenght );

		agregar_a_paquete(paquete, instruccion->parametros[1], instruccion->parametro2_lenght);
		agregar_a_paquete(paquete, instruccion->parametros[2], instruccion->parametro3_lenght);
		enviar_paquete(paquete, socket_memoria);
}

void escribir_archivo(int socket_kernel,int socket_memoria, FILE* bloques,int tamanio_bloque){

	t_instruccion_y_puntero* instruccion_peticion = (t_instruccion_y_puntero*) recibir_instruccion_y_puntero_kernel(socket_kernel);

	enviar_peticion_memoria(READ_MEMORY,instruccion_peticion->instruccion);

	int cod_op = recibir_operacion(socket_memoria);

	if(cod_op != READ_MEMORY){
		return;
	}

	int size;
	void * buffer = recibir_buffer(&size, socket_memoria);

	//t_fcb* fcb = dictionary_get(fcb_por_archivo,instruccion_peticion->instruccion->parametros[0]);

	fseek(bloques,0,(instruccion_peticion->puntero*tamanio_bloque));
	fwrite(buffer, size, 1, bloques);

	enviar_mensaje("OK", socket_kernel, ESCRIBIR_ARCHIVO);

}


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

/*
 * Persistencia
Todas las operaciones que se realicen sobre los FCBs, Bitmap y Bloques deberán mantenerse actualizadas
en disco a medida que ocurren.
En caso de utilizar la función mmap(), se recomienda investigar el uso de la función msync() para tal fin
 * */
