#include <stdlib.h>

#include "client_thread.h"

int
main (int argc, char *argv[])
{ 
  // argc = nombre d'arguments
  // argv = tableau des arguments
  if (argc < 5) {
    fprintf (stderr, "Usage: %s <port-nb> <nb-clients> <nb-requests> <resources>...\n",
        argv[0]);
    exit (1);
  }

  port_number = atoi (argv[1]);
  num_clients = atoi (argv[2]);
  num_request_per_client = atoi (argv[3]);
  num_resources = argc - 4;

  //initialisation du available
  provisioned_resources = malloc (num_resources * sizeof (int));
  for (unsigned int i = 0; i < num_resources; i++)
    provisioned_resources[i] = atoi (argv[i + 4]);

  //initialisation des structure du server avec un setup thread
  client_thread *setup_thread = malloc(1*sizeof(client_thread));
  // donne un ID au thread de setup
  ct_init(&(setup_thread[0]));
  ct_create_and_setup(&(setup_thread[0]));


  //initialisation de l'id de chaque client_thread
  client_thread *client_threads = malloc (num_clients * sizeof (client_thread));
  for (unsigned int i = 0; i < num_clients; i++)
    ct_init (&(client_threads[i]));

  // creating et execution du code de chaque client thread
  for (unsigned int i = 0; i < num_clients; i++) {
    ct_create_and_start (&(client_threads[i]));
  }

  //TODO aucune idee quesque sa fait
  ct_wait_server ();

  // Affiche le journal.
  st_print_results (stdout, true);
  FILE *fp = fopen("client.log", "w");
  if (fp == NULL) {
    fprintf(stderr, "Could not print log");
    return EXIT_FAILURE;
  }
  st_print_results (fp, false);
  fclose(fp);

  return EXIT_SUCCESS;
}
