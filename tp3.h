#ifndef __tp3_h__
#define __tp3_h__

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include "mpi.h"

#define FALSE 0
#define TRUE 1
#define RANK_CERO 0
#define ROL_SERVIDOR 0
#define ROL_CLIENTE 1


/* Tags para identificar distintas clases de de mensaje */

#define TAG_INICIAL           10    /* rk0 -> cli */
#define TAG_PEDIDO            20    /* cli -> srv */
#define TAG_OTORGADO          30    /* srv -> cli */
#define TAG_LIBERO            40    /* cli -> srv */
#define TAG_TERMINE           50    /* cli -> srv */

#define TAG_PEDIR             60    /* srv -> srv */
#define TAG_TE_PERMITO        70    /* srv -> srv */
#define TAG_ME_VOY            80    /* srv -> srv */


/* Abrevs. de constantes MPI que usaremos seguido */

#define COMM_WORLD      MPI_COMM_WORLD
#define ANY_SOURCE      MPI_ANY_SOURCE
#define ANY_TAG         MPI_ANY_TAG

/* Valores por defecto */

#define DEFAULT_CHAR_INICIAL               'a' - 1
#define DEFAULT_MSEG_COMPUTO_INICIAL       250
#define DEFAULT_MSEG_COMPUTO_FINAL        2000
#define DEFAULT_CANT_ITERACIONES             5
#define DEFAULT_MSEG_COMPUTO_PREVIO        500
#define DEFAULT_MSEG_SECCION_CRITICA       500

#define MSEG_MIN_TRAS_NEWLINE               10
#define INSTR_POR_SECCION_CRITICA           80

/* Parámetros que definen el comportamiento de un cliente */

typedef struct {
    int caracter;
    int mseg_computo_inicial;
    int mseg_computo_final;
    int cant_iteraciones;
    int mseg_computo_previo;
    int mseg_seccion_critica;
} t_params;


/* Dos o tres variables que se usan por doquier */

extern int cant_ranks, mi_rank, mi_nro, mi_rol, mi_char, cant_servidores;

extern int* request_diferido;

extern int faltantes, mi_numero, buf_entrante, buf_numero;


extern void debug(const char* mensaje);

#endif
