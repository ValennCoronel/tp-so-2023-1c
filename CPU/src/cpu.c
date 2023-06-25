#include "cpu.h"

// Sockets
int socket_cpu;
int socket_kernel;
int socket_memoria;
int socket_fs;

int main(void){


	//Declaracion variables para config
	int RETARDO_INSTRUCCION;
	int TAM_MAX_SEGMENTO;
	char* ip_memoria;
	char* puerto_memoria;
	char* puerto_escucha;


	//Iniciar logger y config

	logger = iniciar_logger();
	t_config* config = iniciar_config();


	//Testeo config
	if(config == NULL){
		log_error(logger, "No se pudo iniciar el archivo de configuración !!");
		terminar_programa(logger, config);
	}

	//Levantar datos de config a variables

	RETARDO_INSTRUCCION = config_get_int_value(config, "RETARDO_INSTRUCCION");
	TAM_MAX_SEGMENTO = config_get_int_value(config, "TAM_MAX_SEGMENTO");

	ip_memoria = config_get_string_value(config, "IP_MEMORIA");
	puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
	puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");

	//Testeo de carga de variables
	if(!ip_memoria || !puerto_memoria || !puerto_escucha){
		log_error(logger, "Falta una de las siguientes propiedades en el archivo de configuración: 'RETARDO_INSTRUCCION', 'IP_MEMORIA', 'PUERTO_MEMORIA', 'PUERTO_ESCUCHA', 'TAM_MAX_SEGMENTO'");

		terminar_programa(logger, config);
	}

	//Realizo la conexion con memoria
	int result_conexion_memoria = conectar_memoria(ip_memoria, puerto_memoria);

	//Testeo el resultado de la conexion
	if(result_conexion_memoria == -1){
		log_error(logger, "La CPU no se pudo conectar con el modulo Memoria !!");

		terminar_programa(logger, config);
	}

	log_info(logger, "La CPU se conecto con el modulo Memoria correctamente");

	// Escucho conexiones del Kernel
	socket_cpu = iniciar_servidor(puerto_escucha);

	log_info(logger, "CPU listo para recibir peticiones del Kernel");


	escuchar_peticiones_kernel(logger, RETARDO_INSTRUCCION,TAM_MAX_SEGMENTO);

	terminar_programa(logger, config);

} //FIN DEL MAIN

//Funciones de inicio de Config y Logger
t_log* iniciar_logger(){

	t_log* nuevo_logger = log_create("cpu.log", "CPU", true, LOG_LEVEL_INFO);

	return nuevo_logger;
}

t_config* iniciar_config(void){
	t_config* nueva_config = config_create("cpu.config");

	return nueva_config;
}

//Funcion para finalizar el programa
void terminar_programa(t_log* logger, t_config* config){
	log_destroy(logger);
	config_destroy(config);
	close(socket_memoria);
	close(socket_kernel);
}

//Funcion para crear conexion entre modulos
int conectar_memoria(char* ip, char* puerto){

	socket_memoria = crear_conexion(ip, puerto);

	//enviar handshake
	enviar_mensaje("OK", socket_memoria, HANDSHAKE);

	int size;
	char* buffer = recibir_buffer(&size, socket_memoria);

	if(strcmp(buffer, "ERROR") == 0 || strcmp(buffer, "") == 0){
		return -1;
	}

	return 0;
}

//Funcion para manejo de peticiones del kernel:
void escuchar_peticiones_kernel(t_log* logger, int RETARDO_INSTRUCCION, int TAM_MAX_SEGMENTO){

	socket_kernel = esperar_cliente(socket_cpu);

	while (1) {
			int cod_op = recibir_operacion(socket_kernel);

			switch (cod_op) {
				case MENSAJE:
					recibir_mensaje(socket_kernel);
					break;
				case HANDSHAKE:
					recibir_handshake(socket_kernel);
					break;
				case PETICION_CPU:
					manejar_peticion_al_cpu(RETARDO_INSTRUCCION, TAM_MAX_SEGMENTO);
					break;
				case -1:
					log_error(logger, "El cliente se desconecto. Terminando servidor");
					return;
				default:
					log_warning(logger,"Operacion desconocida. No quieras meter la pata");
					break;
			}
		}

	return ;
}
/**
 * Fetch, Decode
 */
