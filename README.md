# bravo-rwlock
I implemented the scalable reader-writer lock introduced in the paper below.
* [BRAVO—Biased Locking for Reader-Writer Locks](https://www.usenix.org/conference/atc19/presentation/dice) (USENIX ATC '19)

## How to use
The current implementation provides the following functions.
They are similar in usage to pthread reader-writer lock.
```c
void bravo_rwlock_init(bravo_rwlock_t *);
void bravo_rwlock_destroy(bravo_rwlock_t *);
void bravo_read_lock(bravo_rwlock_t *);
int bravo_read_trylock(bravo_rwlock_t *);
void bravo_read_unlock(bravo_rwlock_t *);
void bravo_write_lock(bravo_rwlock_t *);
int bravo_write_trylock(bravo_rwlock_t *);
void bravo_write_unlock(bravo_rwlock_t *);
```

An example code showing how to use is the [test.c](https://github.com/chjs/bravo-rwlock/blob/main/test.c) file.
