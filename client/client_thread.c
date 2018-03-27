/* This `define` tells unistd to define usleep and random.  */
#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "client_thread.h"

// Socket library
//#include <netdb.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

int max_wait_time = 2;
int port_number = -1;
int num_request_per_client = -1;
int num_resources = -1;
int *provisioned_resources = NULL;
int num_clients = -1;

unsigned int socket_fd;

// Variable d'initialisation des threads clients.
unsigned int count = 0;

//bool wasSetup = false;

// Variable du journal.
// Nombre de requête acceptée (ACK reçus en réponse à REQ)
unsigned int count_accepted = 0;

// Nombre de requête en attente (WAIT reçus en réponse à REQ)
unsigned int count_on_wait = 0;

// Nombre de requête refusée (REFUSE reçus en réponse à REQ)
unsigned int count_invalid = 0;

// Nombre de client qui se sont terminés correctement (ACC reçu en réponse à END)
unsigned int count_dispatched = 0;

// Nombre total de requêtes envoyées.
unsigned int request_sent = 0;

int **maxRessources;


// Vous devez modifier cette fonction pour faire l'envoie des requêtes
// Les ressources demandées par la requête doivent être choisies aléatoirement
// (sans dépasser le maximum pour le client). Elles peuvent être positives
// ou négatives.
// Assurez-vous que la dernière requête d'un client libère toute les ressources
// qu'il a jusqu'alors accumulées.

// My client mutexes
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;

void
send_request (int client_id, int request_id, int socket_fd)
{
  // TP2 TODO

  char message[1000];
  sprintf (message, "Client %d is sending its %d request, socket_fd: %d\n", client_id, request_id, socket_fd);
  send(socket_fd, message, strlen(message), 0);
  // TP2 TODO:END

}

void
ct_open_socket (int port_number) {
  socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  if (socket_fd < 0)
    perror("ERROR opening socket");
  if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &(int) {1}, sizeof(int)) < 0) {
    perror("setsockopt()");
    exit(1);
  }

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(2018);
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  //connect client socket to server.
  connect(socket_fd, (struct sockaddr *) &addr, sizeof(addr));

}

void
close_socket(int socket_fd){
  close(socket_fd);
}

// Code executed by setup thread
void* ct_setup(){
  //open a socket

  // memory allocation of a 2-D array of max ressources 
  // where each line is a different thread and maxRessources[i][0] = tid
  // [[tid, max0, max1, ..., maxN]]
  maxRessources = (int**)malloc(num_clients * sizeof(int*));
  for(int i = 0; i < num_clients; i++){
    maxRessources[i] = (int*)malloc((num_resources + 1) * sizeof(int));
  }

  // Filling the with N-1 values;
  for(int i = 0; i < num_clients; i++){
    for(int j = 0; j < num_resources + 1; j++){
      maxRessources[i][j] = -1;
    }
  }
  // for(int i = 0; i < num_clients; i++){
  //   for(int j = 0; j < num_resources + 1; j++){
  //     printf("%d", maxRessources[i][j]);
  //   }
  //   printf("\n");
  // }


  ct_open_socket(port_number);
  send_begRequest(socket_fd);
  close(socket_fd);

  ct_open_socket(port_number);
  send_proRequest(socket_fd);
  close(socket_fd);

  return NULL;
} 


void send_begRequest(int socket_fd){
  //send the BEG command
  char beg[256];
  sprintf(beg, "BEG %d\n", num_resources);  
  send(socket_fd, beg, sizeof(beg), 0);
}

void send_proRequest(int socket_fd){
  //send the PRO command
  char pro[256];
  sprintf(pro, "PRO");
  for (unsigned int i = 0; i < num_resources; i++){
    char tmp[12];
    sprintf(tmp, " %d", provisioned_resources[i]);
    strcat(pro, tmp);
  }
  // printf("%s\n",pro);
  strcat(pro, "\n");
  send(socket_fd, pro, sizeof(pro), 0);
}

void setup_maxRessource(int client_id){
  //print the max ressource table
  pthread_mutex_lock(&mutex);
  // printf("ID R1 R2 R3 R4 R5\n");
  //  for(int i = 0; i < num_clients; i++){
  //   for(int j = 0; j < num_resources + 1; j++){
  //     printf("%d ",maxRessources[i][j]);
  //   }
  //   printf("\n");
  // }
  
  /* UPDATE THE MAX RESSOURCE TABLE {ID M1 M2 M3 ... MN}*/
  for(int i = 0; i < num_clients; i++){
    // Find an empty row for the thread to setup the max ressources
    if(maxRessources[i][0]== -1){
      maxRessources[i][0] = client_id;
      for(int j = 1; j < num_resources + 1; j++){
        maxRessources[i][j] = randint(provisioned_resources[j - 1] + 1, client_id);
      }
      break;
    }
  }

 // print the random values of each thread;
  printf("tid:%d\n", client_id);
  for(int i = 0; i < num_clients; i++){
    for(int j = 0; j < num_resources + 1; j++){
      printf("%d ",maxRessources[i][j]);
    }
    printf("\n");
  }
  pthread_mutex_unlock(&mutex);
  // sprintf (message, "Client %d is sending its %d request, socket_fd: %d\n", client_id, request_id, socket_fd);
  // send(socket_fd, message, strlen(message), 0);;

}
void send_INIrequest(int client_id, int socket_fd){
  char ini[256];
  sprintf(ini, "INI");
  char tmp[12];

  /* Find the proper row to send to the server*/
  for(int i = 0; i < num_clients; i++){
    // Find an empty row for the thread to setup the max ressources
    if(maxRessources[i][0] == client_id){
      sprintf(tmp, " %d", maxRessources[i][0]);
      strcat(ini, tmp);
      for (int j = 1; j < num_resources + 1; j++){
        
        sprintf(tmp, " %d", maxRessources[i][j]);
        strcat(ini, tmp);
      }
      break;
    }
  }
  // printf("ini:%s\n",ini);
  while(true){
  	if(send(socket_fd, ini, sizeof(ini), 0)> -1){
      printf("thread %d send INI\n", client_id);
      break;
    }
  }
}