void manejar_peticion_al_cpu(int RETARDO_INSTRUCCION, int TAM_MAX_SEGMENTO)
{

	t_contexto_ejec* contexto = recibir_contexto_de_ejecucion(socket_kernel);

	t_list *lista = contexto->lista_instrucciones;

	bool continuar_con_el_ciclo_instruccion = true;

	while(continuar_con_el_ciclo_instruccion){
		//FETCH
		t_instruccion* instruction = list_get(lista, (contexto)->program_counter-1);

		if(instruction->parametro1_lenght == 0){
			log_info(logger, "PID: %d - Ejecutando: %s ", contexto->pid, instruction->opcode);

		} else if(instruction->parametro2_lenght == 0){
			log_info(logger, "PID: %d - Ejecutando: %s - %s", contexto->pid, instruction->opcode, instruction->parametros[0]);

		}else if(instruction->parametro3_lenght == 0){
			log_info(logger, "PID: %d - Ejecutando: %s - %s %s", contexto->pid, instruction->opcode, instruction->parametros[0], instruction->parametros[1]);

		} else {
			log_info(logger, "PID: %d - Ejecutando: %s - %s %s %s", contexto->pid, instruction->opcode, instruction->parametros[0], instruction->parametros[1], instruction->parametros[2]);
		}


		// al final del fetch, actualizo el program counter
		contexto->program_counter += 1;

		//DECODE y EXECUTE

		if(strcmp(instruction->opcode,"SET")==0)
		{
			//A modo de simular el tiempo que transcurre en la CPU.

			dormir_en_millis(RETARDO_INSTRUCCION);

			manejar_instruccion_set(&contexto, instruction);
		}

		if(strcmp(instruction->opcode,"MOV_IN")==0)
		{	//require traduccion de logica a física
			manejar_instruccion_mov_in(socket_kernel, &contexto, instruction, TAM_MAX_SEGMENTO);
		}

		if(strcmp(instruction->opcode,"MOV_OUT")==0)
		{//require traduccion de logica a física
			manejar_instruccion_mov_out(socket_kernel, contexto, instruction, TAM_MAX_SEGMENTO);
		}

		if(strcmp(instruction->opcode,"I/O")==0)
		{

			enviar_contexto_a_kernel(BLOQUEAR_PROCESO,socket_kernel, contexto );

			// debe romper con el bucle
			continuar_con_el_ciclo_instruccion = false;
		}
		if(strcmp(instruction->opcode,"F_OPEN")==0)
		{
			enviar_contexto_a_kernel(ABRIR_ARCHIVO,socket_kernel, contexto );
			// debe romper con el bucle
			continuar_con_el_ciclo_instruccion = false;
		}
		if(strcmp(instruction->opcode,"F_CLOSE")==0)
		{
			enviar_contexto_a_kernel(CERRAR_ARCHIVO,socket_kernel, contexto );
			// debe romper con el bucle
			continuar_con_el_ciclo_instruccion = false;
		}
		if(strcmp(instruction->opcode,"F_SEEK")==0)
		{
			enviar_contexto_a_kernel(APUNTAR_ARCHIVO,socket_kernel, contexto );
			// debe romper con el bucle
			continuar_con_el_ciclo_instruccion = false;
		}
		if(strcmp(instruction->opcode,"F_READ")==0)
		{

			manejar_instruccion_f_read(socket_kernel, contexto, instruction, TAM_MAX_SEGMENTO);

			// debe romper con el bucle
			continuar_con_el_ciclo_instruccion = false;
		}
		if(strcmp(instruction->opcode,"F_WRITE")==0)
		{

			manejar_instruccion_f_write(socket_kernel, contexto, instruction, TAM_MAX_SEGMENTO);
			// debe romper con el bucle
			continuar_con_el_ciclo_instruccion = false;
		}
		if(strcmp(instruction->opcode,"F_TRUNCATE")==0)
		{
			enviar_contexto_a_kernel(TRUNCAR_ARCHIVO,socket_kernel, contexto );
			// debe romper con el bucle
			continuar_con_el_ciclo_instruccion = false;
		}

		if(strcmp(instruction->opcode,"WAIT")==0)
		{
			enviar_contexto_a_kernel(APROPIAR_RECURSOS,socket_kernel, contexto );

			// debe romper con el bucle
			continuar_con_el_ciclo_instruccion = false;
		}
		if(strcmp(instruction->opcode,"SIGNAL")==0)
		{
			enviar_contexto_a_kernel(DESALOJAR_RECURSOS,socket_kernel, contexto );
			// debe romper con el bucle
			continuar_con_el_ciclo_instruccion = false;
		}
		if(strcmp(instruction->opcode,"CREATE_SEGMENT")==0)
		{
			enviar_contexto_a_kernel(CREAR_SEGMENTO,socket_kernel, contexto );

			// debe romper con el bucle
			continuar_con_el_ciclo_instruccion = false;
		}
		if(strcmp(instruction->opcode,"DELETE_SEGMENT")==0)
		{
			enviar_contexto_a_kernel(ELIMINAR_SEGMENTO,socket_kernel, contexto );

			// debe romper con el bucle
			continuar_con_el_ciclo_instruccion = false;
		}

		if(strcmp(instruction->opcode,"YIELD")==0)
		{
			enviar_contexto_a_kernel(DESALOJAR_PROCESO,socket_kernel, contexto);
			// debe romper con el bucle
			continuar_con_el_ciclo_instruccion = false;
		}
		if(strcmp(instruction->opcode,"EXIT")==0)
		{
			enviar_contexto_a_kernel(FINALIZAR_PROCESO,socket_kernel, contexto);

			// debe romper con el bucle
			continuar_con_el_ciclo_instruccion = false;
		}
	}

	// si rompe con el bucle, destruyo el contexto de ejecucion
	destroy_contexto_de_ejecucion_completo(contexto);
}

