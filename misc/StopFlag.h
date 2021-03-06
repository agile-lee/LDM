/**
 * This file defines a "stop" flag.
 *
 * Copyright 2018, University Corporation for Atmospheric Research
 * All rights reserved. See file COPYRIGHT in the top-level source-directory for
 * copying and redistribution conditions.
 * 
 *        File: StopFlag.h
 *  Created on: May 2, 2018
 *      Author: Steven R. Emmerson
 */
#include "config.h"

#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>

#ifndef MCAST_LIB_LDM7_DONE_H_
#define MCAST_LIB_LDM7_DONE_H_

struct stopFlag {
    pthread_mutex_t        mutex;
    pthread_cond_t         cond;
    bool                 (*done)(void);
    bool                   isSet;
};
typedef struct stopFlag StopFlag;

#ifdef __cplusplus
    extern "C" {
#endif

/**
 * Initializes a stop flag.
 *
 * @param[out] stopFlag     Stop flag
 * @param[in]  done         Optional external "done" function or `NULL`
 * @retval     0            Success
 * @retval     EAGAIN       Out of resources. `log_add()` called.
 * @retval     ENOMEM       Out of memory. `log_add()` called.
 */
int
stopFlag_init(
        StopFlag* const stopFlag,
        bool          (*done)(void));

void
stopFlag_destroy(StopFlag* const stopFlag);

void
stopFlag_set(StopFlag* const stopFlag);

bool
stopFlag_shouldStop(StopFlag* const stopFlag);

/**
 * Waits until the flag is set or the external "done" function returns true (if
 * not `NULL`), whichever comes first.
 * @param[in] stopFlag  Stop flag
 * @asyncsignalsafety   Unsafe
 */
void
stopFlag_wait(StopFlag* const stopFlag);

/**
 * Waits until the flag is set, the external "done" function returns true (if
 * not `NULL`), or a given time is reached, whichever comes first.
 * @param[in] stopFlag  Stop flag
 * @param[in] when      When to stop waiting
 * @asyncsignalsafety   Unsafe
 */
void
stopFlag_timedWait(
        StopFlag* const restrict              stopFlag,
        const struct timespec* const restrict when);

#ifdef __cplusplus
    }
#endif

#endif /* MCAST_LIB_LDM7_DONE_H_ */
