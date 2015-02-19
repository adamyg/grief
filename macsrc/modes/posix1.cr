/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: posix1.cr,v 1.3 2014/10/22 02:34:35 ayoung Exp $
 * posix1 markup.
 *
 *
 */

#include "../grief.h"

//  POSIX.1b (formerly known as POSIX.4)
//      IEEE Std 1003.1b-1993 describing real-time facilities for portable operating systems,
//      ratified by ISO in 1996 (ISO/IEC 9945-1:1996).
//
//  POSIX.1c IEEE Std 1003.1c-1995 describing the POSIX threads interfaces.
//
//  POSIX.1d IEEE Std 1003.1c-1999 describing additional real-time extensions.
//
//  POSIX.1g IEEE Std 1003.1g-2000 describing networking APIs (including sockets).
//
//  POSIX.1j IEEE Std 1003.1j-2000 describing advanced real-time extensions.
//
//  POSIX.1-1996
//      A 1996 revision of POSIX.1 which incorporated POSIX.1b and POSIX.1c.
//

static list posix_constant = {
    // <pthreads.h>
    "PTHREAD_BARRIER_SERIAL_THREAD",
    "PTHREAD_CANCEL_ASYNCHRONOUS",
    "PTHREAD_CANCEL_ENABLE",
    "PTHREAD_CANCEL_DEFERRED",
    "PTHREAD_CANCEL_DISABLE",
    "PTHREAD_CANCELED",
    "PTHREAD_CREATE_DETACHED",
    "PTHREAD_CREATE_JOINABLE",
    "PTHREAD_EXPLICIT_SCHED",
    "PTHREAD_INHERIT_SCHED",
    "PTHREAD_MUTEX_DEFAULT",
    "PTHREAD_MUTEX_ERRORCHECK",
    "PTHREAD_MUTEX_NORMAL",
    "PTHREAD_MUTEX_RECURSIVE",
    "PTHREAD_MUTEX_ROBUST",
    "PTHREAD_MUTEX_STALLED",
    "PTHREAD_ONCE_INIT",
    "PTHREAD_PRIO_INHERIT",
    "PTHREAD_PRIO_NONE",
    "PTHREAD_PRIO_PROTECT",
    "PTHREAD_PROCESS_SHARED",
    "PTHREAD_PROCESS_PRIVATE",
    "PTHREAD_SCOPE_PROCESS",
    "PTHREAD_SCOPE_SYSTEM",

    // <trace.h>
    "POSIX_TRACE_ALL_EVENTS",
    "POSIX_TRACE_APPEND",
    "POSIX_TRACE_CLOSE_FOR_CHILD",
    "POSIX_TRACE_FILTER",
    "POSIX_TRACE_FLUSH",
    "POSIX_TRACE_FLUSH_START",
    "POSIX_TRACE_FLUSH_STOP",
    "POSIX_TRACE_FLUSHING",
    "POSIX_TRACE_FULL",
    "POSIX_TRACE_LOOP",
    "POSIX_TRACE_NO_OVERRUN",
    "POSIX_TRACE_NOT_FLUSHING",
    "POSIX_TRACE_NOT_FULL",
    "POSIX_TRACE_INHERITED",
    "POSIX_TRACE_NOT_TRUNCATED",
    "POSIX_TRACE_OVERFLOW",
    "POSIX_TRACE_OVERRUN",
    "POSIX_TRACE_RESUME",
    "POSIX_TRACE_RUNNING",
    "POSIX_TRACE_START",
    "POSIX_TRACE_STOP",
    "POSIX_TRACE_SUSPENDED",
    "POSIX_TRACE_SYSTEM_EVENTS",
    "POSIX_TRACE_TRUNCATED_READ",
    "POSIX_TRACE_TRUNCATED_RECORD",
    "POSIX_TRACE_UNNAMED_USER_EVENT",
    "POSIX_TRACE_UNTIL_FULL",
    "POSIX_TRACE_WOPID_EVENTS"
    };