void enviar_contexto_a_kernel(op_code opcode,int socket_cliente,t_contexto_ejec* proceso_a_ejecutar ){

	t_paquete* paquete = crear_paquete(opcode);

	agregar_a_paquete_sin_agregar_tamanio(paquete, &(proceso_a_ejecutar->pid), sizeof(int));

	agregar_a_paquete_sin_agregar_tamanio(paquete, &(proceso_a_ejecutar->tamanio_lista), sizeof(int));

		for(int i =0; i<proceso_a_ejecutar->tamanio_lista; i++){
			t_instruccion* instruccion = list_get(proceso_a_ejecutar->lista_instrucciones, i);

			agregar_a_paquete(paquete, instruccion->opcode, sizeof(char)*instruccion->opcode_lenght);

			agregar_a_paquete(paquete, instruccion->parametros[0], instruccion->parametro1_lenght);
			agregar_a_paquete(paquete, instruccion->parametros[1], instruccion->parametro2_lenght);
			agregar_a_paquete(paquete, instruccion->parametros[2], instruccion->parametro3_lenght);

		}

		agregar_a_paquete_sin_agregar_tamanio(paquete, (void *) &(proceso_a_ejecutar->program_counter), sizeof(int));

		agregar_a_paquete(paquete,  proceso_a_ejecutar->registros_CPU->AX, sizeof(char)*4);
		agregar_a_paquete(paquete,  proceso_a_ejecutar->registros_CPU->BX, sizeof(char)*4);
		agregar_a_paquete(paquete,  proceso_a_ejecutar->registros_CPU->CX, sizeof(char)*4);
		agregar_a_paquete(paquete,  proceso_a_ejecutar->registros_CPU->DX, sizeof(char)*4);

		agregar_a_paquete(paquete,  proceso_a_ejecutar->registros_CPU->EAX, sizeof(char)*8);
		agregar_a_paquete(paquete,  proceso_a_ejecutar->registros_CPU->EBX, sizeof(char)*8);
		agregar_a_paquete(paquete,  proceso_a_ejecutar->registros_CPU->ECX, sizeof(char)*8);
		agregar_a_paquete(paquete,  proceso_a_ejecutar->registros_CPU->EDX, sizeof(char)*8);

		agregar_a_paquete(paquete,  proceso_a_ejecutar->registros_CPU->RAX, sizeof(char)*16);
		agregar_a_paquete(paquete,  proceso_a_ejecutar->registros_CPU->RBX, sizeof(char)*16);
		agregar_a_paquete(paquete,  proceso_a_ejecutar->registros_CPU->RCX, sizeof(char)*16);
		agregar_a_paquete(paquete,  proceso_a_ejecutar->registros_CPU->RDX, sizeof(char)*16);


		agregar_a_paquete_sin_agregar_tamanio(paquete, &(proceso_a_ejecutar->tabla_de_segmentos->pid), sizeof(uint32_t));
		agregar_a_paquete_sin_agregar_tamanio(paquete, &(proceso_a_ejecutar->tabla_de_segmentos->cantidad_segmentos), sizeof(uint32_t));

		int tamanio_tabla = list_size(proceso_a_ejecutar->tabla_de_segmentos->segmentos);

		agregar_a_paquete_sin_agregar_tamanio(paquete, &(tamanio_tabla), sizeof(int));

		for(int i =0; i<tamanio_tabla ; i++){
			t_segmento* segmento = list_get(proceso_a_ejecutar->tabla_de_segmentos->segmentos,i);


			agregar_a_paquete_sin_agregar_tamanio(paquete, &(segmento->direccion_base), sizeof(uint32_t));
			agregar_a_paquete_sin_agregar_tamanio(paquete, &(segmento->id_segmento), sizeof(uint32_t));
			agregar_a_paquete_sin_agregar_tamanio(paquete, &(segmento->tamano), sizeof(uint32_t));
		}

		enviar_paquete(paquete, socket_cliente);


		eliminar_paquete(paquete);
}
void enviar_instruccion_a_kernel(op_code code,int cliente_fd,t_instruccion* instruccion )
{
	t_paquete* paquete = crear_paquete(code);

	agregar_a_paquete(paquete, instruccion->opcode, sizeof(char)*instruccion->opcode_lenght );

	agregar_a_paquete(paquete, instruccion->parametros[0], instruccion->parametro1_lenght);
	agregar_a_paquete(paquete, instruccion->parametros[1], instruccion->parametro2_lenght);
	agregar_a_paquete(paquete, instruccion->parametros[2], instruccion->parametro3_lenght);
	enviar_paquete(paquete, cliente_fd);
}
void manejar_instruccion_set(t_contexto_ejec** contexto,t_instruccion* instruccion)
{
	char* registro = strdup(instruccion->parametros[0]);
	char* valor = strdup(instruccion->parametros[1]);
	setear_registro(contexto, registro, valor);
}
void setear_registro(t_contexto_ejec** contexto,char* registro, char* valor)
{
	if(strcmp(registro,"AX")==0)
	{
		strcpy((*contexto)->registros_CPU->AX, string_substring_until(valor,4));

	}else if(strcmp(registro,"BX")==0)
	{
		strcpy((*contexto)->registros_CPU->BX, string_substring_until(valor,4));
	}else if(strcmp(registro,"CX")==0)
	{
		strcpy((*contexto)->registros_CPU->CX, string_substring_until(valor,4));
	}else if(strcmp(registro,"DX")==0)
	{
		strcpy((*contexto)->registros_CPU->DX, string_substring_until(valor,4));
	}else if(strcmp(registro,"EAX")==0)
	{
		strcpy((*contexto)->registros_CPU->EAX, string_substring_until(valor,8));
	}else if(strcmp(registro,"EBX")==0)
	{
		strcpy((*contexto)->registros_CPU->EBX,string_substring_until(valor,8));
	}else if(strcmp(registro,"ECX")==0)
	{
		strcpy((*contexto)->registros_CPU->ECX,string_substring_until(valor,8));
	}else if(strcmp(registro,"EDX")==0)
	{
		strcpy((*contexto)->registros_CPU->EDX,string_substring_until(valor,8));
	}else if(strcmp(registro,"RAX")==0)
	{
		strcpy((*contexto)->registros_CPU->RAX,string_substring_until(valor,16));
	}else if(strcmp(registro,"RBX")==0)
	{
		strcpy((*contexto)->registros_CPU->RBX,string_substring_until(valor,16));
	}else if(strcmp(registro,"RCX")==0)
	{
		strcpy((*contexto)->registros_CPU->RCX,string_substring_until(valor,16));
	}else if(strcmp(registro,"RDX")==0)
	{
		strcpy((*contexto)->registros_CPU->RDX,string_substring_until(valor,16));
	}

}

