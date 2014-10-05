/****************************************************************************
 * include/aio.h
 *
 *   Copyright (C) 2014 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#ifndef __INCLUDE_AIO_H
#define __INCLUDE_AIO_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>
#include <signal.h>
#include <time.h>

#include <nuttx/wqueue.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
/* Configuration ************************************************************/
/* These interfaces are not available to kernel code */

#if (defined(CONFIG_BUILD_PROTECTED) || defined(CONFIG_BUILD_KERNEL)) && defined(__KERNEL__)
#  undef CONFIG_LIBC_AIO
#endif

/* Work queue support is required.  In the flat, embedded build the low-
 * priority work queue is required so that the asynchronous I/O does not
 * interfere with high priority driver operations.  In the protected and
 * kernel mode builds, user-space work queue support is required.  If these
 * pre-requisites are met, then asynchronous I/O support can be enabled with
 * CONFIG_LIBC_AIO
 */

#ifdef CONFIG_LIBC_AIO

#ifndef CONFIG_SCHED_WORKQUEUE
#  error Asynchronous I/O requires CONFIG_SCHED_WORKQUEUE
#else
#  if defined (CONFIG_BUILD_PROTECTED) || defined(CONFIG_BUILD_KERNEL)
#    ifndef CONFIG_SCHED_USRWORK
#      error User-space asynchronous I/O requires CONFIG_SCHED_USRWORK
#    endif
#  else
#    ifndef CONFIG_SCHED_LPWORK
#      error Flat-build asynchronous I/O requires CONFIG_SCHED_LPWORK
#    endif
#  endif
#endif

/* Standard Definitions *****************************************************/
/* aio_cancel return values
 *
 * AIO_ALLDONE     - Indicates that none of the requested operations could
 *                   be cancelled since they are already complete.
 * AIO_CANCELED    - Indicates that all requested operations have been
 *                   cancelled.
 * AIO_NOTCANCELED - Indicates that some of the requested operations could
 *                   not be cancelled since they are in progress.
 */

#define AIO_CANCELED    0
#define AIO_ALLDONE     1
#define AIO_NOTCANCELED 2

/* lio_listio element operations
 *
 * LIO_NOP         - Indicates that no transfer is requested.
 * LIO_READ        - Requests a read operation.
 * LIO_WRITE       - Requests a write operation.
 */

#define LIO_NOP         0
#define LIO_READ        1
#define LIO_WRITE       2

/* lio_listio modes
 *
 * LIO_NOWAIT      - Indicates that the calling thread is to continue
 *                   execution while the lio_listio() operation is being
 *                   performed, and no notification is given when the
 *                   operation is complete.
 * LIO_WAIT        - Indicates that the calling thread is to suspend until
 *                   the lio_listio() operation is complete.
 */

#define LIO_NOWAIT      0
#define LIO_WAIT        1

/****************************************************************************
 * Type Definitions
 ****************************************************************************/

struct aiocb
{
  /* Standard fields required by POSIX */

  struct sigevent aio_sigevent;  /* Signal number and value */
  FAR volatile void *aio_buf;    /* Location of buffer */
  off_t aio_offset;              /* File offset */
  size_t aio_nbytes;             /* Length of transfer */
  int aio_fildes;                /* File descriptor */
  int aio_reqprio;               /* Request priority offset */
  int aio_lio_opcode;            /* Operation to be performed */

  /* Non-standard, implementation-dependent data */

  struct work_s aio_work;        /* Used to defer I/O to the work thread */
  pid_t aio_pid;                 /* ID of client to be notify at completion */
  volatile ssize_t aio_result;   /* Support for aio_error() and aio_return() */
};

/****************************************************************************
 * Public Data
 ****************************************************************************/

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

int aio_cancel(int, FAR struct aiocb *aiocbp);
int aio_error(FAR const struct aiocb *aiocbp);
int aio_fsync(int, FAR struct aiocb *aiocbp);
int aio_read(FAR struct aiocb *aiocbp);
ssize_t aio_return(FAR struct aiocb *aiocbp);
int aio_suspend(FAR const struct aiocb *const list[], int nent,
                FAR const struct timespec *timeout);
int aio_write(FAR struct aiocb *aiocbp);

#ifndef CONFIG_PTHREAD_DISABLE /* Depends on pthread support */
int lio_listio(int mode, FAR struct aiocb *const list[], int nent,
               FAR struct sigevent *sig);
#endif

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif /* CONFIG_LIBC_AIO */
#endif /* __INCLUDE_AIO_H */
