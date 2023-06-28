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
 *

*///enviar_mensaje("OK",socket_kernel);
}

void truncar_archivo(int socket_kernel, int socket_memoria){

	t_instruccion* instruccion_peticion = (t_instruccion*) recibir_instruccion(socket_kernel);

    t_fcb* peticion_truncado = (t_fcb*) malloc(sizeof(t_fcb));

            peticion_truncado->nombre_archivo = instruccion_peticion->parametros[0];
            peticion_truncado->tamanio_archivo = instruccion_peticion->parametros[1];


            enviar_mensaje("OK", socket_kernel, TRUNCAR_ARCHIVO);

}


void leer_archivo(int socket_kernel, int socket_memoria, FILE* bloques){


	t_instruccion_y_puntero* instruccion_peticion = (t_instruccion_y_puntero*) recibir_instruccion_y_puntero_kernel(socket_kernel);



	enviar_peticion_memoria(WRITE_MEMORY,instruccion_peticion);

	int cod_op = recibir_operacion(socket_memoria);

	if(cod_op != WRITE_MEMORY){
		return;
	}

	int size;
	void *  buffer = recibir_buffer(&size, socket_memoria);

	t_fcb* fcb = dictionary_get(fcb_por_archivo,instruccion_peticion->instruccion->parametros[0]);




	enviar_mensaje("OK", socket_kernel,LEER_ARCHIVO)
}
/*
 	fseek(bloques,0,(instruccion_peticion->puntero*tamanio_bloque));
 	fread(bloques,"r");
        //while(!feof(bloques))
        //{
         //fread(,1,bloques);
        // }

        fread(,1,aArchivo);
 Acceso a espacio de usuario
Tanto CPU como File System pueden, dada una dirección física, solicitar accesos
al espacio de usuario de Memoria. El módulo deberá realizar lo siguiente:

****1Ante un pedido de lectura, devolver el valor que se encuentra en la posición pedida.
****2Ante un pedido de escritura, escribir lo indicado en la posición pedida y responder un mensaje de ‘OK’.

Para simular la realidad y la velocidad de los accesos a Memoria,
cada acceso al espacio de usuario tendrá un tiempo de espera en milisegundos
definido por archivo de configuración.


 */

void enviar_peticion_memoria(op_code code,t_instruccion* instruccion ){
	t_paquete* paquete = crear_paquete(code);

		agregar_a_paquete(paquete, instruccion->opcode, sizeof(char)*instruccion->opcode_lenght );

		agregar_a_paquete(paquete, instruccion->parametros[1], instruccion->parametro2_lenght);
		agregar_a_paquete(paquete, instruccion->parametros[2], instruccion->parametro3_lenght);
		enviar_paquete(paquete, socket_memoria);
}

void escribir_archivo(int socket_kernel,int socket_memoria, FILE* bloques,int tamanio_bloque){

	t_instruccion_y_puntero* instruccion_peticion = (t_instruccion_y_puntero*) recibir_instruccion_y_puntero_kernel(socket_kernel);

	enviar_peticion_memoria(READ_MEMORY,instruccion_peticion);

	int cod_op = recibir_operacion(socket_memoria);

	if(cod_op != READ_MEMORY){
		return;
	}

	int size;
	void * buffer = recibir_buffer(&size, socket_memoria);

	t_fcb* fcb = dictionary_get(fcb_por_archivo,instruccion_peticion->instruccion->parametros[0]);

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
/*
 * Persistencia
Todas las operaciones que se realicen sobre los FCBs, Bitmap y Bloques deberán mantenerse actualizadas
en disco a medida que ocurren.
En caso de utilizar la función mmap(), se recomienda investigar el uso de la función msync() para tal fin
 * */
