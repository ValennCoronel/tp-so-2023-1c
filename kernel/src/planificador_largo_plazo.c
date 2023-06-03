#include "planificador_largo_plazo.h"

t_queue* cola_new;
t_queue* cola_ready;
t_pcb* proceso_ejecutando;
t_temporal* rafaga_proceso_ejecutando;

sem_t consumidor;
//TODO cambiar estos por un mutex real
sem_t m_cola_ready;
sem_t m_cola_new;



void inicializar_colas_y_semaforos(){
	cola_new = queue_create();
	cola_ready = queue_create();
	sem_init(&m_cola_ready,0,1);
	sem_init(&m_cola_new, 0, 1);
	sem_init(&consumidor,0,0);
}


void *planificar_nuevos_procesos_largo_plazo(void *arg){

	planificar_nuevos_procesos_largo_plazo_args* args = (planificar_nuevos_procesos_largo_plazo_args*) arg;
	int grado_max_multiprogramacion = args->grado_max_multiprogramacion;
	int conexion_memoria = args->conexion_memoria;

	while(1){
		sem_wait(&m_cola_ready);
		int tamanio_cola_ready = queue_size(cola_ready);
		sem_post(&m_cola_ready);
		sem_wait(&m_cola_new);
		int tamanio_cola_new = queue_size(cola_new);
		sem_post(&m_cola_new);

		if(tamanio_cola_ready == 0 && tamanio_cola_new != 0 ){
			agregar_proceso_a_ready(conexion_memoria);

		} else if(tamanio_cola_new != 0){
			//verificar si se lo puede admitir a la cola de ready
			if(puede_ir_a_ready(grado_max_multiprogramacion)){
				agregar_proceso_a_ready(conexion_memoria);
			}
		}
	}

	return NULL;
}

// si el proceso no es new, no es necesario el socket de memoria
void agregar_proceso_a_ready(int conexion_memoria){
	sem_wait(&m_cola_new);
	t_pcb* proceso_new_a_ready = queue_pop(cola_new);
	sem_post(&m_cola_new);

	//si el proceso es nuevo se calcula tiempo de llegada a ready en milisegundos
	// y se pide la tabla de segmentos a memoria
	if(proceso_new_a_ready->tiempo_llegada_rady == 0){

		//TODO descomentar cuando este hecho esta parte de memoria
		//proceso_new_a_ready->tabla_segmentos = obtener_tabla_segmentos(conexion_memoria);

		proceso_new_a_ready->tiempo_llegada_rady = temporal_gettime(proceso_new_a_ready->temporal_ready);
		temporal_stop(proceso_new_a_ready->temporal_ready);
		temporal_destroy(proceso_new_a_ready->temporal_ready);
		proceso_new_a_ready->temporal_ready= NULL;

		// si es nuevo no tiene rafaga anterior
		proceso_new_a_ready->rafaga_anterior = 0;
	}


	//inicio el cronomitro para el hrrn, si es fifo no lo va a usar
	proceso_new_a_ready->temporal_ultimo_desalojo = temporal_create();


	sem_wait(&m_cola_ready);
	queue_push(cola_ready, proceso_new_a_ready);
	char *pids = listar_pids_cola_ready();
	sem_post(&m_cola_ready);

	log_info(logger, "Cola Ready FIFO: [%s]", pids);

	//si no hay nadie ejecutandose, lo pone a ejecutar, sino va a quedar en la espera en la cola ready
	if(proceso_ejecutando == NULL){
		sem_post(&consumidor);
	}
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

t_list* obtener_tabla_segmentos(int conexion_memoria){
	enviar_mensaje("Iniciar estructuras", conexion_memoria, NUEVO_PROCESO_MEMORIA);

	t_list* tabla_sementos = recibir_paquete(conexion_memoria);

	return tabla_sementos;
}

char* listar_pids_cola_ready(void){

	char** array_pids = string_array_new();
	char* string_pids = string_new();


	t_list* lista_ready = cola_ready->elements;

	int tamano_cola_ready = queue_size(cola_ready);

	for(int i =0; i< tamano_cola_ready; i++){

		t_pcb* item = list_get(lista_ready, i);

		char *PID_string = string_itoa(item->PID);

		string_array_push(&array_pids, PID_string);
	}


	void crear_string(char *pid_string){
	    string_append(&string_pids, pid_string);
	    string_append(&string_pids, ",");
	}

	string_iterate_lines(array_pids,crear_string);

	string_array_destroy(array_pids);

	return string_pids ;
}
