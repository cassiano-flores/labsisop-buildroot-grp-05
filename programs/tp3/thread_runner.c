#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <linux/unistd.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <sys/syscall.h>
#include <pthread.h>

int politica = -1;
int prioridade = -1;
int num_threads = -1;
int tamanho_buffer = -1;
int index_buffer = 0;
int current_char = 64;
int t = 5;

int semaforo = 0;

pthread_mutex_t mutex;

void *run_thread(void *data)
{
  semaforo++;

  char c = current_char + 1;
  current_char++;

  while (semaforo < num_threads)
  {
    int x = 1;
  }

  sleep(1);

  char *buffer = (char *)data;

  printf("%c - thread started [%d]\n", c, gettid());

  while (index_buffer < tamanho_buffer - 1)
  {
    pthread_mutex_lock(&mutex);

    buffer[index_buffer] = c;
    index_buffer++;

    pthread_mutex_unlock(&mutex);
  }

  printf("%c - thread dies [%d]\n", c, gettid());

  return NULL;
}

void dump_buffer(char *buffer)
{
  int i = 0;
  printf("buffer dump:\n");
  while (buffer[i] != '\0')
  {
    printf("%c", buffer[i]);
    i++;
  }
  printf("\n\n");
}

char *concatena_consecutivos(char *buffer)
{
  int i = 1;
  char current_char;

  if (buffer[0] == '\0')
  {
    return NULL;
  }

  int total = 1;

  current_char = buffer[0];

  while (buffer[i] != '\0')
  {
    if (buffer[i] != current_char)
    {
      total++;
      current_char = buffer[i];
    }
    else
    {
      buffer[i] = 'x';
    }
    i++;
  }

  char *buffer_novo = (char *)malloc((total + 1) * sizeof(char));

  i = 0;
  int j = 0;

  while (buffer[i] != '\0')
  {
    if (buffer[i] != 'x')
    {
      buffer_novo[j] = buffer[i];
      j++;
    }
    i++;
  }

  buffer_novo[total] = '\0';

  return buffer_novo;
}

int main(int argc, char *argv[])
{

  if (argc < 4)
  {
    printf("expected input: thread_runner <número_de_threads> <tamanho_do_buffer_global_em_kilobytes> <politica> <prioridade>\n");
    return 1;
  }

  num_threads = atoi(argv[1]);
  tamanho_buffer = atoi(argv[2]) * 1024;

  char buffer[tamanho_buffer];

  char *sched = argv[3];

  if (strcmp(sched, "SCHED_OTHER") == 0)
    politica = 0;
  if (strcmp(sched, "SCHED_BATCH") == 0)
    politica = 1;
  if (strcmp(sched, "SCHED_IDLE") == 0)
    politica = 2;
  if (strcmp(sched, "SCHED_RR") == 0)
    politica = 3;
  if (strcmp(sched, "SCHED_FIFO") == 0)
    politica = 4;

  prioridade = atoi(argv[4]);

  printf("numero de threads: %d\n", num_threads);
  printf("tamanho do buffer: %d\n", tamanho_buffer);
  printf("politica: %d\n", politica);
  printf("prioridade: %d\n", prioridade);
  printf("sched: %s\n", sched);

  printf("\n");

  printf("main thread [%d]\n", gettid());

  pthread_mutex_init(&mutex, NULL);

  for (int i = 0; i < num_threads; ++i)
  {
    pthread_t thread;
    pthread_attr_t attr;
    struct sched_param param;
    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
    param.sched_priority = prioridade;
    pthread_attr_setschedparam(&attr, &param);
    pthread_create(&thread, &attr, run_thread, (void *)buffer);
  }

  // pthread_join(thread, NULL);

  pthread_mutex_destroy(&mutex);

  sleep(2);

  buffer[tamanho_buffer - 1] = '\0';

  dump_buffer(buffer);

  char *buffer_novo = concatena_consecutivos(buffer);

  dump_buffer(buffer_novo);

  printf("main dies [%d]\n", gettid());

  free(buffer_novo);
  return 0;
}
