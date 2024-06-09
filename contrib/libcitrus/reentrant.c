
typedef struct rwlock_t {
        SRWLOCK lock;
        LONG rcnt;
        LONG wcnt;
} rwlock_t;

#define rwlock_init(rwl)
        do {
                (rwl)->rcnt = (rwl)->wcnt = 0;
                InitializeSRWLock(&(rwl)->lock);
        } while (0)

#define rwlock_destroy(rwl)
        do {
                assert((rwl)->rcnt == 0 && (rwl)->wcnt == 0);
                (rwl)->rcnt = (rwl)->wcnt = 0;
        } while (0)

#define rwlock_rdlock(rwl)
        do {
                if (0) printf("Thr %i: at %i:   RDLOCK %p   %s (%i, %i)\n", GetCurrentThreadId(), __LINE__, rwl, __FUNCTION__, (rwl)->rcnt, (rwl)->wcnt);
                assert((rwl)->rcnt >= 0 && (rwl)->wcnt >= 0);
                AcquireSRWLockShared(&(rwl)->lock);
                InterlockedIncrement(&(rwl)->rcnt);
        } while (0)

#define rwlock_wrlock(rwl)
        do {
                if (0) printf("Thr %i: at %i:   WRLOCK %p   %s (%i, %i)\n", GetCurrentThreadId(), __LINE__, rwl, __FUNCTION__, (rwl)->rcnt, (rwl)->wcnt);
                assert((rwl)->rcnt >= 0 && (rwl)->wcnt >= 0);
                AcquireSRWLockExclusive(&(rwl)->lock);
                InterlockedIncrement(&(rwl)->wcnt);
        } while (0)

#define rwlock_rdunlock(rwl)
        do {
                if (0) printf("Thr %i: at %i: RDUNLOCK %p   %s (%i, %i)\n", GetCurrentThreadId(), __LINE__, rwl, __FUNCTION__, (rwl)->rcnt, (rwl)->wcnt);
                assert((rwl)->rcnt > 0 && (rwl)->wcnt >= 0);
                InterlockedDecrement(&(rwl)->rcnt);
                ReleaseSRWLockShared(&(rwl)->lock);
        } while (0)

#define rwlock_wrunlock(rwl)
        do {
                if (0) printf("Thr %i: at %i: RWUNLOCK %p   %s (%i, %i)\n", GetCurrentThreadId(), __LINE__, rwl, __FUNCTION__, (rwl)->rcnt, (rwl)->wcnt);
                assert((rwl)->rcnt >= 0 && (rwl)->wcnt > 0);
                InterlockedDecrement(&(rwl)->wcnt);
                ReleaseSRWLockExclusive(&(rwl)->lock);
        } while (0)

#define rwlock_rdlock_d(rwl)
        do {
                if (1) printf("Thr %i: at %i:   RDLOCK %p   %s (%i, %i)\n", GetCurrentThreadId(), __LINE__, rwl, __FUNCTION__, (rwl)->rcnt, (rwl)->wcnt);
                assert((rwl)->rcnt >= 0 && (rwl)->wcnt >= 0);
                AcquireSRWLockShared(&(rwl)->lock);
                InterlockedIncrement(&(rwl)->rcnt);
        } while (0)

#define rwlock_wrlock_d(rwl)
        do {
                if (1) printf("Thr %i: at %i:   WRLOCK %p   %s (%i, %i)\n", GetCurrentThreadId(), __LINE__, rwl, __FUNCTION__, (rwl)->rcnt, (rwl)->wcnt);
                assert((rwl)->rcnt >= 0 && (rwl)->wcnt >= 0);
                AcquireSRWLockExclusive(&(rwl)->lock);
                InterlockedIncrement(&(rwl)->wcnt);
        } while (0)

#define rwlock_rdunlock_d(rwl)
        do {
                if (1) printf("Thr %i: at %i: RDUNLOCK %p   %s (%i, %i)\n", GetCurrentThreadId(), __LINE__, rwl, __FUNCTION__, (rwl)->rcnt, (rwl)->wcnt);
                assert((rwl)->rcnt > 0 && (rwl)->wcnt >= 0);
                InterlockedDecrement(&(rwl)->rcnt);
                ReleaseSRWLockShared(&(rwl)->lock);
        } while (0)

#define rwlock_wrunlock_d(rwl)
        do {
                if (1) printf("Thr %i: at %i: RWUNLOCK %p   %s (%i, %i)\n", GetCurrentThreadId(), __LINE__, rwl, __FUNCTION__, (rwl)->rcnt, (rwl)->wcnt);
                assert((rwl)->rcnt >= 0 && (rwl)->wcnt > 0);
                InterlockedDecrement(&(rwl)->wcnt);
                ReleaseSRWLockExclusive(&(rwl)->lock);
        } while (0)

/*end*/



