/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * arts.c
 *
 * arts backend.
 */
#include "al_siteconfig.h"

#include <AL/al.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <artsc.h>

#if OPENAL_DLOPEN_ARTS
#include <dlfcn.h>
#endif

#include "al_main.h"
#include "al_debug.h"

#include "arch/arts/arts.h"
#include "arch/interface/interface_sound.h"

#define DEF_SPEED	_ALC_CANON_SPEED
#define DEF_SIZE	_AL_DEF_BUFSIZ
#define DEF_SAMPLES     (DEF_SIZE / 2)
#define DEF_BITS        16
#define DEF_CHANNELS	2

static int openal_arts_ref_count = 0;

typedef struct {
	arts_stream_t stream;
} t_arts_handle;

static const char *genartskey(void);
static int openal_load_arts_library(void);

/*
 * aRts library functions.
 */
static int (*parts_init)(void);
static void (*parts_free)(void);
static int (*parts_suspend)(void);
static int (*parts_suspended)(void);
static const char *(*parts_error_text)(int errorcode);
static arts_stream_t (*parts_play_stream)(int rate, int bits, int channels, const char *name);
static arts_stream_t (*parts_record_stream)(int rate, int bits, int channels, const char *name);
static void (*parts_close_stream)(arts_stream_t stream);
static int (*parts_read)(arts_stream_t stream, void *buffer, int count);
static int (*parts_write)(arts_stream_t stream, const void *buffer, int count);
static int (*parts_stream_set)(arts_stream_t stream, arts_parameter_t param, int value);
static int (*parts_stream_get)(arts_stream_t stream, arts_parameter_t param);

/*
 * aRts library handle.
 */
static void * arts_lib_handle = NULL;

static int openal_load_arts_library(void)
{
#if OPENAL_DLOPEN_ARTS
        char * error = NULL;
#endif
    
	if (arts_lib_handle != NULL)
		return 1;  /* already loaded. */

	#if OPENAL_DLOPEN_ARTS
		#define OPENAL_LOAD_ARTS_SYMBOL(x) p##x = dlsym(arts_lib_handle, #x); \
                                                   error = dlerror(); \
                                                   if (p##x == NULL) { \
                                                           fprintf(stderr,"Could not resolve aRts symbol %s: %s\n", #x, ((error!=NULL)?(error):("(null)"))); \
                                                           dlclose(arts_lib_handle); arts_lib_handle = NULL; \
                                                           return 0; }
                dlerror(); /* clear error state */
		arts_lib_handle = dlopen("libartsc.so", RTLD_LAZY | RTLD_GLOBAL);
                error = dlerror();
		if (arts_lib_handle == NULL) {
                        fprintf(stderr,"Could not open aRts library: %s\n",((error!=NULL)?(error):("(null)")));
			return 0;
                }
	#else
		#define OPENAL_LOAD_ARTS_SYMBOL(x) p##x = x;
		arts_lib_handle = (void *) 0xF00DF00D;
	#endif

        OPENAL_LOAD_ARTS_SYMBOL(arts_init);
        OPENAL_LOAD_ARTS_SYMBOL(arts_free);
        OPENAL_LOAD_ARTS_SYMBOL(arts_suspend);
        OPENAL_LOAD_ARTS_SYMBOL(arts_suspended);
        OPENAL_LOAD_ARTS_SYMBOL(arts_error_text);
        OPENAL_LOAD_ARTS_SYMBOL(arts_play_stream);
        OPENAL_LOAD_ARTS_SYMBOL(arts_record_stream);
        OPENAL_LOAD_ARTS_SYMBOL(arts_close_stream);
        OPENAL_LOAD_ARTS_SYMBOL(arts_read);
        OPENAL_LOAD_ARTS_SYMBOL(arts_write);
        OPENAL_LOAD_ARTS_SYMBOL(arts_stream_set);
        OPENAL_LOAD_ARTS_SYMBOL(arts_stream_get);

	return 1;
}

void *grab_read_arts(void) {
	return NULL;
}

void *grab_write_arts(void) {
        int err;
        t_arts_handle * ahandle;

        if (!openal_load_arts_library()) {
                return NULL;
        }
    
        if (!(ahandle=malloc(sizeof(t_arts_handle))))
                return NULL;
        
        if (openal_arts_ref_count==0) {
                err = parts_init();

                if(err < 0) {
                        fprintf(stderr, "aRTs init failed: %s\n",
                                parts_error_text(err));
                        free(ahandle);
                        return NULL;
                }
        }

        openal_arts_ref_count++;
        
	_alBlitBuffer = arts_blitbuffer;

#if 1
        ahandle->stream = NULL;
#else
        ahandle->stream = parts_play_stream(DEF_SPEED,
                                            DEF_BITS,
                                            DEF_CHANNELS,
				 	    genartskey());
#endif
        
	fprintf(stderr, "arts grab audio ok\n");

	_alDebug(ALD_CONTEXT, __FILE__, __LINE__,
		"arts grab audio ok");

        return ahandle;
}

void arts_blitbuffer(void *handle, void *data, int bytes)  {
	t_arts_handle * ahandle = (t_arts_handle *) handle;

	if ((ahandle == NULL)||(ahandle->stream == NULL)) {
		return;
	}

	parts_write(ahandle->stream, data, bytes);

        return;
}

void release_arts(void *handle) {
	t_arts_handle * ahandle = (t_arts_handle *) handle;

	if ((ahandle == NULL)||(ahandle->stream == NULL)) {
		return;
	}

        openal_arts_ref_count--;
        
	parts_close_stream(ahandle->stream);
        free(ahandle);

        if (openal_arts_ref_count==0)
                parts_free();

	return;
}

static const char *genartskey(void) {
	static char retval[1024];

	snprintf(retval, sizeof(retval), "openal%d", getpid());

	return retval;
}

ALboolean set_write_arts(void *handle,
		   ALuint *bufsiz,
		   ALenum *fmt,
		   ALuint *speed) {
	t_arts_handle * ahandle = (t_arts_handle *) handle;

	if (ahandle == NULL) {
		return AL_FALSE;
	}

#if 0
	fprintf(stderr, "set_arts forcing speed from %d to 44100\n", *speed);

        /* @@@ FIXME: What was this good for?! */
	*speed = 44100;
#endif

        if (ahandle->stream != NULL)
                parts_close_stream(ahandle->stream);

        /* According to the aRtsC library source, this function
         * returns 0 if it fails.
         */
	ahandle->stream = parts_play_stream(*speed,
					    _al_formatbits(*fmt),
					    _al_ALCHANNELS(*fmt),
                                            genartskey());
        
        if (ahandle->stream == 0) {
                ahandle->stream = NULL;
                return AL_FALSE;
        }
        
	parts_stream_set(ahandle->stream, ARTS_P_BUFFER_SIZE, *bufsiz);
	parts_stream_set(ahandle->stream, ARTS_P_BLOCKING, 1);

	*bufsiz = parts_stream_get(ahandle->stream, ARTS_P_BUFFER_SIZE);

        return AL_TRUE;
}

ALboolean set_read_arts(UNUSED(void *handle),
		   UNUSED(ALuint *bufsiz),
		   UNUSED(ALenum *fmt),
		   UNUSED(ALuint *speed)) {
	return AL_FALSE;
}
