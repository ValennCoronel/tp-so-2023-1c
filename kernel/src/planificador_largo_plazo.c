#include "planificador_largo_plazo.h"

t_queue* cola_new;
t_queue* cola_ready;
t_pcb* proceso_ejecutando;
t_temporal* rafaga_proceso_ejecutando;


sem_t consumidor;
sem_t m_cola_ready;
sem_t m_cola_new;
sem_t m_proceso_ejecutando;

t_dictionary* colas_de_procesos_bloqueados_para_cada_archivo;



void inicializar_colas_y_semaforos(){
	cola_new = queue_create();
	cola_ready = queue_create();
	sem_init(&m_cola_ready,0,1);
	sem_init(&m_cola_new, 0, 1);
	sem_init(&m_proceso_ejecutando, 0, 1);
	sem_init(&consumidor,0,0);
}


void *planificar_nuevos_procesos_largo_plazo(void *arg){

	planificar_nuevos_procesos_largo_plazo_args* args = (planificar_nuevos_procesos_largo_plazo_args*) arg;
	int grado_max_multiprogramacion = args->grado_max_multiprogramacion;
	int conexion_memoria = args->conexion_memoria;
	char* algoritmo_planificacion = args->algoritmo_planificacion;


	while(1){

		sem_wait(&m_cola_ready);
		int tamanio_cola_ready = queue_size(cola_ready);
		sem_post(&m_cola_ready);
		sem_wait(&m_cola_new);
		int tamanio_cola_new = queue_size(cola_new);
		sem_post(&m_cola_new);

		if(tamanio_cola_ready == 0 && tamanio_cola_new != 0 ){

			agregar_proceso_a_ready(conexion_memoria, algoritmo_planificacion);

		} else if(tamanio_cola_new != 0){
			//verificar si se lo puede admitir a la cola de ready
			if(puede_ir_a_ready(grado_max_multiprogramacion)){
				agregar_proceso_a_ready(conexion_memoria, algoritmo_planificacion);
			}
		}
	}

	return NULL;
}

// si el proceso no es new, no es necesario el socket de memoria
void agregar_proceso_a_ready(int conexion_memoria, char* algoritmo_planificacion){
	sem_wait(&m_cola_new);
	t_pcb* proceso_new_a_ready = queue_pop(cola_new);
	sem_post(&m_cola_new);

	log_info(logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", proceso_new_a_ready->PID, "NEW", "READY");

	//si el proceso es nuevo se calcula tiempo de llegada a ready en milisegundos
	// y se pide la tabla de segmentos a memoria
	if(proceso_new_a_ready->tiempo_llegada_rady == 0){
		proceso_new_a_ready->tabla_segmentos = obtener_tabla_segmentos(conexion_memoria, proceso_new_a_ready->PID);

		proceso_new_a_ready->tiempo_llegada_rady = temporal_gettime(proceso_new_a_ready->temporal_ready);
		temporal_stop(proceso_new_a_ready->temporal_ready);
		temporal_destroy(proceso_new_a_ready->temporal_ready);
		proceso_new_a_ready->temporal_ready= NULL;

		// si es nuevo no tiene rafaga anterior
		proceso_new_a_ready->rafaga_anterior = 0;
	}


	//inicio el cronometro para el hrrn, si es fifo no lo va a usar
	// es para medir el tiempo de espera en ready
	proceso_new_a_ready->temporal_ultimo_desalojo = temporal_create();


	sem_wait(&m_cola_ready);
	queue_push(cola_ready, proceso_new_a_ready);
	char *pids = listar_pids_cola_ready();
	sem_post(&m_cola_ready);


	log_info(logger, "Cola Ready %s: [%s]",algoritmo_planificacion, pids);

	//si no hay nadie ejecutandose, lo pone a ejecutar, sino va a quedar en la espera en la cola ready
	sem_wait(&m_proceso_ejecutando);
	if(proceso_ejecutando == NULL){
		sem_post(&m_proceso_ejecutando);
		sem_post(&consumidor);
	}
	sem_post(&m_proceso_ejecutando);

	free(pids);
}

void pasar_a_ready(t_pcb* proceso_bloqueado,int grado_max_multiprogramacion){

	while(1){
	if(puede_ir_a_ready(grado_max_multiprogramacion)){

		sem_wait(&m_cola_ready);
		queue_push(cola_ready, proceso_bloqueado);
		//char *pids = listar_pids_cola_ready();
		sem_post(&m_cola_ready);
		break;

	}
}

}

int puede_ir_a_ready(int grado_max_multiprogramacion){

	sem_wait(&m_cola_ready);
	int size_cola_ready = queue_size(cola_ready);
	sem_post(&m_cola_ready);

	if(size_cola_ready < (grado_max_multiprogramacion-1)){
		return 1;
	}

	return 0;
}

void agregar_cola_new(t_pcb* pcb_proceso){
	pcb_proceso->temporal_ready = temporal_create();
	sem_wait(&m_cola_new);
	queue_push(cola_new, pcb_proceso);
	sem_post(&m_cola_new);

	log_info(logger, "Se crea el proceso %d en NEW", pcb_proceso->PID);
}

t_tabla_de_segmento* obtener_tabla_segmentos(int conexion_memoria, int pid){

	t_paquete* paquete = crear_paquete(NUEVO_PROCESO_MEMORIA);

	agregar_a_paquete_sin_agregar_tamanio(paquete, &(pid), sizeof(int));

	enviar_paquete(paquete, conexion_memoria);

	t_tabla_de_segmento* tabla_segmentos = malloc(sizeof(t_tabla_de_segmento));


	int cod_op = recibir_operacion(conexion_memoria);

	if(cod_op == NUEVO_PROCESO_MEMORIA){
		int size, tam_segmentos;
		int desplazamiento = 0;

		void* buffer = recibir_buffer(&size, conexion_memoria);

		while(desplazamiento < size){
			memcpy(&(tabla_segmentos->pid),buffer + desplazamiento, sizeof(uint32_t));
			desplazamiento += sizeof(uint32_t);
			memcpy(&(tabla_segmentos->cantidad_segmentos), buffer + desplazamiento, sizeof(uint32_t));
			desplazamiento += sizeof(uint32_t);

			memcpy(&(tam_segmentos),buffer + desplazamiento, sizeof(int));
			desplazamiento += sizeof(int);

			tabla_segmentos->segmentos = list_create();

			for(int i=0; i<tam_segmentos; i++){

				t_segmento* segmento_n = malloc(sizeof(t_segmento));

				memcpy(&(segmento_n->direccion_base), buffer + desplazamiento, sizeof(uint32_t));
				desplazamiento += sizeof(uint32_t);
				memcpy(&(segmento_n->id_segmento), buffer + desplazamiento, sizeof(uint32_t));
				desplazamiento += sizeof(uint32_t);
				memcpy(&(segmento_n->tamano), buffer + desplazamiento, sizeof(uint32_t));
				desplazamiento += sizeof(uint32_t);

				list_add(tabla_segmentos->segmentos, segmento_n);
			}
		}

	}

	return tabla_segmentos;
}

char* listar_pids_cola_ready(void){

	char** array_pids = string_array_new();


	t_list* lista_ready = cola_ready->elements;

	int tamano_cola_ready = queue_size(cola_ready);

	for(int i =0; i< tamano_cola_ready; i++){

		t_pcb* item = list_get(lista_ready, i);

		char *PID_string = string_itoa(item->PID);

		string_array_push(&array_pids, PID_string);
	}


	char* string_pids = pasar_a_string(array_pids);

	string_array_destroy(array_pids);

	return string_pids ;
}
