#include "bravo.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BILLION 1000000000L
#define N 9
#define NR_ENTRIES 4096

volatile bravo_rwlock_t *visible_readers[NR_ENTRIES] = {
    0,
};

static inline int mix32(unsigned long z) {
  z = (z ^ (z >> 33)) * 0xff51afd7ed558ccdL;
  z = (z ^ (z >> 33)) * 0xc4ceb9fe1a85ec53L;
  return abs((int)(z >> 32));
}

static inline int bravo_hash(bravo_rwlock_t *l) {
  unsigned long a, b;
  a = (unsigned long)pthread_self();
  b = (unsigned long)l;
  return mix32(a + b) % NR_ENTRIES;
}

void bravo_rwlock_init(bravo_rwlock_t *l) {
  int s;

  l->rbias = true;
  l->inhibit_until = 0;

  s = pthread_rwlock_init(&l->underlying, NULL);
  if (__glibc_unlikely(s != 0)) {
    perror("pthread_rwlock_init");
  }
}

void bravo_rwlock_destroy(bravo_rwlock_t *l) {
  int s;
  bravo_write_lock(l);
  s = pthread_rwlock_destroy(&l->underlying);
  if (__glibc_unlikely(s != 0)) {
    perror("pthread_rwlock_destroy");
  }
}

void bravo_read_lock(bravo_rwlock_t *l) {
  struct timespec now;
  unsigned long now_time;
  int slot, s;

  if (l->rbias) {
    slot = bravo_hash(l);

    if (__sync_bool_compare_and_swap(&visible_readers[slot], NULL, l)) {
      if (l->rbias) {
        return;
      }

      visible_readers[slot] = NULL;
    }
  }

  /* slow-path */
  s = pthread_rwlock_rdlock(&l->underlying);
  if (__glibc_unlikely(s != 0)) {
    perror("pthread_rwlock_rdlock");
  }

  s = clock_gettime(CLOCK_MONOTONIC, &now);
  if (__glibc_unlikely(s != 0)) {
    perror("clock_gettime");
  }

  now_time = (now.tv_sec * BILLION) + now.tv_nsec;

  if (l->rbias == false && now_time >= l->inhibit_until) {
    l->rbias = true;
  }
}

int bravo_read_trylock(bravo_rwlock_t *l) {
  struct timespec now;
  unsigned long now_time;
  int slot, s;

  if (l->rbias) {
    slot = bravo_hash(l);

    if (__sync_bool_compare_and_swap(&visible_readers[slot], NULL, l)) {
      if (l->rbias) {
        return 0;
      }

      visible_readers[slot] = NULL;
    }
  }

  /* slow-path */
  s = pthread_rwlock_tryrdlock(&l->underlying);
  if (__glibc_unlikely(s != 0)) {
    return s;
  }

  s = clock_gettime(CLOCK_MONOTONIC, &now);
  if (__glibc_unlikely(s != 0)) {
    perror("clock_gettime");
  }

  now_time = (now.tv_sec * BILLION) + now.tv_nsec;

  if (l->rbias == false && now_time >= l->inhibit_until) {
    l->rbias = true;
  }
  return 0;
}

void bravo_read_unlock(bravo_rwlock_t *l) {
  int slot, s;

  slot = bravo_hash(l);

  if (visible_readers[slot] != NULL) {
    visible_readers[slot] = NULL;
  } else {
    s = pthread_rwlock_unlock(&l->underlying);
    if (__glibc_unlikely(s != 0)) {
      perror("pthread_rwlock_unlock");
    }
  }
}

static inline void revocate(bravo_rwlock_t *l) {
  struct timespec start, now;
  unsigned long start_time, now_time;
  int s, i;

  l->rbias = false;

  s = clock_gettime(CLOCK_MONOTONIC, &start);
  if (__glibc_unlikely(s != 0)) {
    perror("clock_gettime");
  }

  for (i = 0; i < NR_ENTRIES; i++) {
    while (visible_readers[i] == l) {
      usleep(1);
    }
  }

  s = clock_gettime(CLOCK_MONOTONIC, &now);
  if (__glibc_unlikely(s != 0)) {
    perror("clock_gettime");
  }

  start_time = (start.tv_sec * BILLION) + start.tv_nsec;
  now_time = (now.tv_sec * BILLION) + now.tv_nsec;

  l->inhibit_until = now_time + ((now_time - start_time) * N);
}

void bravo_write_lock(bravo_rwlock_t *l) {
  int s;

  s = pthread_rwlock_wrlock(&l->underlying);
  if (__glibc_unlikely(s != 0)) {
    perror("pthread_rwlock_wrlock");
  }

  if (l->rbias) {
    revocate(l);
  }
}

int bravo_write_trylock(bravo_rwlock_t *l) {
  int s;

  s = pthread_rwlock_trywrlock(&l->underlying);
  if (__glibc_unlikely(s != 0)) {
    return s;
  }

  if (l->rbias) {
    revocate(l);
  }
  return 0;
}

void bravo_write_unlock(bravo_rwlock_t *l) {
  int s;

  s = pthread_rwlock_unlock(&l->underlying);
  if (__glibc_unlikely(s != 0)) {
    perror("pthread_rwlock_unlock");
  }
}