static list posix_function = {
    // POSIX.1b function calls Function POSIX Description
    "access",                           // Tests for file accessibility
    "alarm",                            // Schedules an alarm
    "asctime ",                         // Converts a time structure to a string
    "cfgetispeed",                      // Reads terminal input baud rate
    "cfgetospeed",                      // Reads terminal output baud rate
    "cfsetispeed",                      // Sets terminal input baud rate
    "cfsetospeed",                      // Sets terminal output baud rate
    "chdir",                            // Changes current working directory
    "chmod",                            // Changes file mode
    "chown",                            // Changes owner and/or group of a file
    "close",                            // Closes a file
    "closedir",                         // Ends directory read operation
    "creat",                            // Creates a new file or rewrites an existing one
    "ctermid ",                         // Generates terminal pathname
    "cuserid ",                         // Gets user name
    "dup  ",                            // Duplicates an open file descriptor
    "dup2 ",                            // Duplicates an open file descriptor
    "execl",                            // Executes a file
    "execle",                           // Executes a file
    "execlp",                           // Executes a file
    "execv",                            // Executes a file
    "execve",                           // Executes a file
    "execvp",                           // Executes a file
    "_exit",                            // Terminates a process
    "fcntl",                            // Manipulates an open file descriptor
    "fdopen",                           // Opens a stream on a file descriptor
    "fork ",                            // Creates a process
    "fpathconf",                        // Gets configuration variable for an open file
    "fstat",                            // Gets file status
    "getcwd",                           // Gets current working directory
    "getegid ",                         // Gets effective group ID
    "getenv",                           // Gets environment variable
    "geteuid ",                         // Gets effective user ID
    "getgid",                           // Gets real group ID
    "getgrgid",                         // Reads groups database based on group ID
    "getgrnam",                         // Reads groups database based on group name
    "getgroups",                        // Gets supplementary group IDs
    "getlogin",                         // Gets user name
    "getpgrp ",                         // Gets process group ID
    "getpid",                           // Gets process ID
    "getppid ",                         // Gets parent process ID
    "getpwnam",                         // Reads user database based on user name
    "getpwuid",                         // Reads user database based on user ID
    "getuid",                           // Gets real user ID
    "isatty",                           // Determines if a file descriptor is associated with a terminal
    "kill ",                            // Sends a kill signal to a process
    "link ",                            // Creates a link to a file
    "longjmp ",                         // Restores the calling environment
    "lseek",                            // Repositions read/write file offset
    "mkdir",                            // Makes a directory
    "mkfifo",                           // Makes a FIFO special file
    "open ",                            // Opens a file
    "opendir ",                         // Opens a directory
    "pathconf",                         // Gets configuration variables for a path
    "pause",                            // Suspends a process execution
    "pipe ",                            // Creates an interprocess channel
    "read ",                            // Reads from a file
    "readdir ",                         // Reads a directory
    "rename",                           // Renames a file
    "rewinddir",                        // Resets the readdir() pointer
    "rmdir",                            // Removes a directory
    "setgid",                           // Sets group ID
    "setjmp",                           // Saves the calling environment for use by longjmp()
    "setlocale",                        // Sets or queries a program's locale
    "setpgid ",                         // Sets a process group ID for job control
    "setuid",                           // Sets the user ID
    "sigaction",                        // Examines and changes signal action
    "sigaddset",                        // Adds a signal to a signal set
    "sigdelset",                        // Removes a signal to a signal set
    "sigemptyset",                      // Creates an empty signal set
    "sigfillset ",                      // Creates a full set of signals
    "sigismember",                      // Tests a signal for a selected member
    "siglongjmp ",                      // Goes to and restores signal mask
    "sigpending ",                      // Examines pending signals
    "sigprocmask",                      // Examines and changes blocked signals
    "sigsetjmp",                        // Saves state for siglongjmp()
    "sigsuspend ",                      // Waits for a signal
    "sleep",                            // Delays process execution
    "stat ",                            // Gets information about a file
    "sysconf ",                         // Gets system configuration information
    "tcdrain ",                         // Waits for all output to be transmitted to the terminal
    "tcflow",                           // Suspends/restarts terminal output
    "tcflush ",                         // Discards terminal data
    "tcgetattr",                        // Gets terminal attributes
    "tcgetpgrp",                        // Gets foreground process group ID
    "tcsendbreak",                      // Sends a break to a terminal
    "tcsetattr",                        // Sets terminal attributes
    "tcsetpgrp",                        // Sets foreground process group ID
    "time ",                            // Determines the current calendar time
    "times",                            // Gets process times
    "ttyname",                          // Determines a terminal pathname
    "tzset",                            // Sets the timezone from environment variables
    "umask",                            // Sets the file creation mask
    "uname",                            // Gets system name
    "unlink",                           // Removes a directory entry
    "utime",                            // Sets file access and modification times
    "waitpid ",                         // Waits for process termination
    "write",                            // Writes to a file

    // POSIX.1b function calls Function POSIX Description
    "aio_cancel",
    "aio_error",
    "aio_read",
    "aio_return",
    "aio_suspend",
    "aio_write",
    "clock_getres",
    "clock_gettime",
    "clock_settime",
    "fdatasync",
    "fsync   ",
    "kill,",
    "sigqueue",
    "lio_listio",
    "mlock",
    "mlockall",
    "mmap",
    "mprotect",
    "mq_close",
    "mq_getattr",
    "mq_notify",
    "mq_open ",
    "mq_receive",
    "mq_send",
    "mq_setattr",
    "msync",
    "munlock",
    "munlockall",
    "munmap",
    "nanosleep",
    "sched_get_priority_max",
    "sched_get_priority_min",
    "sched_getparam",
    "sched_getscheduler",
    "sched_rr_get_interval ",
    "sched_setparam",
    "sched_setscheduler",
    "sched_yield",
    "sem_close",
    "sem_destroy",
    "sem_getvalue",
    "sem_open",
    "sem_post",
    "sem_unlink",
    "sem_wait,",
    "sem_trywait",
    "shm_open",
    "shm_unlink",
    "sigwaitinfosigtimedwait",
    "timer_create",
    "timer_delete",
    "timer_gettime",
    "timer_settime",
    "wait",
    "waitpid",

    //POSIX.1c function calls Function POSIX Description
    "pthread_atfork",
    "pthread_attr_destroy",
    "pthread_attr_getdetachstate",
    "pthread_attr_getinheritsched ",
    "pthread_attr_getschedparam",
    "pthread_attr_getschedpolicy",
    "pthread_attr_getscope",
    "pthread_attr_getstackaddr",
    "pthread_attr_getstacksize",
    "pthread_attr_init",
    "pthread_attr_setdetachstate",
    "pthread_attr_setinheritsched",
    "pthread_attr_setschedparam",
    "pthread_attr_setschedpolicy",
    "pthread_attr_setscope",
    "pthread_attr_setstackaddr",
    "pthread_attr_setstacksize",
    "pthread_cancel",
    "pthread_cleanup_pop",
    "pthread_cleanup_push",
    "pthread_condattr_destroy",
    "pthread_condattr_getpshared",
    "pthread_condattr_init",
    "pthread_condattr_setpshared",
    "pthread_cond_broadcast",
    "pthread_cond_destroy",
    "pthread_cond_init",
    "pthread_cond_signal",
    "pthread_cond_timedwait",
    "pthread_cond_wait",
    "pthread_create",
    "pthread_detach",
    "pthread_equal",
    "pthread_exit",
    "pthread_getschedparam",
    "pthread_getspecific",
    "pthread_join",
    "pthread_key_create",
    "pthread_key_delete",
    "pthread_kill",
    "pthread_mutexattr_destroy",
    "pthread_mutexattr_getprioceiling",
    "pthread_mutexattr_getprotocol",
    "pthread_mutexattr_getpshared",
    "pthread_mutexattr_init",
    "pthread_mutexattr_setprioceiling",
    "pthread_mutexattr_setprotocol",
    "pthread_mutexattr_setpshared",
    "pthread_mutex_destroy",
    "pthread_mutex_init",
    "pthread_mutex_lock",
    "pthread_mutex_trylock",
    "pthread_mutex_unlock",
    "pthread_once",
    "pthread_self",
    "pthread_setcancelstate",
    "pthread_setcanceltype",
    "pthread_setschedparam",
    "pthread_setspecific",
    "pthread_sigmask",
    "pthread_testcancel",

        /// Base _POSIX_THREAD_SAFE_FUNCTIONS

    "asctime_r",
    "ctime_r",
    "flockfile",
    "ftrylockfile",
    "funlockfile",
    "getc_unlocked",
    "getchar_unlocked",
    "getgrgid_r",
    "getgrnam_r",
    "getpwnam_r",
    "getpwuid_r",
    "gmtime_r",
    "localtime_r",
    "putc_unlocked",
    "putchar_unlocked",
    "rand_r",
    "readdir_r",
    "strtok_r",

        /// Functions in the _XOPEN_REALTIME_THREADS feature group

    "pthread_attr_getinheritsched",
    "pthread_attr_getschedpolicy",
    "pthread_attr_getscope",
    "pthread_attr_setinheritsched",
    "pthread_attr_setschedpolicy",
    "pthread_attr_setscope",
    "pthread_getschedparam",
    "pthread_mutex_getprioceiling",
    "pthread_mutex_setprioceiling",
    "pthread_mutexattr_getprioceiling",
    "pthread_mutexattr_getprotocol",
    "pthread_mutexattr_setprioceiling",
    "pthread_mutexattr_setprotocol",
    "pthread_setschedparam",

        /// X/Open threads extensions

    "pthread_attr_getguardsize",
    "pthread_attr_setguardsize",
    "pthread_getconcurrency",
    "pthread_mutexattr_gettype",
    "pthread_mutexattr_settype",
    "pthread_rwlock_destroy",
    "pthread_rwlock_init",
    "pthread_rwlock_rdlock",
    "pthread_rwlock_tryrdlock",
    "pthread_rwlock_trywrlock",
    "pthread_rwlock_unlock",
    "pthread_rwlock_wrlock",
    "pthread_rwlockattr_destroy",
    "pthread_rwlockattr_getpshared",
    "pthread_rwlockattr_init",
    "pthread_rwlockattr_setpshared",
    "pthread_setconcurrency",

    //POSIX.1d
    "posix_spawn",
    "posix_spawn_file_actions_addclose",
    "posix_spawn_file_actions_adddup2",
    "posix_spawn_file_actions_addopen",
    "posix_spawn_file_actions_destroy",
    "posix_spawn_file_actions_init",
    "posix_spawnattr_destroy",
    "posix_spawnattr_getflags",
    "posix_spawnattr_getpgroup",
    "posix_spawnattr_getschedparam",
    "posix_spawnattr_getschedpolicy",
    "posix_spawnattr_getsigdefault",
    "posix_spawnattr_getsigmask",
    "posix_spawnattr_init ",
    "posix_spawnattr_setflags",
    "posix_spawnattr_setpgroup",
    "posix_spawnattr_setschedparam",
    "posix_spawnattr_setschedpolicy",
    "posix_spawnattr_setsigdefault",
    "posix_spawnattr_setsigmask",
    "posix_spawnp",
    "pthread_mutex_timedlock",
    "sem_timedwait",
    "mq_timedreceive",
    "mq_timedsend",
    "clock_getcpuclockid",
    "pthread_getcpuclockid",
    "posix_fadvise",
    "posix_fallocate",
    "posix_madvise",
    "posix_memalign",

    //POSIX.1j
    "posix_typed_mem_open",
    "posix_mem_offset ",
    "posix_typed_mem_get_info",
    "pthread_spin_init",
    "pthread_spin_destroy",
    "pthread_spin_lock",
    "pthread_spin_trylock",
    "pthread_spin_unlock",
    "pthread_barrierattr_init",
    "pthread_barrierattr_destroy",
    "pthread_barrierattr_getpshared",
    "pthread_barrierattr_setpshared",
    "pthread_barrier_init",
    "pthread_barrier_destroy",
    "pthread_barrier_wait",
    "pthread_rwlockattr_init",
    "pthread_rwlockattr_destroy",
    "pthread_rwlockattr_getpshared",
    "pthread_rwlockattr_setpshared",
    "pthread_rwlock_init",
    "pthread_rwlock_destroy",
    "pthread_rwlock_rdlock",
    "pthread_rwlock_tryrdlock",
    "pthread_rwlock_timedrdlock",
    "pthread_rwlock_wrlock",
    "pthread_rwlock_trywrlock",
    "pthread_rwlock_timedwrlock",
    "pthread_rwlock_unlock",
    "clock_nanosleep",
    "pthread_condattr_setclock",
    "pthread_condattr_getclock",

    //POSIX.1x
    "posix_trace_attr_destroy",
    "posix_trace_attr_getclockres",
    "posix_trace_attr_getcreatetime",
    "posix_trace_attr_getgenversion",
    "posix_trace_attr_getinherited",
    "posix_trace_attr_getlogfullpolicy",
    "posix_trace_attr_getlogsize",
    "posix_trace_attr_getmaxdatasize",
    "posix_trace_attr_getmaxsystemeventsize",
    "posix_trace_attr_getmaxusereventsize",
    "posix_trace_attr_getname",
    "posix_trace_attr_getstreamfullpolicy",
    "posix_trace_attr_getstreamsize",
    "posix_trace_attr_init",
    "posix_trace_attr_setinherited",
    "posix_trace_attr_setlogfullpolicy",
    "posix_trace_attr_setlogsize",
    "posix_trace_attr_setmaxdatasize",
    "posix_trace_attr_setname",
    "posix_trace_attr_setstreamfullpolicy",
    "posix_trace_attr_setstreamsize",
    "posix_trace_clear",
    "posix_trace_close",
    "posix_trace_create",
    "posix_trace_create_withlog",
    "posix_trace_event",
    "posix_trace_eventid_equal",
    "posix_trace_eventid_get_name",
    "posix_trace_eventid_open",
    "posix_trace_eventset_add",
    "posix_trace_eventset_del",
    "posix_trace_eventset_empty",
    "posix_trace_eventset_fill",
    "posix_trace_eventset_ismember",
    "posix_trace_eventtypelist_getnext_id",
    "posix_trace_eventtypelist_rewind",
    "posix_trace_flush",
    "posix_trace_get_attr",
    "posix_trace_get_filter",
    "posix_trace_get_status",
    "posix_trace_getnext_event",
    "posix_trace_open",
    "posix_trace_rewind",
    "posix_trace_set_filter",
    "posix_trace_shutdown",
    "posix_trace_start",
    "posix_trace_stop",
    "posix_trace_timedgetnext_event",
    "posix_trace_trid_eventid_open",
    "posix_trace_trygetnext_event",
    };

void
posix_keyword()
{
    define_keywords(SYNK_CONSTANT, posix_constant);
    define_keywords(SYNK_FUNCTION, posix_function);
}
//end
