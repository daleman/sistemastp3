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
	char buffer[120];
	int max;
    int i = 0;
    while(i < cant_servidores){
	request_diferido[i] = FALSE;
	i++;	
	}

	int el_mayor = 0;		// el mayor numero de secuencia enviado o recibido por mi servidor
	
	int faltantes = 1;		// la cantidad de replies que faltan de todos los servidores para obtener el mutex, lo inicializo con un numero 							//positivo asi no me da el mutex de inmediato sin pensarlo 

	int buf_entrante;

    while( ! listo_para_salir ) {
        
        MPI_Recv(&buf_entrante, 1, MPI_INT, ANY_SOURCE, ANY_TAG, COMM_WORLD, &status);
        origen = status.MPI_SOURCE;
        tag = status.MPI_TAG;
        
        if (tag == TAG_PEDIDO) {
            assert(origen == mi_cliente);
	   // sprintf("Mi cliente %d solicita acceso exclusivo", mi_cliente);
            debug("Mi cliente solicita acceso exclusivo");
            assert( hay_pedido_local == FALSE);
            hay_pedido_local = TRUE;						// yo quiero el mutex
            el_mayor++;		//incremento el mayor
			faltantes = cant_servidores - 1;	//faltan todos los servidores menos yo, empiecen a mandar mensajitos...
			if (faltantes == 0) {
			debug("solo hay un servidor");
			MPI_Send(NULL, 0, MPI_INT, mi_cliente, TAG_OTORGADO, COMM_WORLD);
		
		} 	    
		int serv = 0;
 	    int j = 0;
		while(j < cant_servidores){
			serv = 2*j;
				if (serv != mi_cliente-1){
					assert(serv != mi_rank);
					MPI_Send(&el_mayor, 1, MPI_INT, serv, TAG_PEDIR, COMM_WORLD);
				}
			j++;					
		}
	}
                  

		else if (tag == TAG_TE_PERMITO) {
	            assert(origen % 2 == 0 );
        	    debug("un servidor me da permiso para que obtenga el mutex");
            	    assert(hay_pedido_local == TRUE);

			debug("hay un servidor menos que falta");
			faltantes--;
			debug("Me fijo si puedo darle el mutex...");
				if (faltantes == 0) {	// si el ultimo servidor que faltaba me da permiso, le doy el mutex a mi cliente
					debug("Dándole permiso (me dieron todos los permisos! )");
		        		MPI_Send(NULL, 0, MPI_INT, mi_cliente, TAG_OTORGADO, COMM_WORLD);            
					debug("le mande el mensaje a mi cliente para que obtenga el mutex ");				
					}
        }
        

	    else if (tag == TAG_PEDIR) {
            	assert(origen != mi_rank);
		
		       	debug("Me llego un pedido de otro servidor ");
           		if(!hay_pedido_local){
				assert(hay_pedido_local == FALSE);
				debug("no tengo pedidos locales, le permito al servidor que me pidio el mutex");		        				MPI_Send(NULL, 0, MPI_INT, origen, TAG_TE_PERMITO, COMM_WORLD);
				}
			else if( el_mayor < buf_entrante || ( el_mayor == buf_entrante && mi_rank < origen ) ){
				int serv = origen / 2;					
				debug("lo difiero");					
				request_diferido[serv] = TRUE;

			}else{
			debug("lo permito");
			MPI_Send(NULL, 0, MPI_INT, origen, TAG_TE_PERMITO, COMM_WORLD);
			max = el_mayor;		
			}

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
	    int n = cant_servidores -1;
	    int serv = 0;
		while(n > 0){
				if(request_diferido[n]){
					serv = 2*n;
					debug("mandando TAG_TE_PERMITO");
					MPI_Send(NULL, 0, MPI_INT, serv, TAG_TE_PERMITO, COMM_WORLD);
					request_diferido[n] = FALSE;
					}
			n--;
			}
        }
        
        else if (tag == TAG_TERMINE) {
            assert(origen == mi_cliente);
            debug("Mi cliente avisa que terminó");
            listo_para_salir = TRUE;
        }
        
    }
    
}

