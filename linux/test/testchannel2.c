#include "testlib.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <math.h>

#define WAVEFILE "sample.wav"
#define DATABUFFERSIZE (10 * (512 * 3) * 1024)

static void init( const char *fname );

static ALuint movingSource = 0;

static time_t start;
static void *data = ( void * ) 0xDEADBEEF;

static ALCcontext *context;

static void init( const char *fname )
{
	FILE *fh;
	ALfloat zeroes[] = { 0.0f, 0.0f, 0.0f };
	ALfloat back[] = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
	ALfloat front[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
	ALuint stereo;
	int filelen;

	data = malloc( DATABUFFERSIZE );

	start = time( NULL );

	alListenerfv( AL_POSITION, zeroes );
	alListenerfv( AL_VELOCITY, zeroes );
	alListenerfv( AL_ORIENTATION, front );

	alGenBuffers( 1, &stereo );

	fh = fopen( fname, "rb" );
	if( fh == NULL ) {
		fprintf( stderr, "Couldn't open %s\n", fname );
		exit( EXIT_FAILURE );
	}

	filelen = fread( data, 1, DATABUFFERSIZE, fh );
	fclose( fh );

	alGetError(  );

	alBufferData( stereo, AL_FORMAT_WAVE_EXT, data, filelen, 0 );
	if( alGetError(  ) != AL_NO_ERROR ) {
		fprintf( stderr, "Could not BufferData\n" );
		exit( EXIT_FAILURE );
	}
	alGenSources( 1, &movingSource );

	alSourcefv( movingSource, AL_VELOCITY, zeroes );
	alSourcefv( movingSource, AL_ORIENTATION, back );
	alSourcei( movingSource, AL_BUFFER, stereo );
	alSourcei( movingSource, AL_LOOPING, AL_FALSE );
}

int main( int argc, char *argv[] )
{
	ALCdevice *device;
	int attributeList[] = { ALC_FREQUENCY, 22050,
		0
	};
	ALfloat gain;

	device = alcOpenDevice( NULL );
	if( device == NULL ) {
		return EXIT_FAILURE;
	}

	/* Initialize ALUT. */
	context = alcCreateContext( device, attributeList );
	if( context == NULL ) {
		alcCloseDevice( device );

		return EXIT_FAILURE;
	}

	alcMakeContextCurrent( context );

	getExtensionEntries(  );

	if( argc == 1 ) {
		init( WAVEFILE );
	} else {
		init( argv[1] );
	}

	gain = palcGetAudioChannel( ALC_CHAN_CD_LOKI );
	fprintf( stderr, "get ALC_CHAN_CD_LOKI = %f\n", gain );
	palcSetAudioChannel( ALC_CHAN_CD_LOKI, 0.35 );
	fprintf( stderr, "set ALC_CHAN_CD 0.35, get = %f\n",
		 palcGetAudioChannel( ALC_CHAN_CD_LOKI ) );
	palcSetAudioChannel( ALC_CHAN_CD_LOKI, gain );
	fprintf( stderr, "set ALC_CHAN_CD %f get = %f\n",
		 gain, palcGetAudioChannel( ALC_CHAN_CD_LOKI ) );

	free( data );
	alcDestroyContext( context );
	alcCloseDevice( device );

	return EXIT_SUCCESS;
}