int traducir_direccion_memoria(int direccion_logica, int TAM_MAX_SEGMENTO, t_contexto_ejec* contexto)
{
	int num_segmento = obtener_numero_segmento(direccion_logica, TAM_MAX_SEGMENTO);
	int desplazamiento_segmento = direccion_logica % TAM_MAX_SEGMENTO;

	// buscar direccion_tabla_de_segmentos

	bool _encontrar_entrada_segmento(void* elemento){
		t_segmento* segmento = (t_segmento*) elemento;

		return segmento->id_segmento == num_segmento;
	}

	t_segmento* entrada_segmento = list_find(contexto->tabla_de_segmentos->segmentos, _encontrar_entrada_segmento);

	int direccion_fisica;

	// verifico si es válida la peticion
	if(entrada_segmento->tamano > desplazamiento_segmento){
		direccion_fisica = desplazamiento_segmento + entrada_segmento->direccion_base;
	} else {

		log_info(logger, "PID: %d - Error SEG_FAULT- Segmento: %d - Offset: %d - Tamaño: %d", contexto->pid, num_segmento, desplazamiento_segmento, entrada_segmento->tamano);


		enviar_contexto_a_kernel(SEG_FAULT, socket_kernel, contexto);

		return -1;
	}

	return direccion_fisica;
}

