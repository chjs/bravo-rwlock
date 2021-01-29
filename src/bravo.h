#ifndef BRAVO_H
#define BRAVO_H

#include <pthread.h>
#include <stdbool.h>
#include <time.h>

typedef struct bravo_rwlock_struct {
  bool rbias;
  unsigned long inhibit_until;
  pthread_rwlock_t underlying;
} bravo_rwlock_t;

void bravo_rwlock_init(bravo_rwlock_t *);
void bravo_rwlock_destroy(bravo_rwlock_t *);
void bravo_read_lock(bravo_rwlock_t *);
int bravo_read_trylock(bravo_rwlock_t *);
void bravo_read_unlock(bravo_rwlock_t *);
void bravo_write_lock(bravo_rwlock_t *);
int bravo_write_trylock(bravo_rwlock_t *);
void bravo_write_unlock(bravo_rwlock_t *);

#endif /* BRAVO_H */
