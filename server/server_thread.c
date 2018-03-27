#define _XOPEN_SOURCE 700   /* So as to allow use of `fdopen` and `getline`.  */

#include "server_thread.h"

#include <netinet/in.h>
#include <netdb.h>

#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <signal.h>

#include <time.h>

enum { NUL = '\0' };

enum {
  /* Configuration constants.  */
  max_wait_time = 30,
  server_backlog_size = 5
};

unsigned int server_socket_fd;

// Nombre de client enregistré.
int nb_registered_clients;

// Variable du journal.
// Nombre de requêtes acceptées immédiatement (ACK envoyé en réponse à REQ).
unsigned int count_accepted = 0;

// Nombre de requêtes acceptées après un délai (ACK après REQ, mais retardé).
unsigned int count_wait = 0;

// Nombre de requête erronées (ERR envoyé en réponse à REQ).
unsigned int count_invalid = 0;

// Nombre de clients qui se sont terminés correctement
// (ACK envoyé en réponse à CLO).
unsigned int count_dispatched = 0;

// Nombre total de requête (REQ) traités.
unsigned int request_processed = 0;

// Nombre de clients ayant envoyé le message CLO.
unsigned int clients_ended = 0;

//Initialise le nombre de clients
int nb_registered_clients = 0;

// My mutexes
pthread_mutex_t mutexMaxRessource = PTHREAD_MUTEX_INITIALIZER;

// TODO: Ajouter vos structures de données partagées, ici.
int *available;
int nb_ressources;
int **maxRessources;
int old = 0;
bool begSetup = 0;
bool proSetup = 0;

static void sigint_handler(int signum) {
  // Code terminaison.
  accepting_connections = 0;
}

void
st_init ()
{
  // Handle interrupt a.k.a crt-c from the terminal
  signal(SIGINT, &sigint_handler);
  
  //increment the number of clients

  // add a registered client row 

  // first ini cmd edge case
  old = nb_registered_clients;
  nb_registered_clients += 1;
  printf("nb_registered_clients = %d\n", nb_registered_clients);

  //realloc de la memoire du maxRessources
  maxRessources = realloc(maxRessources, sizeof(int*) * nb_registered_clients);
  for(int i = old; i < nb_registered_clients; i++){
    maxRessources[i] = malloc(sizeof(int) * nb_ressources +1);
  }
  // TODO
  

  // Attend la connection d'un client et initialise les structures pour
  // l'algorithme du banquier.

  // END TODO
}