int obtener_numero_segmento(int direccion_logica, int TAM_MAX_SEGMENTO){

	return floor(direccion_logica / TAM_MAX_SEGMENTO);
}

/**
 * (Registro, Dirección Lógica)
 * Lee el valor de memoria correspondiente a la Dirección Lógica y lo almacena en el Registro.
 *
 */
void manejar_instruccion_mov_in(int cliente_fd, t_contexto_ejec** contexto,t_instruccion* instruccion, int TAM_MAX_SEGMENTO)
{
	char* registro_a_guardar = strdup(instruccion->parametros[0]);

	int direccion_logica =  atoi(instruccion->parametros[1]);

	int numero_segmento = obtener_numero_segmento(direccion_logica, TAM_MAX_SEGMENTO);

	int direccion_fisica = traducir_direccion_memoria(direccion_logica, TAM_MAX_SEGMENTO, *contexto);


	char* valor_leido = leer_valor_de(direccion_fisica, (*contexto)->pid);

	log_info(logger,"PID: %d - Acción: LEER - Segmento: %d - Dirección Física: %s - Valor: %s", (*contexto)->pid,numero_segmento, registro_a_guardar, valor_leido);

	//setear registro con lo devuelto
	setear_registro(contexto, registro_a_guardar, valor_leido);

	free(registro_a_guardar);
	free(valor_leido);
}
/**
 * (Dirección Lógica, Registro)
 * Lee el valor del Registro y lo escribe en la dirección física de memoria obtenida a partir de la Dirección Lógica.
 *
 */

