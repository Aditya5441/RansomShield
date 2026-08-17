/**
 * @file    syscalls.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>

extern int __io_putchar(int ch) __attribute__((weak));
extern int __io_getchar(void) __attribute__((weak));

char *__env[1] = { 0 };
char **environ = __env;

/**
 * @brief   TODO: describe what initialise_monitor_handles() does
 */
void initialise_monitor_handles()
{
}

/**
 * @brief   TODO: describe what _getpid() does
 * @retval  TODO: describe return value
 */
int _getpid(void)
{
  return 1;
}

/**
 * @brief   TODO: describe what _kill() does
 * @param   pid  TODO: describe parameter
 * @param   sig  TODO: describe parameter
 * @retval  TODO: describe return value
 */
int _kill(int pid, int sig)
{
  (void)pid;
  (void)sig;
  errno = EINVAL;
  return -1;
}

/**
 * @brief   TODO: describe what _exit() does
 * @param   status  TODO: describe parameter
 */
void _exit (int status)
{
  _kill(status, -1);
  while (1) {}
}

__attribute__((weak)) int _read(int file, char *ptr, int len)
{
  (void)file;
  int DataIdx;

  for (DataIdx = 0; DataIdx < len; DataIdx++)
  {
    *ptr++ = __io_getchar();
  }

  return len;
}

__attribute__((weak)) int _write(int file, char *ptr, int len)
{
  (void)file;
  int DataIdx;

  for (DataIdx = 0; DataIdx < len; DataIdx++)
  {
    __io_putchar(*ptr++);
  }
  return len;
}

/**
 * @brief   TODO: describe what _close() does
 * @param   file  TODO: describe parameter
 * @retval  TODO: describe return value
 */
int _close(int file)
{
  (void)file;
  return -1;
}

/**
 * @brief   TODO: describe what _fstat() does
 * @param   file  TODO: describe parameter
 * @param   st  TODO: describe parameter
 * @retval  TODO: describe return value
 */
int _fstat(int file, struct stat *st)
{
  (void)file;
  st->st_mode = S_IFCHR;
  return 0;
}

/**
 * @brief   TODO: describe what _isatty() does
 * @param   file  TODO: describe parameter
 * @retval  TODO: describe return value
 */
int _isatty(int file)
{
  (void)file;
  return 1;
}

/**
 * @brief   TODO: describe what _lseek() does
 * @param   file  TODO: describe parameter
 * @param   ptr  TODO: describe parameter
 * @param   dir  TODO: describe parameter
 * @retval  TODO: describe return value
 */
int _lseek(int file, int ptr, int dir)
{
  (void)file;
  (void)ptr;
  (void)dir;
  return 0;
}

/**
 * @brief   TODO: describe what _open() does
 * @param   path  TODO: describe parameter
 * @param   flags  TODO: describe parameter
 * @param   ...  TODO: describe parameter
 * @retval  TODO: describe return value
 */
int _open(char *path, int flags, ...)
{
  (void)path;
  (void)flags;

  return -1;
}

/**
 * @brief   TODO: describe what _wait() does
 * @param   status  TODO: describe parameter
 * @retval  TODO: describe return value
 */
int _wait(int *status)
{
  (void)status;
  errno = ECHILD;
  return -1;
}

/**
 * @brief   TODO: describe what _unlink() does
 * @param   name  TODO: describe parameter
 * @retval  TODO: describe return value
 */
int _unlink(char *name)
{
  (void)name;
  errno = ENOENT;
  return -1;
}

/**
 * @brief   TODO: describe what _times() does
 * @param   buf  TODO: describe parameter
 * @retval  TODO: describe return value
 */
int _times(struct tms *buf)
{
  (void)buf;
  return -1;
}

/**
 * @brief   TODO: describe what _stat() does
 * @param   file  TODO: describe parameter
 * @param   st  TODO: describe parameter
 * @retval  TODO: describe return value
 */
int _stat(char *file, struct stat *st)
{
  (void)file;
  st->st_mode = S_IFCHR;
  return 0;
}

/**
 * @brief   TODO: describe what _link() does
 * @param   old  TODO: describe parameter
 * @param   new  TODO: describe parameter
 * @retval  TODO: describe return value
 */
int _link(char *old, char *new)
{
  (void)old;
  (void)new;
  errno = EMLINK;
  return -1;
}

/**
 * @brief   TODO: describe what _fork() does
 * @retval  TODO: describe return value
 */
int _fork(void)
{
  errno = EAGAIN;
  return -1;
}

/**
 * @brief   TODO: describe what _execve() does
 * @param   name  TODO: describe parameter
 * @param   argv  TODO: describe parameter
 * @param   env  TODO: describe parameter
 * @retval  TODO: describe return value
 */
int _execve(char *name, char **argv, char **env)
{
  (void)name;
  (void)argv;
  (void)env;
  errno = ENOMEM;
  return -1;
}