/* Returns an integer in the range [0, n).
 * with an uniform distribution
 * Uses rand(), and so is affected-by/affects the same seed.
 * SOURCE: https://stackoverflow.com/questions/822323/how-to-generate-a-random-number-in-c
 */
int randint(int n, int id) {
  
  if ((n - 1) == RAND_MAX) {
    return rand();
  } else {
    // Chop off all of the values that would cause skew...
    long end = RAND_MAX / n; // truncate skew
    end *= n;

    // ... and ignore results from rand() that fall above that limit.
    // (Worst case the loop condition should succeed 50% of the time,
    // so we can expect to bail out of this loop pretty quickly.)
    int r;
    while ((r = rand()) >= end);

    return r % n;
  }
}

void *
ct_code (void *param)
{ 
  client_thread *ct = (client_thread *) param;
  int id = ct->id;
  srand(time(NULL)+id);
  setup_maxRessource(ct->id);

  //send the INI request
  ct_open_socket(port_number);
  send_INIrequest(ct->id, socket_fd);
  close(socket_fd);


  // TP2 TODO
  // Connection au server.
  // Vous devez ici faire l'initialisation des petits clients (`INI`).
  //creation de la connection
  //Code pris dans le powerpoint de socket


  // TP2 TODO:END

  // for (unsigned int request_id = 0; request_id < num_request_per_client;
  //     request_id++)
  // {

  //   // TP2 TODO
  //   // Vous devez ici coder, conjointement avec le corps de send request,
  //   // le protocole d'envoi de requête.


  //   send_request (ct->id, request_id, socket_fd);

  //   // TP2 TODO:END

  //   /* Attendre un petit peu (0s-0.1s) pour simuler le calcul.  */
  //   usleep (random () % (100 * 1000));
  //   /* struct timespec delay;
  //    * delay.tv_nsec = random () % (100 * 1000000);
  //    * delay.tv_sec = 0;
  //    * nanosleep (&delay, NULL); */
  // }

  return NULL;
}


//
// Vous devez changer le contenu de cette fonction afin de régler le
// problème de synchronisation de la terminaison.
// Le client doit attendre que le serveur termine le traitement de chacune
// de ses requêtes avant de terminer l'exécution.
//
void
ct_wait_server ()
{
	int endtime = time(NULL) + max_wait_time;
  // TP2 TODO: IMPORTANT code non valide.
  // pas pret par l'algo du banquier

  	while(true){
  		if(time(NULL) >= endtime){
  			break;
  		}
  	}

  // TP2 TODO:END

}


void
ct_init (client_thread * ct)
{
  ct->id = count++;
}

void
ct_create_and_start (client_thread * ct)
{
  pthread_attr_init (&(ct->pt_attr));
  pthread_create (&(ct->pt_tid), &(ct->pt_attr), &ct_code, ct);
  // FIGURE THIS SHIT OUT
  pthread_detach (ct->pt_tid);
}

// Setup thread creation
void ct_create_and_setup(client_thread * ct){
  pthread_attr_init(&(ct->pt_attr));
  pthread_create(&(ct->pt_tid), &(ct->pt_attr), &ct_setup, NULL);
  pthread_join(ct->pt_tid, NULL);
  printf("Setup Completed!\n");
}

//
// Affiche les données recueillies lors de l'exécution du
// serveur.
// La branche else ne doit PAS être modifiée.
//
void
st_print_results (FILE * fd, bool verbose)
{
  if (fd == NULL)
    fd = stdout;
  if (verbose)
  {
    fprintf (fd, "\n---- Résultat du client ----\n");
    fprintf (fd, "Requêtes acceptées: %d\n", count_accepted);
    fprintf (fd, "Requêtes : %d\n", count_on_wait);
    fprintf (fd, "Requêtes invalides: %d\n", count_invalid);
    fprintf (fd, "Clients : %d\n", count_dispatched);
    fprintf (fd, "Requêtes envoyées: %d\n", request_sent);
  }
  else
  {
    fprintf (fd, "%d %d %d %d %d\n", count_accepted, count_on_wait,
        count_invalid, count_dispatched, request_sent);
  }
}
