/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_doppler.c
 *
 * Doppler tweakage.
 */

#include "al_siteconfig.h"

#include <AL/al.h>

#include "al_main.h"
#include "al_error.h"

#include "alc/alc_context.h"

#define MIN_DOPPLER 0.0
#define MAX_DOPPLER 40000.0
#define MIN_DOPPLER_VELOCITY 0.001
#define MAX_DOPPLER_VELOCITY 40000.0

/*
 * _alDopplerFactor( ALfloat value )
 *
 * Non locking version of alDopplerFactor.
 *
 * Assumes locked context.
 */
static void _alDopplerFactor( ALfloat value )
{
	AL_context *cc;
	ALboolean inrange = _alCheckRangef( value, MIN_DOPPLER, MAX_DOPPLER );
	if( inrange == AL_FALSE ) {
		_alDCSetError( AL_INVALID_VALUE );
		return;
	}

	cc = _alcDCGetContext(  );
	if( cc == NULL ) {
		return;
	}

	cc->doppler_factor = value;
}

/*
 * alDopplerFactor( ALfloat value )
 *
 * Sets the doppler factor used by the doppler filter.
 */
void alDopplerFactor( ALfloat value )
{
	_alcDCLockContext(  );
	_alDopplerFactor( value );
	_alcDCUnlockContext(  );
}

/*
 * _alDopplerVelocity( ALfloat value )
 *
 * Non locking version of alDopplerVelocity
 *
 * Assumes locked context.
 */
static void _alDopplerVelocity( ALfloat value )
{
	AL_context *cc;
	ALboolean inrange = _alCheckRangef( value,
					    MIN_DOPPLER_VELOCITY,
					    MAX_DOPPLER_VELOCITY );
	if( inrange == AL_FALSE ) {
		_alDCSetError( AL_INVALID_VALUE );
		return;
	}

	cc = _alcDCGetContext(  );
	if( cc == NULL ) {
		return;
	}

	cc->doppler_velocity = value;
}

/*
 * alDopplerVelocity( ALfloat value )
 *
 * Sets the doppler velocity used by the doppler filter.
 */
void alDopplerVelocity( ALfloat value )
{
	_alcDCLockContext(  );
	_alDopplerVelocity( value );
	_alcDCUnlockContext(  );
}

/*
 * alSpeedOfSound( ALfloat value )
 *
 * Sets the speed of sound used by the doppler filter.
 */
void alSpeedOfSound( UNUSED( ALfloat value ) )
{
	/* FIXME -- no code here */
}
