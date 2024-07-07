/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_threadlib.h
 *
 * Header which sorts out which thread package we're using.
 */
#ifndef THREADLIB_H_
#define THREADLIB_H_

#include "al_siteconfig.h"

#if defined(USE_POSIXTHREADS)

#include <pthread.h>
typedef pthread_t *ThreadID;

#elif defined(USE_WINDOWSTHREADS)

#include <windows.h>
typedef HANDLE ThreadID;

#elif defined(USE_MORPHOSTHREADS)

#include <exec/ports.h>
#include <dos/dosextens.h>

struct ThreadData {
	struct Process *td_Thread;
	struct MsgPort *td_MsgPort;
};

typedef struct ThreadData *ThreadID;

#else

#error "No thread package"

#endif

/*
 * _alCreateThread( int (*fn )(void *) )
 *
 * Creates a thread, which starts by running fn.
 */
extern ThreadID _alCreateThread( int ( *fn ) ( void * ) );

/*
 * _alWaitThread( ThreadID thread )
 *
 * Waits for thread to terminate before returning.
 */
extern int _alWaitThread( ThreadID thread );

/*
 * _alSelfThread( void )
 *
 * Returns the identifier for the callee's thread.
 */
extern unsigned int _alSelfThread( void );

/*
 * _alExitThread( void )
 *
 * Forces the callee to terminate.
 */
extern void _alExitThread( void );

#endif				/* THREADLIB_H_ */