void manejar_instruccion_mov_out(int cliente_fd, t_contexto_ejec* contexto, t_instruccion* instruccion, int TAM_MAX_SEGMENTO)
{

	char* registro_a_leer = strdup(instruccion->parametros[1]);

	int direccion_logica = atoi(instruccion->parametros[0]);

	int numero_segmento = obtener_numero_segmento(direccion_logica, TAM_MAX_SEGMENTO);

	int direccion_fisica = traducir_direccion_memoria(direccion_logica, TAM_MAX_SEGMENTO, contexto);

	char* valor_leido = obtener_valor_del_registro(registro_a_leer, &contexto);

	log_info(logger,"PID: %d - Acción: ESCRIBIR - Segmento: %d - Dirección Física: %d - Valor: %s", contexto->pid,numero_segmento, direccion_fisica, valor_leido);

	escribir_valor_en(direccion_fisica, valor_leido,contexto->pid );

	free(registro_a_leer);
	free(valor_leido);
}

void manejar_instruccion_f_write(int cliente_fd, t_contexto_ejec* contexto, t_instruccion* instruccion, int TAM_MAX_SEGMENTO){

		int direccion_logica = atoi(instruccion->parametros[1]);

		int direccion_fisica = traducir_direccion_memoria(direccion_logica, TAM_MAX_SEGMENTO, contexto);


		// cambio el parámetro de la direccion logica por la direccion física
		char* direccion_fisica_sring = string_itoa(direccion_fisica);

		instruccion->parametro2_lenght = strlen(direccion_fisica_sring)+1;

		instruccion->parametros[1] =direccion_fisica_sring;

		// envio el contexto de ejcucion actualizado al kernel
		enviar_contexto_a_kernel(ESCRIBIR_ARCHIVO,socket_kernel, contexto );

}

void manejar_instruccion_f_read(int cliente_fd, t_contexto_ejec* contexto, t_instruccion* instruccion, int TAM_MAX_SEGMENTO){

		int direccion_logica = atoi(instruccion->parametros[1]);

		int direccion_fisica = traducir_direccion_memoria(direccion_logica, TAM_MAX_SEGMENTO, contexto);


		// cambio el parámetro de la direccion logica por la física
		char* direccion_fisica_sring = string_itoa(direccion_fisica);

		instruccion->parametro2_lenght = strlen(direccion_fisica_sring)+1;

		instruccion->parametros[1] =direccion_fisica_sring;

		// envio el contexto de ejcucion actualizado al kernel
		enviar_contexto_a_kernel(LEER_ARCHIVO,socket_kernel, contexto);

}

void dormir_en_millis(int RETARDO_INSTRUCCION){
	t_temporal* cronometro = temporal_create();

	int tiempo_transcurrido = temporal_gettime(cronometro);

	while(tiempo_transcurrido < RETARDO_INSTRUCCION){
		tiempo_transcurrido = temporal_gettime(cronometro);
	}

	temporal_stop(cronometro);
	temporal_destroy(cronometro);
}

char* obtener_valor_del_registro(char* registro_a_leer, t_contexto_ejec** contexto){
	char* valor_a_leido;

	if(strcmp(registro_a_leer,"AX")==0)
		{
		valor_a_leido = malloc(4);

		valor_a_leido= string_substring_until((*contexto)->registros_CPU->AX, 4);

		}else if(strcmp(registro_a_leer,"BX")==0)
		{
			valor_a_leido = malloc(4);

			valor_a_leido= string_substring_until((*contexto)->registros_CPU->BX, 4);

		}else if(strcmp(registro_a_leer,"CX")==0)
		{
			valor_a_leido = malloc(4);

			valor_a_leido= string_substring_until((*contexto)->registros_CPU->CX, 4);

		}else if(strcmp(registro_a_leer,"DX")==0)
		{

			valor_a_leido = malloc(4);

			valor_a_leido= string_substring_until((*contexto)->registros_CPU->DX, 4);

		}else if(strcmp(registro_a_leer,"EAX")==0)
		{
			valor_a_leido = malloc(8);

			valor_a_leido= string_substring_until((*contexto)->registros_CPU->EAX, 8);

		}else if(strcmp(registro_a_leer,"EBX")==0)
		{

			valor_a_leido = malloc(8);

			valor_a_leido= string_substring_until((*contexto)->registros_CPU->EBX, 8);

		}else if(strcmp(registro_a_leer,"ECX")==0)
		{

			valor_a_leido = malloc(8);

			valor_a_leido= string_substring_until((*contexto)->registros_CPU->ECX, 8);

		}else if(strcmp(registro_a_leer,"EDX")==0)
		{
			valor_a_leido = malloc(8);

			valor_a_leido= string_substring_until((*contexto)->registros_CPU->EDX, 8);

		}else if(strcmp(registro_a_leer,"RAX")==0)
		{
			valor_a_leido = malloc(16);

			valor_a_leido= string_substring_until((*contexto)->registros_CPU->RAX, 16);

		}else if(strcmp(registro_a_leer,"RBX")==0)
		{
			valor_a_leido = malloc(16);

			valor_a_leido= string_substring_until((*contexto)->registros_CPU->RBX, 16);

		}else if(strcmp(registro_a_leer,"RCX")==0)
		{
			valor_a_leido = malloc(16);

			valor_a_leido= string_substring_until((*contexto)->registros_CPU->RCX, 16);

		}else if(strcmp(registro_a_leer,"RDX")==0)
		{
			valor_a_leido = malloc(16);

			valor_a_leido= string_substring_until((*contexto)->registros_CPU->RDX, 16);
		}

	return valor_a_leido;
}


