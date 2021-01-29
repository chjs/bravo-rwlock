#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#ifdef BRAVO
#include "bravo.h"
#endif

#define NR_THREADS 16
#define LOOP 2000000

#ifdef BRAVO
bravo_rwlock_t *rwlock;
#else
pthread_rwlock_t *rwlock;
#endif
char data[4096];

void *reader(void *args) {
  char buffer[4096];
  int i;

  for (i = 0; i < LOOP; i++) {
#ifdef BRAVO
    bravo_read_lock(rwlock);
#else
    pthread_rwlock_rdlock(rwlock);
#endif

    memcpy(buffer, data, 4096);

#ifdef BRAVO
    bravo_read_unlock(rwlock);
#else
    pthread_rwlock_unlock(rwlock);
#endif
  }
  return args;
}

int main(void) {
  pthread_t tid[NR_THREADS];
  int i, s;

  memset(data, 'A', 4096);

#ifdef BRAVO
  rwlock = (bravo_rwlock_t *)malloc(sizeof(bravo_rwlock_t));
#else
  rwlock = (pthread_rwlock_t *)malloc(sizeof(pthread_rwlock_t));
#endif

  if (rwlock == NULL) {
    perror("malloc");
  }

#ifdef BRAVO
  bravo_rwlock_init(rwlock);
#else
  pthread_rwlock_init(rwlock, NULL);
#endif

  for (i = 0; i < NR_THREADS; i++) {
    s = pthread_create(&tid[i], NULL, reader, NULL);
    if (s != 0) {
      perror("pthread_create");
    }
  }

  for (i = 0; i < NR_THREADS; i++) {
    s = pthread_join(tid[i], NULL);
    if (s != 0) {
      perror("pthread_join");
    }
  }

#ifdef BRAVO
  bravo_rwlock_destroy(rwlock);
#else
  pthread_rwlock_destroy(rwlock);
#endif

  return 0;
}