void
st_process_requests (server_thread * st, int socket_fd) {
    // TODO: Remplacer le contenu de cette fonction
    FILE *socket_r = fdopen(socket_fd, "r");
    FILE *socket_w = fdopen(socket_fd, "w");

    // buffer for the command
    char cmd[4] = {NUL, NUL, NUL, NUL};
    // read the command
    fread(cmd, 3, 1, socket_r);

    printf("cmd is:%s\n", cmd);

    //initialisation du nombre de ressources
    if (strcmp("BEG", cmd) == 0) {
        if (begSetup != 0) {
            printf("ERROR, BEG CMD was already done!\n");
            return;
        }
        printf("your are in BEG\n");
        // buffer for arguments
        char *args = NULL;
        size_t args_len = 0;
        // dump the space in from of the first command
        char dump[1] = {NUL};
        fread(dump, 1, 1, socket_r);
        getline(&args, &args_len, socket_r);
        printf("args is:%s\n", args);

        //set up number of ressources
        nb_ressources = atoi(args);
        begSetup = 1;

    }

        // Initialisation des provision de chaque ressources
    else if (strcmp("PRO", cmd) == 0) {
        if (proSetup != 0) {
            printf("ERROR, PRO CMD was already done!\n");
            return;
        }
        printf("your are in PRO\n");
        char *args = NULL;
        size_t args_len = 0;
        // dump the space in from of the first command
        char dump[1] = {NUL};
        fread(dump, 1, 1, socket_r);
        getline(&args, &args_len, socket_r);
        printf("args is:%s\n", args);
        printf("nb_ressources:%d\n", nb_ressources);

        //tokenize the args pour les provisions
        char *token[nb_ressources];
        int nb_token = 0;
        token[0] = strtok(args, " ");
        while (token[nb_token] != NULL) {
            nb_token++;
            token[nb_token] = strtok(NULL, " ");
        }
        //iteration pour les ajouter les ressoure dans *available
        available = malloc(nb_ressources * sizeof(int));
        for (unsigned int i = 0; i < nb_ressources; i++) {
            available[i] = atoi(token[i]);
        }

        //allocation de la memoire du tableau des registered client;
        maxRessources = malloc(sizeof(int *) * nb_registered_clients);
        for (int i = old; i < nb_registered_clients; i++) {
            maxRessources[i] = malloc(sizeof(int) * nb_ressources + 1);
        }
        proSetup = 1;
    } else if (strcmp("INI", cmd) == 0) {
        printf("You are in INI\n");
        char *args = NULL;
        size_t args_len = 0;
        // dump the space in from of the first command
        char dump[1] = {NUL};
        fread(dump, 1, 1, socket_r);
        getline(&args, &args_len, socket_r);
        printf("args is:%s\n", args);


        //tokennize the args pour les max values
        char *token[nb_ressources + 1];
        int nb_token = 0;
        token[0] = strtok(args, " ");
        while (token[nb_token] != NULL) {
            nb_token++;
            token[nb_token] = strtok(NULL, " ");
        }
        
        //LOCK
        pthread_mutex_lock(&mutexMaxRessource);
        
        st_init();

        for (int i = old; i < nb_registered_clients; i++) {
            for (int j = 0; j < nb_ressources + 1; j++) {
                maxRessources[i][j] = atoi(token[j]);
            }
            printf("maxRessource table\n");
            for (int i = 0; i < nb_registered_clients; i++) {
                for (int j = 0; j < nb_ressources + 1; j++) {
                    printf("%d ", maxRessources[i][j]);
                }
                printf("\n");
            }
        
        //UNLOCK
        pthread_mutex_unlock(&mutexMaxRessource);
        }
    }

        // while (true)
        // {
        // char cmd[4] = {NUL, NUL, NUL, NUL};
        // if (!fread (cmd, 3, 1, socket_r)) {
        //   break;
        // }
        //   // char *args = NULL; size_t args_len = 0;
        //   // ssize_t cnt = getline (&args, &args_len, socket_r);
        //   if (!args || cnt < 1 || args[cnt - 1] != '\n')
        //   {
        //     printf ("Thread %d received incomplete cmd=%s!\n", st->id, cmd);
        //     break;
        //   }

        //   printf ("Thread %d received the command: %s%s", st->id, cmd, args);

        //   fprintf (socket_w, "ERR Unknown command\n");
        //   fflush(socket_w);
        //   free (args);
        // }

        fclose(socket_r);
        fclose(socket_w);
        // TODO end
    }


    int st_wait() {
        struct sockaddr_in thread_addr;
        socklen_t socket_len = sizeof(thread_addr);
        int thread_socket_fd = -1;
        int end_time = time(NULL) + max_wait_time;

        while (thread_socket_fd < 0 && accepting_connections) {
            thread_socket_fd = accept(server_socket_fd,
                                      (struct sockaddr *) &thread_addr,
                                      &socket_len);
            if (time(NULL) >= end_time) {
                break;
            }
        }
        return thread_socket_fd;
    }

    void *
    st_code(void *param) {
        server_thread *st = (server_thread *) param;

        int thread_socket_fd = -1;

        // Boucle de traitement des requêtes.
        while (accepting_connections) {
            // Wait for a I/O socket.
            thread_socket_fd = st_wait();
            if (thread_socket_fd < 0) {
                fprintf(stderr, "Time out on thread %d.\n", st->id);
                continue;
            }

            if (thread_socket_fd > 0) {
                st_process_requests(st, thread_socket_fd);
                close(thread_socket_fd);
            }
        }
        return NULL;
    }


//
// Ouvre un socket pour le serveur.
//
    void
    st_open_socket(int port_number) {
        server_socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (server_socket_fd < 0)
            perror("ERROR opening socket");

        if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEPORT, &(int) {1}, sizeof(int)) < 0) {
            perror("setsockopt()");
            exit(1);
        }

        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(port_number);

        if (bind
                    (server_socket_fd, (struct sockaddr *) &serv_addr,
                     sizeof(serv_addr)) < 0)
            perror("ERROR on binding");

        listen(server_socket_fd, server_backlog_size);
    }


//
// Affiche les données recueillies lors de l'exécution du
// serveur.
// La branche else ne doit PAS être modifiée.
//
    void
    st_print_results(FILE *fd, bool verbose) {
        if (fd == NULL) fd = stdout;
        if (verbose) {
            fprintf(fd, "\n---- Résultat du serveur ----\n");
            fprintf(fd, "Requêtes acceptées: %d\n", count_accepted);
            fprintf(fd, "Requêtes : %d\n", count_wait);
            fprintf(fd, "Requêtes invalides: %d\n", count_invalid);
            fprintf(fd, "Clients : %d\n", count_dispatched);
            fprintf(fd, "Requêtes traitées: %d\n", request_processed);
        } else {
            fprintf(fd, "%d %d %d %d %d\n", count_accepted, count_wait,
                    count_invalid, count_dispatched, request_processed);
        }
    }

