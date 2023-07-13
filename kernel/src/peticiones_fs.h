/*
 * peticiones_fs.h
 *
 *  Created on: Jun 25, 2023
 *      Author: utnso
 */

#ifndef SRC_PETICIONES_FS_H_
#define SRC_PETICIONES_FS_H_

#include <global.h>
#include <utils/utils_cliente.h>
#include <utils/utils_server.h>
#include "kernel.h"

void enviar_peticion_fs(op_code code,t_instruccion* instruccion );
void enviar_peticion_puntero_fs(op_code code,t_instruccion* instruccion,int puntero, int pid);
void abrir_archivo(t_instruccion* instruccion);
void f_open();
void f_close();
void f_seek(int cliente_fd);
void enviar_cola_archivos_bloqueados(t_instruccion* instruccion);
void truncar_archivo();
void leer_archivo();
void escribir_archivo();


#endif /* SRC_PETICIONES_FS_H_ */


