#include "srv.h"

/*
 *  Ejemplo de servidor que tiene el "sí fácil" para con su
 *  cliente y no se lleva bien con los demás servidores.
 *
 */

void servidor(int mi_cliente)
{
    MPI_Status status; int origen, tag;
    int hay_pedido_local = FALSE;
    int listo_para_salir = FALSE;
	  	
    int cant_servidores = cant_ranks / 2;
 
    int request_diferido[cant_servidores];	// un "booleano" para decir que servidores estan diferidos, es decir
						// que pidieron el mutex para sus clientes y todavia no lo obtuvieron
	char buffer[140];
	int max;
    int i = 0;
    while(i < cant_servidores){
	request_diferido[i] = FALSE;
	i++;	
	}

	int el_mayor = 0;		// el mayor numero de secuencia enviado o recibido por mi servidor
	
	int faltantes = 1;		// la cantidad de replies que faltan de todos los servidores para obtener el mutex, lo inicializo con un numero 							//positivo asi no me da el mutex de inmediato sin pensarlo 
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
            hay_pedido_local = TRUE;						// yo quiero el mutex
            el_mayor++;		//incremento el mayor
			mi_numero = el_mayor;
			faltantes = cant_servidores - 1;	//faltan todos los servidores menos yo, empiecen a mandar mensajitos...
				if (faltantes == 0) {
				debug("solo hay un servidor");
				MPI_Send(NULL, 0, MPI_INT, mi_cliente, TAG_OTORGADO, COMM_WORLD);
				} 	    
			int serv = 0;
	 	    int j = 0;
			while(j < cant_servidores){
				serv = 2*j;
					if (serv != mi_rank){
						assert(serv != mi_rank);
						sprintf(buffer, "Yo servidor %d mando un pedido al servidor %d. HpedLoc: %d, elMay: %d",mi_rank/2, serv/2,hay_pedido_local, el_mayor);
		       			debug(buffer);
						MPI_Send(&mi_numero, 1, MPI_INT, serv, TAG_PEDIR, COMM_WORLD);
					}
				j++;					
			}
	}
                  

		else if (tag == TAG_TE_PERMITO) {
	            assert(origen % 2 == 0 );
        	    sprintf(buffer, "el servidor %d me da permiso para que obtenga el mutex", origen/2);
				debug(buffer);
            	    assert(hay_pedido_local == TRUE);

			//debug("hay un servidor menos que falta");
			faltantes--;
			sprintf(buffer,"Me fijo si puedo darle el mutex... al servidor %d le faltan %d permisos", mi_rank/2, faltantes);
			debug(buffer);
				if (faltantes == 0) {	// si el ultimo servidor que faltaba me da permiso, le doy el mutex a mi cliente
						sprintf(buffer,"(me dieron todos los permisos!) le doy el mutex al cliente %d ", (mi_cliente-1)/2);
						debug(buffer);
		        		MPI_Send(NULL, 0, MPI_INT, mi_cliente, TAG_OTORGADO, COMM_WORLD);            
					//debug("le mande el mensaje a mi cliente para que obtenga el mutex ");				
					}
        }
        

	    else if (tag == TAG_PEDIR) {
            	assert(origen != mi_rank);
				sprintf(buffer, "Me llego un pedido del servidor %d. HpedLoc: %d, elMay: %d, bufE: %d", origen/2,hay_pedido_local, el_mayor,buf_entrante);
		       	debug(buffer);
           		
				if(!hay_pedido_local){
				assert(hay_pedido_local == FALSE);
				sprintf(buffer,"Yo servidor %d no tengo pedidos locales, le permito al servidor %d que me pidio el mutex \n", mi_rank/2, origen/2);				
				debug(buffer);
		  		MPI_Send(NULL, 0, MPI_INT, origen, TAG_TE_PERMITO, COMM_WORLD);
				}
			else if( mi_numero < buf_entrante || ( mi_numero == buf_entrante && mi_rank < origen ) ){
				int serv = origen / 2;					
				sprintf(buffer, "difiero al servidor %d", origen/2);				
				debug(buffer);					
				request_diferido[serv] = TRUE;

			}else{
			sprintf(buffer, "el pedido del servidor %d tiene mas prioridad", origen/2);
			debug(buffer);
			MPI_Send(NULL, 0, MPI_INT, origen, TAG_TE_PERMITO, COMM_WORLD);
					
			}
			max = el_mayor;
			if (buf_entrante  > el_mayor){		//elijo el maximo entre el_mayor y el numero que recibi
					max = buf_entrante ;		
				}			
			el_mayor = max;
			
        }


	else if (tag == TAG_LIBERO) {
            assert(origen == mi_cliente);
            debug("Mi cliente libera su acceso exclusivo");
            assert(hay_pedido_local == TRUE);
            hay_pedido_local = FALSE;
			debug("voy a permitir a los que tenia diferido");
			int n = 0;
			int serv = 0;
			while(n < cant_servidores){
					if(request_diferido[n]){
						serv = 2*n;
						sprintf(buffer,"mandando TAG_TE_PERMITO al servidor %d", n);
						debug(buffer);
						request_diferido[n] = FALSE;
						MPI_Send(NULL, 0, MPI_INT, serv, TAG_TE_PERMITO, COMM_WORLD);
						
						}
				n++;
				}
        }
        
        else if (tag == TAG_TERMINE) {
            assert(origen == mi_cliente);
            debug("Mi cliente avisa que terminó");
            listo_para_salir = TRUE;
        }
        
    }
    
}

