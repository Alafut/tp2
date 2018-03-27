#include <stdlib.h>

#include "server_thread.h"
#include <pthread.h>

bool accepting_connections = true;

int
main (int argc, char *argv[argc + 1])
{
  if (argc < 3)
  {
    fprintf (stderr, "Usage: %s [port-nb] [nb-threads]\n", argv[0]);
    exit (1);
  }
  // Les arguments initial de la ligne de commande [path, port_number, num_server_threads]
  int port_number = atoi (argv[1]);
  int num_server_threads = atoi (argv[2]);
  server_thread *st = malloc (num_server_threads * sizeof (server_thread));

  // Ouvre un socket
  st_open_socket (port_number);

  // Part les fils d'exécution.
  for (unsigned int i = 0; i < num_server_threads; i++)
  {
    // tableau contenant les server_thread.
    // la struct est : unsigned int id; pthread_t pt_tid; pthread_attr_t pt_attr;
    
    // init de l'id du server_thread
    st[i].id = i;

    // initialise the attribut to the thread created
    pthread_attr_init (&(st[i].pt_attr));

    //creates the thread
    // int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
    pthread_create (&(st[i].pt_tid), &(st[i].pt_attr), &st_code, &(st[i]));
  }

  for (unsigned int i = 0; i < num_server_threads; i++)
    // Waits for all the threads to terminate
    pthread_join (st[i].pt_tid, NULL);

  // Affiche le journal.
  st_print_results (stdout, true);
  FILE *fp = fopen("server.log", "w");
  if (fp == NULL)
  {
    fprintf(stderr, "Could not print log");
    return EXIT_FAILURE;
  }
  st_print_results (fp, false);
  fclose(fp);

  return EXIT_SUCCESS;
}
