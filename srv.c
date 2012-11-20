#include "srv.h"

/*
 *  Servidor que no tiene el "sí fácil" para con su
 *  cliente y se lleva bien con los demás servidores.
 *
 */

void servidor(int mi_cliente)
{
    MPI_Status status; int origen, tag;
    int hay_pedido_local = FALSE;		// cuando empieza no hay pedido local, mi cliente no me pidio nada
    int listo_para_salir = FALSE;		// el servidor todavia no se quiere ir, es decir salir del ciclo
	  	
    int cant_servidores = cant_ranks / 2;	// seteo la cantidad de servidores
 
    int request_diferido[cant_servidores];	/* un arreglo de "booleanos" para decir que servidores estan diferidos, es decir
						que pidieron el mutex para sus clientes y todavia no lo obtuvieron */
	char buffer[540];
	
	
	
	int max;
   	int servidores_comienzo = cant_servidores; 
	int i = 0;
	int me_quiero_ir = FALSE;	// un booleano que dice que mi servidor se quiere ir, es decir que el cliente le mando TAG_TERMINE
	int cant_terminaron = 0;	// inicializo la cantidad de servidores que terminaron en 0
   
 while(i < cant_servidores){	// todos los servidores empiezan sin estar diferidos
	request_diferido[i] = FALSE;
	i++;	
	}

	
	int el_mayor = 0;		// el mayor numero de secuencia enviado o recibido por mi servidor
	
	int faltantes = 1;		/* la cantidad de replies que faltan de todos los servidores para obtener el mutex,
					lo inicializo con un numero positivo asi no me da el mutex de inmediato sin pensarlo*/ 
	int mi_numero = 0;
	int buf_entrante;


    while( ! listo_para_salir ) {
        
        MPI_Recv(&buf_entrante, 1, MPI_INT, ANY_SOURCE, ANY_TAG, COMM_WORLD, &status);
        origen = status.MPI_SOURCE;
        tag = status.MPI_TAG;
        
        if (tag == TAG_PEDIDO) {
            assert(origen == mi_cliente);
	    sprintf(buffer,"Mi cliente %d solicita acceso exclusivo", (mi_cliente-1)/2);
            debug(buffer);
            assert( hay_pedido_local == FALSE);
            hay_pedido_local = TRUE;					// yo quiero el mutex
            el_mayor++;							//incremento el mayor
			mi_numero = el_mayor;
			faltantes = cant_servidores - 1 ;		/*faltan todos los servidores menos yo, empiecen a mandar mensajitos...
									 cant_servidores esta actualizado con la cantidad sin incluir a los finalizados*/
				if (faltantes == 0) {
				debug("solo hay un servidor");
				MPI_Send(NULL, 0, MPI_INT, mi_cliente, TAG_OTORGADO, COMM_WORLD);
				} 	    
			int serv = 0;
	 	    int j = 0;
			while(j < cant_servidores ){
				serv = 2*j;
					if (serv != mi_rank ){		// si no soy yo le mando un mensaje de pedido de permiso
						assert(serv != mi_rank );
						sprintf(buffer, "Yo servidor %d mando un pedido al servidor %d. HpedLoc: %d, elMay: %d",mi_rank/2, serv/2,hay_pedido_local, el_mayor);
		       			debug(buffer);
						MPI_Send(&mi_numero, 1, MPI_INT, serv, TAG_PEDIR, COMM_WORLD);	// le mando el pedido de permiso
					}
				j++;					
			}
	}
                  

		else if (tag == TAG_TE_PERMITO) {
	            assert(origen % 2 == 0 );
        	    sprintf(buffer, "el servidor %d me da permiso para que obtenga el mutex", origen/2);
				debug(buffer);
            	    debug("hay un servidor menos que falta");
			faltantes--;	// decremento faltantes porque un servidor me mando un permiso
			sprintf(buffer,"Me fijo si puedo darle el mutex... al servidor %d le faltan %d permisos", mi_rank/2, faltantes);
			debug(buffer);
			
			
				if (faltantes == 0) {	// si era el ultimo servidor el que faltaba quien me dio permiso, le doy el mutex a mi cliente
					sprintf(buffer,"(me dieron todos los permisos!) le doy el mutex al cliente %d ", (mi_cliente-1)/2);
					debug(buffer);
		        		MPI_Send(NULL, 0, MPI_INT, mi_cliente, TAG_OTORGADO, COMM_WORLD);   // le doy el mutex a mi cliente         
					debug("le mande el mensaje a mi cliente para que obtenga el mutex ");				
					}
        }
        

	    else if (tag == TAG_PEDIR) {
            	assert(origen != mi_rank);
		sprintf(buffer, "Me llego un pedido del servidor %d. HpedLoc: %d, elMay: %d, bufE: %d", origen/2,hay_pedido_local, el_mayor,buf_entrante);
	 	debug(buffer);
           		
			if(!hay_pedido_local || me_quiero_ir){	// si mi cliente no necesita el mutex (me pidio a mi) o me quiero ir 
				assert(hay_pedido_local == FALSE || me_quiero_ir == TRUE);
				sprintf(buffer,"Yo servidor %d no tengo pedidos locales o me quiero ir, le permito al servidor %d que me pidio el mutex \n", mi_rank/2, origen/2);				
				debug(buffer);
				MPI_Send(NULL, 0, MPI_INT, origen, TAG_TE_PERMITO, COMM_WORLD);			// le mando el mensaje de permiso
			}
			else if( mi_numero < buf_entrante || ( mi_numero == buf_entrante && mi_rank < origen ) ){// si mi servidor tiene prioridad
				int serv = origen / 2;					
				sprintf(buffer, "difiero al servidor %d, a mi me faltan %d", origen/2,faltantes);				
				debug(buffer);					
				request_diferido[serv] = TRUE;	// difiero al servidor serv (no le mando mensaje hasta que me llegue el mensaje TAG_LIBERO)

			}else{					// el servidor que me pidio permiso tiene prioridad sobre mi
				sprintf(buffer, "el pedido del servidor %d tiene mas prioridad", origen/2);
				debug(buffer);
				MPI_Send(NULL, 0, MPI_INT, origen, TAG_TE_PERMITO, COMM_WORLD);	// le mando el mensaje de que le permito
				debug("pude llegar a mandar");			
			}
		max = el_mayor;
		if (buf_entrante  > el_mayor){		//elijo el maximo entre el_mayor y el numero que recibi
				max = buf_entrante ;		
			}			
		el_mayor = max;				// actualizo el_mayor
		debug ("actualizo el mayor");
        }


	else if (tag == TAG_LIBERO) {
            assert(origen == mi_cliente);
            debug("Mi cliente libera su acceso exclusivo");
            assert(hay_pedido_local == TRUE);
            hay_pedido_local = FALSE;		//ahora ya no tiene pedido local, es decir, mi cliente no me pidio el mutex
			debug("voy a permitir a los que tenia diferido");
			int n = 0;			//inicializo la variable
			int serv = 0;			//inicializo la variable
			while(n < cant_servidores){
					if(request_diferido[n]){
						
						serv = 2*n;		// seteo el servidor
						sprintf(buffer,"mandando TAG_TE_PERMITO al servidor %d", n);
						debug(buffer);
						request_diferido[n] = FALSE;		//ahora lo saco de los diferidos, ya que les voy a mandar el mensaje a todos
						MPI_Send(NULL, 0, MPI_INT, serv, TAG_TE_PERMITO, COMM_WORLD);	//le mando el mensaje al servidor que estaba diferido y esta esperando
						sprintf(buffer, "le pude mandar al servidor %d",n);
						debug(buffer);	
						}
					
					n++;
					
				}
			
        }
        
        else if (tag == TAG_TERMINE) {
            assert(origen == mi_cliente);
            debug("Mi cliente avisa que terminó");
            
			int n = 0;			//inicializo la variable
			int serv = 0;			//inicializo la variable
			while(n < cant_servidores){
					if( (2*n) != mi_rank ){		// si no soy yo el servidor en cuestion:
						serv = 2*n;		// seteo el servidor
						sprintf(buffer,"mandando TAG_ME_VOY al servidor %d", n);
						debug(buffer);
						request_diferido[n] = FALSE;
						MPI_Send(NULL, 0, MPI_INT, serv, TAG_ME_VOY, COMM_WORLD);	// y le mando un mesnaje al servidor serv que termine
						
						}
				n++;
				}
		me_quiero_ir = TRUE;
		if (cant_terminaron == servidores_comienzo -1) {// si todos los servidores (menos yo) me mandaron mensaje de que terminaron, me voy de ciclo
			listo_para_salir = TRUE;
		}
	
        }


		else if(tag == TAG_ME_VOY){
			int servidor = origen / 2; 		// seteo el servidor 

			sprintf(buffer,"yo soy el servidor %d y se quiere ir %d ", (mi_cliente-1)/2, origen);
								debug(buffer);
		
			cant_terminaron++;

			if (cant_terminaron == servidores_comienzo -1 && me_quiero_ir ){/* si todos los servidores (menos yo)
											 me mandaron mensaje de que terminaron y me quiero ir, me voy de ciclo*/ 
				listo_para_salir = TRUE;	
				}

			}

	
        
    }
    
}