//pide a memoria una escritura del valor valor_a_escribir al modulo memoria
// 	espera hasta recibir un OK de memoria
void escribir_valor_en(int direccion_fisica, char* valor_a_escribir, int pid){
	t_paquete* paquete_a_enviar = crear_paquete(WRITE_MEMORY);

	agregar_a_paquete_sin_agregar_tamanio(paquete_a_enviar, &(direccion_fisica), sizeof(int));

	agregar_a_paquete(paquete_a_enviar, valor_a_escribir, sizeof(valor_a_escribir));

	enviar_paquete(paquete_a_enviar, socket_memoria);

	int cod_op = recibir_operacion(socket_memoria);

	if(cod_op == WRITE_MEMORY){

		// recibo el OK de memoria
		int size;
		void *  buffer = recibir_buffer(&size, socket_memoria);
		char* mensaje = malloc(size);
		memcpy(mensaje, buffer,size);

		if(strcmp(mensaje,"OK") == 0){
			log_info(logger,"PID: %d - escritura en memoria exitosa!! ", pid );
		} else {
			// si esto se llega a ejecutar, debe haber algun error en memoria o aca o algo raro falla
			log_info(logger,"PID: %d - no se pudo escribir en memoria, esto no deberia pasar", pid );
		}
	}


}

//pide a memoria una lectura al modulo memoria
// 	espera hasta recibir el valor leido de memoria
char* leer_valor_de(int direccion_fisica, int pid){
	t_paquete* paquete_a_enviar = crear_paquete(READ_MEMORY);

	agregar_a_paquete_sin_agregar_tamanio(paquete_a_enviar, &(direccion_fisica), sizeof(int));

	enviar_paquete(paquete_a_enviar, socket_memoria);

	int cod_op = recibir_operacion(socket_memoria);

	char* valor_leido;

	if(cod_op == READ_MEMORY){

		// recibo el OK de memoria
		int size;
		void *  buffer = recibir_buffer(&size, socket_memoria);
		valor_leido = malloc(size);
		memcpy(valor_leido, buffer,size);

		log_info(logger,"PID: %d - se leyo de memoria: %s", pid, valor_leido);
	}

	return valor_leido;
}

void destroy_contexto_de_ejecucion_completo(t_contexto_ejec* contexto){
	void destructor_instrucciones (void* arg){
		t_instruccion* inst = (t_instruccion*) arg;

		instruccion_destroy(inst);
	}

	list_destroy_and_destroy_elements(contexto->lista_instrucciones, destructor_instrucciones);

	registro_cpu_destroy(contexto->registros_CPU);

	destroy_tabla_de_segmentos(contexto->tabla_de_segmentos);

	// no libera las estructuras dinámicas
	contexto_ejecucion_destroy(contexto);
}
