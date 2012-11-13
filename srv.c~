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

    int i = 0;
    while(i < cant_servidores){
	request_diferido[i] = FALSE;
	i++;	
	}

	int el_mayor = 0;		// el mayor numero de secuencia enviado o recibido por mi servidor
	int mi_numero = {0};			// el numero de secuencia de mi servidor
	int buf_numero = {0};			//el buffer que se pasa por mensaje
	int faltantes = 1;		// la cantidad de replies que faltan de todos los servidores para obtener el mutex, lo inicializo con un numero 							//positivo asi no me da el mutex de inmediato sin pensarlo 

    while( ! listo_para_salir ) {
        
        MPI_Recv(NULL, 0, MPI_INT, ANY_SOURCE, ANY_TAG, COMM_WORLD, &status);
        origen = status.MPI_SOURCE;
        tag = status.MPI_TAG;
        
        if (tag == TAG_PEDIDO) {
            assert(origen == mi_cliente);
            debug("Mi cliente solicita acceso exclusivo");
            assert( hay_pedido_local == FALSE);
            hay_pedido_local = TRUE;				// yo quiero el mutex
			mi_numero[0] = el_mayor + 1;				// actualizo mi_numero
			buf_numero[0] = (void*) mi_numero[0];
			faltantes = cant_servidores - 1;		//faltan todos los servidores menos yo, empiecen a mandar mensajitos...
			int serv = 0;
			int j = 0;
				while(j < cant_servidores){
				serv = 2*j;
						if (serv != mi_rank){
						MPI_Send(buf_numero, 1, MPI_INT, serv, TAG_PEDIR, COMM_WORLD);
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
				if (faltantes == 0) {					// si el ultimo servidor que faltaba me da permiso, le doy el mutex a mi cliente
						debug("Dándole permiso (me dieron todos los permisos! )");
		        		MPI_Send(NULL, 0, MPI_INT, mi_cliente, TAG_OTORGADO, COMM_WORLD);            
						debug("le mande el mensaje a mi cliente para que obtenga el mutex y escribo pedido local en false");					
						hay_pedido_local = FALSE;
				}
        }
        

		else if (tag == TAG_PEDIR) {
            assert(origen != mi_rank);
            debug("Me llego un pedido de otro servidor ");
           
				if(!hay_pedido_local){
					assert(hay_pedido_local == FALSE);
					debug("no tengo pedidos locales, le permito al servidor que me pidio el mutex");		        	
					MPI_Send(NULL, 0, MPI_INT, origen, TAG_TE_PERMITO, COMM_WORLD);
				}
				else if( buf_numero[0] > mi_numero[0] || ( buf_numero[0] == mi_numero[0] && origen > mi_rank) ){
					int serv = origen / 2;					
					debug("lo difiero");					
					request_diferido[serv] = TRUE;

				}
				else{
				debug("lo permito");
				MPI_Send(NULL, 0, MPI_INT, origen, TAG_TE_PERMITO, COMM_WORLD);
				}
			
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
					debug("mandando TAG_TE_PERMITO");
					MPI_Send(NULL, 0, MPI_INT, serv, TAG_TE_PERMITO, COMM_WORLD);
					request_diferido[n] = FALSE;
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

