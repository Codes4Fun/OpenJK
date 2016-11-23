/*
===========================================================================
Copyright (C) 1999 - 2005, Id Software, Inc.
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
Copyright (C) 2013 - 2015, OpenJK contributors

This file is part of the OpenJK source code.

OpenJK is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/

#include "../server/exe_headers.h"

#include "tr_local.h"
#include "tr_common.h"

#include <openvr.h>

backEndData_t	*backEndData;
backEndState_t	backEnd;

bool tr_stencilled = false;
extern qboolean tr_distortionPrePost; //tr_shadows.cpp
extern qboolean tr_distortionNegate; //tr_shadows.cpp
extern void RB_CaptureScreenImage(void); //tr_shadows.cpp
extern void RB_DistortionFill(void); //tr_shadows.cpp
static void RB_DrawGlowOverlay();
static void RB_BlurGlowTexture();

// Whether we are currently rendering only glowing objects or not.
bool g_bRenderGlowingObjects = false;

// Whether the current hardware supports dynamic glows/flares.
bool g_bDynamicGlowSupported = false;

static const float s_flipMatrix[16] = {
	// convert from our coordinate system (looking down X)
	// to OpenGL's coordinate system (looking down -Z)
	0, 0, -1, 0,
	-1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 0, 1
};


/*
** GL_Bind
*/
void GL_Bind( image_t *image ) {
	int texnum;

	if ( !image ) {
		ri.Printf( PRINT_WARNING, "GL_Bind: NULL image\n" );
		texnum = tr.defaultImage->texnum;
	} else {
		texnum = image->texnum;
	}

	if ( r_nobind->integer && tr.dlightImage ) {		// performance evaluation option
		texnum = tr.dlightImage->texnum;
	}

	if ( glState.currenttextures[glState.currenttmu] != texnum ) {
		image->frameUsed = tr.frameCount;
		glState.currenttextures[glState.currenttmu] = texnum;
		qglBindTexture (GL_TEXTURE_2D, texnum);
	}
}

/*
** GL_SelectTexture
*/
void GL_SelectTexture( int unit )
{
	if ( glState.currenttmu == unit )
	{
		return;
	}

	if ( unit == 0 )
	{
		qglActiveTextureARB( GL_TEXTURE0_ARB );
		GLimp_LogComment( "glActiveTextureARB( GL_TEXTURE0_ARB )\n" );
		qglClientActiveTextureARB( GL_TEXTURE0_ARB );
		GLimp_LogComment( "glClientActiveTextureARB( GL_TEXTURE0_ARB )\n" );
	}
	else if ( unit == 1 )
	{
		qglActiveTextureARB( GL_TEXTURE1_ARB );
		GLimp_LogComment( "glActiveTextureARB( GL_TEXTURE1_ARB )\n" );
		qglClientActiveTextureARB( GL_TEXTURE1_ARB );
		GLimp_LogComment( "glClientActiveTextureARB( GL_TEXTURE1_ARB )\n" );
	}
	else if ( unit == 2 )
	{
		qglActiveTextureARB( GL_TEXTURE2_ARB );
		GLimp_LogComment( "glActiveTextureARB( GL_TEXTURE2_ARB )\n" );
		qglClientActiveTextureARB( GL_TEXTURE2_ARB );
		GLimp_LogComment( "glClientActiveTextureARB( GL_TEXTURE2_ARB )\n" );
	}
	else if ( unit == 3 )
	{
		qglActiveTextureARB( GL_TEXTURE3_ARB );
		GLimp_LogComment( "glActiveTextureARB( GL_TEXTURE3_ARB )\n" );
		qglClientActiveTextureARB( GL_TEXTURE3_ARB );
		GLimp_LogComment( "glClientActiveTextureARB( GL_TEXTURE3_ARB )\n" );
	}
	else {
		Com_Error( ERR_DROP, "GL_SelectTexture: unit = %i", unit );
	}

	glState.currenttmu = unit;
}


/*
** GL_Cull
*/
void GL_Cull( int cullType ) {
	if ( glState.faceCulling == cullType ) {
		return;
	}
	glState.faceCulling = cullType;
	if (backEnd.projection2D){	//don't care, we're in 2d when it's always disabled
		return;
	}

	if ( cullType == CT_TWO_SIDED )
	{
		qglDisable( GL_CULL_FACE );
	}
	else
	{
		qglEnable( GL_CULL_FACE );

		if ( cullType == CT_BACK_SIDED )
		{
			if ( backEnd.viewParms.isMirror )
			{
				qglCullFace( GL_FRONT );
			}
			else
			{
				qglCullFace( GL_BACK );
			}
		}
		else
		{
			if ( backEnd.viewParms.isMirror )
			{
				qglCullFace( GL_BACK );
			}
			else
			{
				qglCullFace( GL_FRONT );
			}
		}
	}
}

/*
** GL_TexEnv
*/
void GL_TexEnv( int env )
{
	if ( env == glState.texEnv[glState.currenttmu] )
	{
		return;
	}

	glState.texEnv[glState.currenttmu] = env;


	switch ( env )
	{
	case GL_MODULATE:
		qglTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
		break;
	case GL_REPLACE:
		qglTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
		break;
	case GL_DECAL:
		qglTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );
		break;
	case GL_ADD:
		qglTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD );
		break;
	default:
		Com_Error( ERR_DROP, "GL_TexEnv: invalid env '%d' passed\n", env );
		break;
	}
}

/*
** GL_StateOverride
**
** This routine overrides states set by shaders.
** stateMask are bits we want, stateBits are what are set.
*/
void GL_StateOverride(uint32_t stateMask, uint32_t stateBits)
{
	glState.glStateOverrideMask = ~stateMask;
	glState.glStateOverrideBits = stateBits & stateMask;
	GL_State(glState.glStateBits);
}

/*
** GL_IsStateSet
**
** This routine checks if a state was set.
*/
qboolean GL_IsStateSet(uint32_t stateMask, uint32_t stateBits)
{
	stateBits = (stateBits & glState.glStateOverrideMask) | glState.glStateOverrideBits;
	if ((glState.glStateBits & stateMask) == stateBits)
	{
		return qtrue;
	}
	return qfalse;
}

/*
** GL_State
**
** This routine is responsible for setting the most commonly changed state
** in Q3.
*/
void GL_State( uint32_t stateBits )
{
	stateBits = (stateBits & glState.glStateOverrideMask) | glState.glStateOverrideBits;
	uint32_t diff = stateBits ^ glState.glStateBits;

	if ( !diff )
	{
		return;
	}

	//
	// check depthFunc bits
	//
	if ( diff & GLS_DEPTHFUNC_EQUAL )
	{
		if ( stateBits & GLS_DEPTHFUNC_EQUAL )
		{
			qglDepthFunc( GL_EQUAL );
		}
		else
		{
			qglDepthFunc( GL_LEQUAL );
		}
	}

	//
	// check blend bits
	//
	if ( diff & ( GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS ) )
	{
		GLenum srcFactor, dstFactor;

		if ( stateBits & ( GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS ) )
		{
			switch ( stateBits & GLS_SRCBLEND_BITS )
			{
			case GLS_SRCBLEND_ZERO:
				srcFactor = GL_ZERO;
				break;
			case GLS_SRCBLEND_ONE:
				srcFactor = GL_ONE;
				break;
			case GLS_SRCBLEND_DST_COLOR:
				srcFactor = GL_DST_COLOR;
				break;
			case GLS_SRCBLEND_ONE_MINUS_DST_COLOR:
				srcFactor = GL_ONE_MINUS_DST_COLOR;
				break;
			case GLS_SRCBLEND_SRC_ALPHA:
				srcFactor = GL_SRC_ALPHA;
				break;
			case GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA:
				srcFactor = GL_ONE_MINUS_SRC_ALPHA;
				break;
			case GLS_SRCBLEND_DST_ALPHA:
				srcFactor = GL_DST_ALPHA;
				break;
			case GLS_SRCBLEND_ONE_MINUS_DST_ALPHA:
				srcFactor = GL_ONE_MINUS_DST_ALPHA;
				break;
			case GLS_SRCBLEND_ALPHA_SATURATE:
				srcFactor = GL_SRC_ALPHA_SATURATE;
				break;
			default:
				srcFactor = GL_ONE;		// to get warning to shut up
				Com_Error( ERR_DROP, "GL_State: invalid src blend state bits\n" );
				break;
			}

			switch ( stateBits & GLS_DSTBLEND_BITS )
			{
			case GLS_DSTBLEND_ZERO:
				dstFactor = GL_ZERO;
				break;
			case GLS_DSTBLEND_ONE:
				dstFactor = GL_ONE;
				break;
			case GLS_DSTBLEND_SRC_COLOR:
				dstFactor = GL_SRC_COLOR;
				break;
			case GLS_DSTBLEND_ONE_MINUS_SRC_COLOR:
				dstFactor = GL_ONE_MINUS_SRC_COLOR;
				break;
			case GLS_DSTBLEND_SRC_ALPHA:
				dstFactor = GL_SRC_ALPHA;
				break;
			case GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA:
				dstFactor = GL_ONE_MINUS_SRC_ALPHA;
				break;
			case GLS_DSTBLEND_DST_ALPHA:
				dstFactor = GL_DST_ALPHA;
				break;
			case GLS_DSTBLEND_ONE_MINUS_DST_ALPHA:
				dstFactor = GL_ONE_MINUS_DST_ALPHA;
				break;
			default:
				dstFactor = GL_ONE;		// to get warning to shut up
				Com_Error( ERR_DROP, "GL_State: invalid dst blend state bits\n" );
				break;
			}

			qglEnable( GL_BLEND );
			qglBlendFunc( srcFactor, dstFactor );
		}
		else
		{
			qglDisable( GL_BLEND );
		}
	}

	//
	// check depthmask
	//
	if ( diff & GLS_DEPTHMASK_TRUE )
	{
		if ( stateBits & GLS_DEPTHMASK_TRUE )
		{
			qglDepthMask( GL_TRUE );
		}
		else
		{
			qglDepthMask( GL_FALSE );
		}
	}

	//
	// fill/line mode
	//
	if ( diff & GLS_POLYMODE_LINE )
	{
		if ( stateBits & GLS_POLYMODE_LINE )
		{
			qglPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		else
		{
			qglPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
	}

	//
	// depthtest
	//
	if ( diff & GLS_DEPTHTEST_DISABLE )
	{
		if ( stateBits & GLS_DEPTHTEST_DISABLE )
		{
			qglDisable( GL_DEPTH_TEST );
		}
		else
		{
			qglEnable( GL_DEPTH_TEST );
		}
	}

	//
	// alpha test
	//
	if ( diff & GLS_ATEST_BITS )
	{
		switch ( stateBits & GLS_ATEST_BITS )
		{
		case 0:
			qglDisable( GL_ALPHA_TEST );
			break;
		case GLS_ATEST_GT_0:
			qglEnable( GL_ALPHA_TEST );
			qglAlphaFunc( GL_GREATER, 0.0f );
			break;
		case GLS_ATEST_LT_80:
			qglEnable( GL_ALPHA_TEST );
			qglAlphaFunc( GL_LESS, 0.5f );
			break;
		case GLS_ATEST_GE_80:
			qglEnable( GL_ALPHA_TEST );
			qglAlphaFunc( GL_GEQUAL, 0.5f );
			break;
		case GLS_ATEST_GE_C0:
			qglEnable( GL_ALPHA_TEST );
			qglAlphaFunc( GL_GEQUAL, 0.75f );
			break;
		default:
			assert( 0 );
			break;
		}
	}

	glState.glStateBits = stateBits;
}

void MatrixLoadIdentity(float * matrix)
{
	memset(matrix, 0, sizeof(*matrix)*16);
	matrix[0] = 1;
	matrix[5] = 1;
	matrix[10] = 1;
	matrix[15] = 1;
}

void MatrixLoadScale(float * matrix, float x, float y, float z)
{
	memset(matrix, 0, sizeof(*matrix)*16);
	matrix[0] = x;
	matrix[5] = y;
	matrix[10] = z;
	matrix[15] = 1;
}

void MatrixScale(float * matrix, float x, float y, float z)
{
	const float *a = matrix;
	float a00 = a[0], a01 = a[1], a02 = a[2], a03 = a[3],
		a10 = a[4], a11 = a[5], a12 = a[6], a13 = a[7],
		a20 = a[8], a21 = a[9], a22 = a[10], a23 = a[11],
		a30 = a[12], a31 = a[13], a32 = a[14], a33 = a[15];

	matrix[0] *= x;
	matrix[1] *= x;
	matrix[2] *= x;
	matrix[3] *= x;

	matrix[4] *= y;
	matrix[5] *= y;
	matrix[6] *= y;
	matrix[7] *= y;

	matrix[8] *= z;
	matrix[9] *= z;
	matrix[10] *= z;
	matrix[11] *= z;
}

void MatrixMultiply(float * matrix, const float * first)
{
	const float *a = matrix;
	const float *b = first;
	float a00 = a[0], a01 = a[1], a02 = a[2], a03 = a[3],
		a10 = a[4], a11 = a[5], a12 = a[6], a13 = a[7],
		a20 = a[8], a21 = a[9], a22 = a[10], a23 = a[11],
		a30 = a[12], a31 = a[13], a32 = a[14], a33 = a[15];

	float b0  = b[0], b1 = b[1], b2 = b[2], b3 = b[3];  
	matrix[0] = b0*a00 + b1*a10 + b2*a20 + b3*a30;
	matrix[1] = b0*a01 + b1*a11 + b2*a21 + b3*a31;
	matrix[2] = b0*a02 + b1*a12 + b2*a22 + b3*a32;
	matrix[3] = b0*a03 + b1*a13 + b2*a23 + b3*a33;

	b0 = b[4]; b1 = b[5]; b2 = b[6]; b3 = b[7];
	matrix[4] = b0*a00 + b1*a10 + b2*a20 + b3*a30;
	matrix[5] = b0*a01 + b1*a11 + b2*a21 + b3*a31;
	matrix[6] = b0*a02 + b1*a12 + b2*a22 + b3*a32;
	matrix[7] = b0*a03 + b1*a13 + b2*a23 + b3*a33;

	b0 = b[8]; b1 = b[9]; b2 = b[10]; b3 = b[11];
	matrix[8] = b0*a00 + b1*a10 + b2*a20 + b3*a30;
	matrix[9] = b0*a01 + b1*a11 + b2*a21 + b3*a31;
	matrix[10] = b0*a02 + b1*a12 + b2*a22 + b3*a32;
	matrix[11] = b0*a03 + b1*a13 + b2*a23 + b3*a33;

	b0 = b[12]; b1 = b[13]; b2 = b[14]; b3 = b[15];
	matrix[12] = b0*a00 + b1*a10 + b2*a20 + b3*a30;
	matrix[13] = b0*a01 + b1*a11 + b2*a21 + b3*a31;
	matrix[14] = b0*a02 + b1*a12 + b2*a22 + b3*a32;
	matrix[15] = b0*a03 + b1*a13 + b2*a23 + b3*a33;
}

// scale rotation translation inverse
void MatrixSRTInverse(float * _m)
{
	float temp1, temp2, temp3;
	float tx, ty, tz, tw;
	float sx, sy, sz;

	// get the scale
	sx = 1.f/VectorLength(_m);
	sy = 1.f/VectorLength(_m + 4);
	sz = 1.f/VectorLength(_m + 8);

	// normalize
	VectorScale(_m, sx, _m);
	VectorScale(_m + 4, sy, _m + 4);
	VectorScale(_m + 8, sz, _m + 8);

	// inverse rotation
	temp1 = _m[1];
	temp2 = _m[2];
	temp3 = _m[6];
	_m[1] = _m[4];
	_m[2] = _m[8];
	_m[6] = _m[9];
	_m[4] = temp1;
	_m[8] = temp2;
	_m[9] = temp3;

	// inverse scale
	VectorScale(_m, sx, _m);
	VectorScale(_m + 4, sy, _m + 4);
	VectorScale(_m + 8, sz, _m + 8);

	// inverse translation
	tx = _m[12] * _m[0]
	   + _m[13] * _m[4]
	   + _m[14] * _m[8];
	ty = _m[12] * _m[1]
	   + _m[13] * _m[5]
	   + _m[14] * _m[9];
	tz = _m[12] * _m[2]
	   + _m[13] * _m[6]
	   + _m[14] * _m[10];
	tw = _m[12] * _m[3]
	   + _m[13] * _m[7]
	   + _m[14] * _m[11]
	   + _m[15];

	_m[12] = -tx;
	_m[13] = -ty;
	_m[14] = -tz;
	_m[15] = tw;
}

// rotation translation inverse
void MatrixRTInverse(float * _m)
{
	float temp1, temp2, temp3;
	float tx, ty, tz, tw;

	// inverse rotation
	temp1 = _m[1];
	temp2 = _m[2];
	temp3 = _m[6];
	_m[1] = _m[4];
	_m[2] = _m[8];
	_m[6] = _m[9];
	_m[4] = temp1;
	_m[8] = temp2;
	_m[9] = temp3;

	// inverse translation
	tx = _m[12] * _m[0]
	   + _m[13] * _m[4]
	   + _m[14] * _m[8];
	ty = _m[12] * _m[1]
	   + _m[13] * _m[5]
	   + _m[14] * _m[9];
	tz = _m[12] * _m[2]
	   + _m[13] * _m[6]
	   + _m[14] * _m[10];
	tw = _m[12] * _m[3]
	   + _m[13] * _m[7]
	   + _m[14] * _m[11]
	   + _m[15];

	_m[12] = -tx;
	_m[13] = -ty;
	_m[14] = -tz;
	_m[15] = tw;
}

void MatrixSetProjection(
	float * projectionMatrix,
	float tanFovLeft, float tanFovRight,
	float tanFovBottom, float tanFovTop,
	float zNear, float zFar)
{
	float	xmin, xmax, ymin, ymax;
	float	width, height, depth;

	ymax = tanFovBottom;
	ymin = tanFovTop;
	xmax = tanFovRight;
	xmin = tanFovLeft;
	width = xmax - xmin;
	height = ymax - ymin;
	depth = zFar - zNear;

	projectionMatrix[0] = 2 / width;
	projectionMatrix[4] = 0;
	projectionMatrix[8] = ( xmax + xmin ) / width;	// normally 0
	projectionMatrix[12] = 0;

	projectionMatrix[1] = 0;
	projectionMatrix[5] = 2 / height;
	projectionMatrix[9] = ( ymax + ymin ) / height;	// normally 0
	projectionMatrix[13] = 0;

	projectionMatrix[2] = 0;
	projectionMatrix[6] = 0;
	projectionMatrix[10] = -( zFar + zNear ) / depth;
	projectionMatrix[14] = -2 * zFar * zNear / depth;

	projectionMatrix[3] = 0;
	projectionMatrix[7] = 0;
	projectionMatrix[11] = -1;
	projectionMatrix[15] = 0;
}

void MatrixSetProjectionNearFar(float *projectionMatrix, float zNear, float zFar)
{
	float depth = zFar - zNear;
	projectionMatrix[10] = -( zFar + zNear ) / depth;
	projectionMatrix[14] = -2 * zFar * zNear / depth;
}

extern bool g_vrEnabled;
extern vr::IVRSystem * hmd;
extern float hmdEyeLeft[16];
extern float hmdEyeRight[16];
extern bool g_OVRCompositor;
extern int g_vidWidth;
extern int g_vidHeight;
extern float hmdProjectionLeft[16];
extern float hmdProjectionRight[16];

float hmdHeadMatrix[16];

float vrUIProjectionLeftMatrix[16];
float vrUIProjectionRightMatrix[16];
float vrUIViewLeftMatrix[16];
float vrUIViewRightMatrix[16];

float vrProjectionMatrix[16];
float vrFarViewMatrix[16];
float vrViewLeftMatrix[16];
float vrViewRightMatrix[16];


void VRCreateProjectionMatrix(float * matrix, bool leftEye, float zNear, float zFar)
{
	float *hmdProjection;
	if (leftEye)
	{
		hmdProjection = hmdProjectionLeft;
	}
	else
	{
		hmdProjection = hmdProjectionRight;
	}
	memcpy(matrix, hmdProjection, sizeof(*matrix)*16);
	MatrixSetProjectionNearFar(matrix, zNear, zFar);
}

void VRCreateViewMatrix(float * matrix, bool leftEye, float separation)
{
	float *vrEyeMatrix;
	if (leftEye)
	{
		vrEyeMatrix = hmdEyeLeft;
	}
	else
	{
		vrEyeMatrix = hmdEyeRight;
	}
	if (separation != 0)
	{
		float eyeScale = fabs(vrEyeMatrix[12]) / separation;
		float eyeScaleR = 1.f / eyeScale;
		MatrixLoadScale(matrix, eyeScaleR, eyeScaleR, eyeScaleR);
		MatrixMultiply(matrix, vrEyeMatrix);
		MatrixMultiply(matrix, hmdHeadMatrix);
		MatrixScale(matrix, eyeScale, eyeScale, eyeScale);
	}
	else
	{
		memcpy(matrix, hmdHeadMatrix, sizeof(*matrix)*12);
		matrix[12] = matrix[13] = matrix[14] = 0;
		matrix[15] = 1;
	}
}

static void VRUpdate()
{
	if (!g_vrEnabled)
	{
		return;
	}

	vr::VREvent_t e;
	while( hmd->PollNextEvent( &e, sizeof( e ) ) )
	{
		switch( e.eventType )
		{
		case vr::VREvent_TrackedDeviceActivated:
			{
				Com_Printf( "Device %u attached\n", e.trackedDeviceIndex);
			}
			break;
		case vr::VREvent_TrackedDeviceDeactivated:
			{
				Com_Printf( "Device %u detached\n", e.trackedDeviceIndex);
			}
			break;
		case vr::VREvent_TrackedDeviceUpdated:
			{
				//Com_Printf( "Device %u updated\n", e.trackedDeviceIndex);
			}
			break;
		}
	}

	vr::TrackedDevicePose_t m_rTrackedDevicePose[ vr::k_unMaxTrackedDeviceCount ];
	vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0 );

	if (m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
	{
		vr::HmdMatrix34_t &mat = m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking;
		hmdHeadMatrix[0] = mat.m[0][0];
		hmdHeadMatrix[1] = mat.m[1][0];
		hmdHeadMatrix[2] = mat.m[2][0];
		hmdHeadMatrix[3] = 0;
		hmdHeadMatrix[4] = mat.m[0][1];
		hmdHeadMatrix[5] = mat.m[1][1];
		hmdHeadMatrix[6] = mat.m[2][1];
		hmdHeadMatrix[7] = 0;
		hmdHeadMatrix[8] = mat.m[0][2];
		hmdHeadMatrix[9] = mat.m[1][2];
		hmdHeadMatrix[10] = mat.m[2][2];
		hmdHeadMatrix[11] = 0;
		hmdHeadMatrix[12] = mat.m[0][3];
		hmdHeadMatrix[13] = mat.m[1][3];
		hmdHeadMatrix[14] = mat.m[2][3];
		hmdHeadMatrix[15] = 1;
		MatrixRTInverse(hmdHeadMatrix);
	}
	else
	{
		memset(hmdHeadMatrix, 0, sizeof(hmdHeadMatrix));
		hmdHeadMatrix[0] = 1;
		hmdHeadMatrix[5] = 1;
		hmdHeadMatrix[10] = 1;
		hmdHeadMatrix[15] = 1;
	}

	VRCreateProjectionMatrix(vrUIProjectionLeftMatrix, true, 0.1f, 512);
	VRCreateProjectionMatrix(vrUIProjectionRightMatrix, false, 0.1f, 512);
	VRCreateViewMatrix(vrUIViewLeftMatrix,true,3.2f);
	VRCreateViewMatrix(vrUIViewRightMatrix,false,3.2f);

	VRCreateViewMatrix(vrFarViewMatrix,true,0);
	VRCreateViewMatrix(vrViewLeftMatrix,true,r_stereoSeparation->value);
	VRCreateViewMatrix(vrViewRightMatrix,false,r_stereoSeparation->value);
}

#if 0
bool viewportReset;
int viewportTarget;
int viewportCount;
float viewportOldX;
float viewportOldY;
float viewportOldWidth;
float viewportOldHeight;
#endif
void GL_Viewport( int x, int y, int width, int height ) {
	if (g_vrEnabled)
	{
		qglViewport( 0, 0, glConfig.vidWidth, glConfig.vidHeight);

#if 0
		viewportCount++;
		if (!viewportReset && viewportCount != viewportTarget &&
			viewportOldX == x &&
			viewportOldY == y &&
			viewportOldWidth == width &&
			viewportOldHeight == height)
		{
			return;
		}
		viewportReset = false;
		viewportOldX = x;
		viewportOldY = y;
		viewportOldWidth = width;
		viewportOldHeight = height;
#endif

		float * projectionMatrix;
		float * viewMatrix;
		if (backEnd.stereoLeft)
		{
			projectionMatrix = vrUIProjectionLeftMatrix;
			viewMatrix = vrUIViewLeftMatrix;
		}
		else
		{
			projectionMatrix = vrUIProjectionRightMatrix;
			viewMatrix = vrUIViewRightMatrix;
		}

		qglMatrixMode(GL_PROJECTION);
		qglPushMatrix();
		qglLoadMatrixf(projectionMatrix);

		qglMatrixMode(GL_MODELVIEW);
		qglPushMatrix();
		qglLoadMatrixf(viewMatrix);

		static float UI_scale = 0.1f;
		static float UI_z = -381.36f;
		qglScalef(UI_scale, UI_scale, UI_scale); // scale it down
		qglTranslatef(-320.f, -240.f, UI_z); // center X/Y, offset z

#if 1
		GL_State( GLS_DEPTHTEST_DISABLE );
		qglEnable(GL_STENCIL_TEST);
		qglDisable( GL_DEPTH_TEST );
		qglDisable( GL_CULL_FACE );
		qglDisable( GL_CLIP_PLANE0 );
		qglStencilFunc(GL_ALWAYS, 1, 0xFF);
		qglStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		qglStencilMask(0xFF);
		qglColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		qglDepthMask(GL_FALSE);

		// draw viewport to stencil
		qglClearStencil( 0U );
		qglClear(GL_STENCIL_BUFFER_BIT);
#else
		GL_State( GLS_ALPHA );
		qglDisable( GL_DEPTH_TEST );
		qglDisable( GL_TEXTURE_2D );
		qglDisable( GL_CULL_FACE );
		qglDisable( GL_CLIP_PLANE0 );
		qglColor4f(1,1,0,0.5);
#endif
		qglScalef(640.f/(float)glConfig.vidWidth, 480.f/(float)glConfig.vidHeight, 1);
		qglBegin(GL_TRIANGLE_FAN);
		qglVertex2f(x, y+height);
		qglVertex2f(x+width, y+height);
		qglVertex2f(x+width, y);
		qglVertex2f(x, y);
		qglEnd();

#if 1
		qglStencilFunc(GL_EQUAL, 1, 0xFF);
		qglStencilMask(0x00);
		qglColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		qglDepthMask(GL_TRUE);
#else
		qglEnable( GL_TEXTURE_2D );
#endif

		qglPopMatrix();
		qglMatrixMode(GL_PROJECTION);
		qglPopMatrix();
		qglMatrixMode(GL_MODELVIEW);
	}
	else
	{
		// set the window clipping
		qglViewport( 0, 0, glConfig.vidWidth, glConfig.vidHeight);
		qglScissor( x, y, width, height );
	}
}

extern GLuint  g_fbo[3];
extern GLuint  g_colorBuffer[3];

void GL_DrawBuffer( int buffer ) {
#if 0
	viewportReset = true;
	viewportCount = 0;
#endif
	if (glConfig.stereoEnabled == 2)
	{
		qglDrawBuffer(GL_BACK);
		if (buffer == QGL_BACK_LEFT)
		{
			qglBindFramebuffer(GL_FRAMEBUFFER, g_fbo[0]);
			backEnd.stereoLeft = qtrue;
			backEnd.projection2D = qfalse;
		}
		else if (buffer == QGL_BACK_RIGHT)
		{
			qglBindFramebuffer(GL_FRAMEBUFFER, g_fbo[1]);
			backEnd.stereoLeft = qfalse;
			backEnd.projection2D = qfalse;
		}
		else if (buffer == QGL_OFFSCREEN_LEFT)
		{
			qglBindFramebuffer(GL_FRAMEBUFFER, g_fbo[2]);
			backEnd.stereoLeft = qtrue;
			backEnd.projection2D = qfalse;
		}
		else if (buffer == QGL_OFFSCREEN_RIGHT)
		{
			qglBindFramebuffer(GL_FRAMEBUFFER, g_fbo[2]);
			backEnd.stereoLeft = qfalse;
			backEnd.projection2D = qfalse;
		}
		else
		{
			qglBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}
	else
	{
		qglDrawBuffer(GL_BACK);
		if (buffer == QGL_OFFSCREEN)
		{
			qglBindFramebuffer(GL_FRAMEBUFFER, g_fbo[2]);
		}
		else
		{
			qglBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}
}

void GL_Present( void )
{
	if (glConfig.stereoEnabled == 2)
	{
		GLint viewport[4];
		qglGetIntegerv(GL_VIEWPORT, viewport);

		qglBindFramebuffer(GL_FRAMEBUFFER, 0);
		qglViewport( 0, 0, glConfig.vidWidth, glConfig.vidHeight );

		qglDisable(GL_STENCIL_TEST);
		qglDisable(GL_SCISSOR_TEST);

		qglDisable (GL_CLIP_PLANE0);
		GL_Cull( CT_TWO_SIDED );

		// Go into orthographic 2d mode.
		qglMatrixMode(GL_PROJECTION);
		qglPushMatrix();
		qglLoadIdentity();
		qglOrtho(0, 1, 1, 0, -1, 1);
		qglMatrixMode(GL_MODELVIEW);
		qglPushMatrix();
		qglLoadIdentity();

		GL_State(GLS_DEPTHTEST_DISABLE);

		GLint texture;
		qglGetIntegerv(GL_TEXTURE_BINDING_2D, &texture);

		if (g_OVRCompositor)
		{
			vr::Texture_t leftEyeTexture = {(void*)g_colorBuffer[0], vr::API_OpenGL, vr::ColorSpace_Gamma };
			vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture );
			vr::Texture_t rightEyeTexture = {(void*)g_colorBuffer[1], vr::API_OpenGL, vr::ColorSpace_Gamma };
			vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture );
			vr::VRCompositor()->PostPresentHandoff();
		}

		if (g_vrEnabled)
		{
			qglViewport( 0, 0, g_vidWidth, g_vidHeight );
		}

		// left
		qglBindTexture( GL_TEXTURE_2D, g_colorBuffer[0] );
		qglBegin(GL_QUADS);
			qglColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
			qglTexCoord2f( 0, 1.f );
			qglVertex2f( 0, 0 );

			qglTexCoord2f( 0, 0 );
			qglVertex2f( 0, 1 );

			qglTexCoord2f( 1.f, 0 );
			qglVertex2f( 0.5f, 1 );

			qglTexCoord2f( 1.f, 1.f );
			qglVertex2f( 0.5f, 0 );
		qglEnd();

		// right
		qglBindTexture( GL_TEXTURE_2D, g_colorBuffer[1] );
		qglBegin(GL_QUADS);
			qglColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
			qglTexCoord2f( 0, 1.f );
			qglVertex2f( 0.5f, 0 );

			qglTexCoord2f( 0, 0 );
			qglVertex2f( 0.5f, 1 );

			qglTexCoord2f( 1.f, 0 );
			qglVertex2f( 1, 1 );

			qglTexCoord2f( 1.f, 1.f );
			qglVertex2f( 1, 0 );
		qglEnd();

		qglBindTexture( GL_TEXTURE_2D, texture );

		qglMatrixMode(GL_PROJECTION);
		qglPopMatrix();
		qglMatrixMode(GL_MODELVIEW);
		qglPopMatrix();

		qglViewport(viewport[0],viewport[1],viewport[2],viewport[3]);
	}

	ri.WIN_Present(&window);

	if (g_OVRCompositor)
	{
		VRUpdate();
	}
}


/*
================
RB_Hyperspace

A player has predicted a teleport, but hasn't arrived yet
================
*/
static void RB_Hyperspace( void ) {
	float		c;

	if ( !backEnd.isHyperspace ) {
		// do initialization shit
	}

	c = ( backEnd.refdef.time & 255 ) / 255.0f;
	qglClearColor( c, c, c, 1 );
	qglClear( GL_COLOR_BUFFER_BIT );
	backEnd.needPresent = qtrue;

	backEnd.isHyperspace = qtrue;
}

void SetViewportAndScissor( void ) {
	qglMatrixMode(GL_PROJECTION);
	if (g_vrEnabled)
	{
		GL_Viewport(backEnd.viewParms.viewportX, backEnd.viewParms.viewportY,
			backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight);

		float eyeScaleR = r_stereoSeparation->value / hmdEyeRight[12];
		float zOffset = hmdHeadMatrix[14] * eyeScaleR;
		if( zOffset < 0 ) zOffset = 0;

		VRCreateProjectionMatrix(vrProjectionMatrix, backEnd.stereoLeft == qtrue, r_znear->value + zOffset, backEnd.viewParms.zFar + zOffset);
		qglMatrixMode(GL_PROJECTION);
		qglLoadMatrixf( vrProjectionMatrix );

		float * viewMatrix;
		if (backEnd.refdef.rdflags & RDF_STEREO_FAR)
		{
			viewMatrix = vrFarViewMatrix;
		}
		else// if (backEnd.refdef.rdflags & RDF_STEREO)
		{
			if (backEnd.refdef.rdflags & RDF_UI)
			{
				if (backEnd.stereoLeft)
				{
					viewMatrix = vrUIViewLeftMatrix;
				}
				else
				{
					viewMatrix = vrUIViewRightMatrix;
				}
			}
			else
			{
				if (backEnd.stereoLeft)
				{
					viewMatrix = vrViewLeftMatrix;
				}
				else
				{
					viewMatrix = vrViewRightMatrix;
				}
			}
		}
		qglMultMatrixf(viewMatrix);

		// zoom scale
		if (!(backEnd.refdef.rdflags & RDF_UI))
		{
			float defaultTanX = tanf((float)(80*M_PI/360));
			float tanX = tanf(backEnd.refdef.fov_x * M_PI/360.f);
			float scale = defaultTanX / tanX;
			qglScalef(scale, scale, 1);
		}

		qglMatrixMode(GL_MODELVIEW);
		qglLoadIdentity();

		GL_StateOverride(0,0);

		return;
	}
	
	if (glConfig.stereoEnabled)
	{
		qglLoadIdentity();
		if (backEnd.refdef.rdflags & (RDF_STEREO|RDF_STEREO_FAR))
		{
			float tvStereoOffset = r_stereoEyeDistance->value / r_stereoDisplayWidth->value;
			if (backEnd.stereoLeft)
			{
				qglTranslatef(-tvStereoOffset, 0.f, 0.f);
			}
			else
			{
				qglTranslatef(tvStereoOffset, 0.f, 0.f);
			}
		}

		if (backEnd.refdef.rdflags & RDF_UI)
		{
			float zNear = r_znear->value;
			float zFar	= backEnd.viewParms.zFar;
			float scalex = r_stereoDisplayWidth->value *0.5f / r_stereoDisplayDistance->value;
			float scaley = scalex * 480.f / 640.f;
			float hwidth = zNear * scalex;
			float hheight = zNear * scaley;
			qglFrustum(-hwidth,hwidth,-hheight,hheight,zNear,zFar);
		}
		else
		{
			qglMultMatrixf( backEnd.viewParms.projectionMatrix );
		}

		if (backEnd.refdef.rdflags & RDF_STEREO)
		{
			float separation = r_stereoSeparation->value/2;
			if (backEnd.stereoLeft)
			{
				qglTranslatef(separation, 0.f, 0.f);
			}
			else
			{
				qglTranslatef(-separation, 0.f, 0.f);
			}
		}
	}
	else
	{
		qglLoadMatrixf( backEnd.viewParms.projectionMatrix );
	}
	qglMatrixMode(GL_MODELVIEW);
	qglLoadIdentity();

	// set the window clipping
	GL_Viewport(backEnd.viewParms.viewportX, backEnd.viewParms.viewportY,
		backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight);
}

/*
=================
RB_BeginDrawingView

Any mirrored or portaled views have already been drawn, so prepare
to actually render the visible surfaces for this view
=================
*/
static void RB_BeginDrawingView (void) {
	int clearBits = GL_DEPTH_BUFFER_BIT;

	// sync with gl if needed
	if ( r_finish->integer == 1 && !glState.finishCalled ) {
		qglFinish ();
		glState.finishCalled = qtrue;
	}
	if ( r_finish->integer == 0 ) {
		glState.finishCalled = qtrue;
	}

	// we will need to change the projection matrix before drawing
	// 2D images again
	backEnd.projection2D = qfalse;

	//
	// set the modelview matrix for the viewer
	//
	SetViewportAndScissor();

	// ensures that depth writes are enabled for the depth clear
	GL_State( GLS_DEFAULT );

	// clear relevant buffers
	if ( r_measureOverdraw->integer || r_shadows->integer == 2 || tr_stencilled )
	{
		clearBits |= GL_STENCIL_BUFFER_BIT;
		tr_stencilled = false;
	}

	if (skyboxportal)
	{
		if ( backEnd.refdef.rdflags & RDF_SKYBOXPORTAL )
		{	// portal scene, clear whatever is necessary
			if (r_fastsky->integer || (backEnd.refdef.rdflags & RDF_NOWORLDMODEL) )
			{	// fastsky: clear color
				// try clearing first with the portal sky fog color, then the world fog color, then finally a default
				clearBits |= GL_COLOR_BUFFER_BIT;
				if (tr.world && tr.world->globalFog != -1)
				{
					const fog_t		*fog = &tr.world->fogs[tr.world->globalFog];
					qglClearColor(fog->parms.color[0],  fog->parms.color[1], fog->parms.color[2], 1.0f );
				}
				else
				{
					qglClearColor ( 0.3f, 0.3f, 0.3f, 1.0 );
				}
			}
		}
	}
	else
	{
		if ( r_fastsky->integer && !( backEnd.refdef.rdflags & RDF_NOWORLDMODEL ) && !g_bRenderGlowingObjects )
		{
			if (tr.world && tr.world->globalFog != -1)
			{
				const fog_t		*fog = &tr.world->fogs[tr.world->globalFog];
				qglClearColor(fog->parms.color[0],  fog->parms.color[1], fog->parms.color[2], 1.0f );
			}
			else
			{
				qglClearColor( 0.3f, 0.3f, 0.3f, 1 );	// FIXME: get color of sky
			}
			clearBits |= GL_COLOR_BUFFER_BIT;	// FIXME: only if sky shaders have been used
		}
	}

	if ( !( backEnd.refdef.rdflags & RDF_NOWORLDMODEL ) && ( r_DynamicGlow->integer && !g_bRenderGlowingObjects ) )
	{
		if (tr.world && tr.world->globalFog != -1)
		{ //this is because of a bug in multiple scenes I think, it needs to clear for the second scene but it doesn't normally.
			const fog_t		*fog = &tr.world->fogs[tr.world->globalFog];

			qglClearColor(fog->parms.color[0],  fog->parms.color[1], fog->parms.color[2], 1.0f );
			clearBits |= GL_COLOR_BUFFER_BIT;
		}
	}
	// If this pass is to just render the glowing objects, don't clear the depth buffer since
	// we're sharing it with the main scene (since the main scene has already been rendered). -AReis
	if ( g_bRenderGlowingObjects )
	{
		clearBits &= ~GL_DEPTH_BUFFER_BIT;
	}

	if (clearBits)
	{
		qglClear( clearBits );
		backEnd.needPresent = qtrue;
	}

	if ( ( backEnd.refdef.rdflags & RDF_HYPERSPACE ) )
	{
		RB_Hyperspace();
		return;
	}
	else
	{
		backEnd.isHyperspace = qfalse;
	}

	glState.faceCulling = -1;		// force face culling to set next time

	// we will only draw a sun if there was sky rendered in this view
	backEnd.skyRenderedThisView = qfalse;

	// clip to the plane of the portal
	if ( backEnd.viewParms.isPortal ) {
		float	plane[4];
		double	plane2[4];

		plane[0] = backEnd.viewParms.portalPlane.normal[0];
		plane[1] = backEnd.viewParms.portalPlane.normal[1];
		plane[2] = backEnd.viewParms.portalPlane.normal[2];
		plane[3] = backEnd.viewParms.portalPlane.dist;

		plane2[0] = DotProduct (backEnd.viewParms.ori.axis[0], plane);
		plane2[1] = DotProduct (backEnd.viewParms.ori.axis[1], plane);
		plane2[2] = DotProduct (backEnd.viewParms.ori.axis[2], plane);
		plane2[3] = DotProduct (plane, backEnd.viewParms.ori.origin) - plane[3];

		qglLoadMatrixf( s_flipMatrix );
		qglClipPlane (GL_CLIP_PLANE0, plane2);
		qglEnable (GL_CLIP_PLANE0);
	} else {
		qglDisable (GL_CLIP_PLANE0);
	}
}

#define	MAC_EVENT_PUMP_MSEC		5

//used by RF_DISTORTION
static inline bool R_WorldCoordToScreenCoordFloat(vec3_t worldCoord, float *x, float *y)
{
	int	xcenter, ycenter;
	vec3_t	local, transformed;
	vec3_t	vfwd;
	vec3_t	vright;
	vec3_t	vup;
	float xzi;
	float yzi;

	xcenter = glConfig.vidWidth / 2;
	ycenter = glConfig.vidHeight / 2;

	//AngleVectors (tr.refdef.viewangles, vfwd, vright, vup);
	VectorCopy(tr.refdef.viewaxis[0], vfwd);
	VectorCopy(tr.refdef.viewaxis[1], vright);
	VectorCopy(tr.refdef.viewaxis[2], vup);

	VectorSubtract (worldCoord, tr.refdef.vieworg, local);

	transformed[0] = DotProduct(local,vright);
	transformed[1] = DotProduct(local,vup);
	transformed[2] = DotProduct(local,vfwd);

	// Make sure Z is not negative.
	if(transformed[2] < 0.01)
	{
		return false;
	}

	xzi = xcenter / transformed[2] * (90.0/tr.refdef.fov_x);
	yzi = ycenter / transformed[2] * (90.0/tr.refdef.fov_y);

	*x = xcenter + xzi * transformed[0];
	*y = ycenter - yzi * transformed[1];

	return true;
}

//used by RF_DISTORTION
static inline bool R_WorldCoordToScreenCoord( vec3_t worldCoord, int *x, int *y )
{
	float	xF, yF;
	bool retVal = R_WorldCoordToScreenCoordFloat( worldCoord, &xF, &yF );
	*x = (int)xF;
	*y = (int)yF;
	return retVal;
}

/*
==================
RB_RenderDrawSurfList
==================
*/
//number of possible surfs we can postrender.
//note that postrenders lack much of the optimization that the standard sort-render crap does,
//so it's slower.
#define MAX_POST_RENDERS	128

typedef struct
{
	int			fogNum;
	int			entNum;
	int			dlighted;
	int			depthRange;
	drawSurf_t	*drawSurf;
	shader_t	*shader;
} postRender_t;

static postRender_t g_postRenders[MAX_POST_RENDERS];
static int g_numPostRenders = 0;

void RB_RenderDrawSurfList( drawSurf_t *drawSurfs, int numDrawSurfs ) {
	shader_t		*shader, *oldShader;
	int				fogNum, oldFogNum;
	int				entityNum, oldEntityNum;
	int				dlighted, oldDlighted;
	int				depthRange, oldDepthRange;
	int				i;
	drawSurf_t		*drawSurf;
	unsigned int	oldSort;
	float			originalTime;
	trRefEntity_t	*curEnt;
	postRender_t	*pRender;
	bool			didShadowPass = false;

	if (g_bRenderGlowingObjects)
	{ //only shadow on initial passes
		didShadowPass = true;
	}

	// save original time for entity shader offsets
	originalTime = backEnd.refdef.floatTime;

	// clear the z buffer, set the modelview, etc
	RB_BeginDrawingView ();

	// draw everything
	oldEntityNum = -1;
	backEnd.currentEntity = &tr.worldEntity;
	oldShader = NULL;
	oldFogNum = -1;
	oldDepthRange = qfalse;
	oldDlighted = qfalse;
	oldSort = (unsigned int) -1;
	depthRange = qfalse;

	backEnd.pc.c_surfaces += numDrawSurfs;

	for (i = 0, drawSurf = drawSurfs ; i < numDrawSurfs ; i++, drawSurf++) {
		if ( drawSurf->sort == oldSort ) {
			// fast path, same as previous sort
			rb_surfaceTable[ *drawSurf->surface ]( drawSurf->surface );
			continue;
		}
		R_DecomposeSort( drawSurf->sort, &entityNum, &shader, &fogNum, &dlighted );

		// If we're rendering glowing objects, but this shader has no stages with glow, skip it!
		if ( g_bRenderGlowingObjects && !shader->hasGlow )
		{
			shader = oldShader;
			entityNum = oldEntityNum;
			fogNum = oldFogNum;
			dlighted = oldDlighted;
			continue;
		}

		oldSort = drawSurf->sort;

		//
		// change the tess parameters if needed
		// a "entityMergable" shader is a shader that can have surfaces from seperate
		// entities merged into a single batch, like smoke and blood puff sprites
		if (entityNum != REFENTITYNUM_WORLD &&
			g_numPostRenders < MAX_POST_RENDERS)
		{
			if ( (backEnd.refdef.entities[entityNum].e.renderfx & RF_DISTORTION)/* ||
				(backEnd.refdef.entities[entityNum].e.renderfx & RF_FORCE_ENT_ALPHA)*/)
				//not sure if we need this alpha fix for sp or not, leaving it out for now -rww
			{ //must render last
				curEnt = &backEnd.refdef.entities[entityNum];
				pRender = &g_postRenders[g_numPostRenders];

				g_numPostRenders++;

				depthRange = 0;
				//figure this stuff out now and store it
				if ( curEnt->e.renderfx & RF_NODEPTH )
				{
					depthRange = 2;
				}
				else if ( curEnt->e.renderfx & RF_DEPTHHACK )
				{
					depthRange = 1;
				}
				pRender->depthRange = depthRange;

				//It is not necessary to update the old* values because
				//we are not updating now with the current values.
				depthRange = oldDepthRange;

				//store off the ent num
				pRender->entNum = entityNum;

				//remember the other values necessary for rendering this surf
				pRender->drawSurf = drawSurf;
				pRender->dlighted = dlighted;
				pRender->fogNum = fogNum;
				pRender->shader = shader;

				//assure the info is back to the last set state
				shader = oldShader;
				entityNum = oldEntityNum;
				fogNum = oldFogNum;
				dlighted = oldDlighted;

				oldSort = (unsigned int)-1; //invalidate this thing, cause we may want to postrender more surfs of the same sort

				//continue without bothering to begin a draw surf
				continue;
			}
		}

		if (shader != oldShader || fogNum != oldFogNum || dlighted != oldDlighted
			|| ( entityNum != oldEntityNum && !shader->entityMergable ) )
		{
			if (oldShader != NULL) {
				RB_EndSurface();

				if (!didShadowPass && shader && shader->sort > SS_BANNER)
				{
					RB_ShadowFinish();
					didShadowPass = true;
				}
			}
			RB_BeginSurface( shader, fogNum );
			oldShader = shader;
			oldFogNum = fogNum;
			oldDlighted = dlighted;
		}

		//
		// change the modelview matrix if needed
		//
		if ( entityNum != oldEntityNum ) {
			depthRange = qfalse;

			if ( entityNum != REFENTITYNUM_WORLD ) {
				backEnd.currentEntity = &backEnd.refdef.entities[entityNum];
				backEnd.refdef.floatTime = originalTime - backEnd.currentEntity->e.shaderTime;

				// set up the transformation matrix
				R_RotateForEntity( backEnd.currentEntity, &backEnd.viewParms, &backEnd.ori );

				// set up the dynamic lighting if needed
				if ( backEnd.currentEntity->needDlights ) {
					R_TransformDlights( backEnd.refdef.num_dlights, backEnd.refdef.dlights, &backEnd.ori );
				}

				if ( backEnd.currentEntity->e.renderfx & RF_NODEPTH ) {
					// No depth at all, very rare but some things for seeing through walls
					depthRange = 2;
				}
				else if ( backEnd.currentEntity->e.renderfx & RF_DEPTHHACK ) {
					// hack the depth range to prevent view model from poking into walls
					depthRange = qtrue;
				}
			} else {
				backEnd.currentEntity = &tr.worldEntity;
				backEnd.refdef.floatTime = originalTime;
				backEnd.ori = backEnd.viewParms.world;
				R_TransformDlights( backEnd.refdef.num_dlights, backEnd.refdef.dlights, &backEnd.ori );
			}

			qglLoadMatrixf( backEnd.ori.modelMatrix );

			//
			// change depthrange if needed
			//
			if ( oldDepthRange != depthRange ) {
				switch ( depthRange ) {
					default:
					case 0:
						qglDepthRange (0, 1);
						break;

					case 1:
						qglDepthRange (0, .3);
						break;

					case 2:
						qglDepthRange (0, 0);
						break;
				}

				oldDepthRange = depthRange;
			}

			oldEntityNum = entityNum;
		}

		// add the triangles for this surface
		rb_surfaceTable[ *drawSurf->surface ]( drawSurf->surface );
	}

	// draw the contents of the last shader batch
	if (oldShader != NULL) {
		RB_EndSurface();
	}

	if (tr_stencilled && tr_distortionPrePost)
	{ //ok, cap it now
		RB_CaptureScreenImage();
		RB_DistortionFill();
	}

	//render distortion surfs (or anything else that needs to be post-rendered)
	if (g_numPostRenders > 0)
	{
		int lastPostEnt = -1;

		while (g_numPostRenders > 0)
		{
			g_numPostRenders--;
			pRender = &g_postRenders[g_numPostRenders];

			RB_BeginSurface( pRender->shader, pRender->fogNum );

			backEnd.currentEntity = &backEnd.refdef.entities[pRender->entNum];

			backEnd.refdef.floatTime = originalTime - backEnd.currentEntity->e.shaderTime;

			// set up the transformation matrix
			R_RotateForEntity( backEnd.currentEntity, &backEnd.viewParms, &backEnd.ori );

			// set up the dynamic lighting if needed
			if ( backEnd.currentEntity->needDlights )
			{
				R_TransformDlights( backEnd.refdef.num_dlights, backEnd.refdef.dlights, &backEnd.ori );
			}

			qglLoadMatrixf( backEnd.ori.modelMatrix );

			depthRange = pRender->depthRange;
			switch ( depthRange )
			{
				default:
				case 0:
					qglDepthRange (0, 1);
					break;

				case 1:
					qglDepthRange (0, .3);
					break;

				case 2:
					qglDepthRange (0, 0);
					break;
			}

			if ((backEnd.currentEntity->e.renderfx & RF_DISTORTION) &&
				lastPostEnt != pRender->entNum)
			{ //do the capture now, we only need to do it once per ent
				int x, y;
				int rad = backEnd.currentEntity->e.radius;
				//We are going to just bind this, and then the CopyTexImage is going to
				//stomp over this texture num in texture memory.
				GL_Bind( tr.screenImage );

				if (R_WorldCoordToScreenCoord( backEnd.currentEntity->e.origin, &x, &y ))
				{
					int cX, cY;
					cX = glConfig.vidWidth-x-(rad/2);
					cY = glConfig.vidHeight-y-(rad/2);

					if (cX+rad > glConfig.vidWidth)
					{ //would it go off screen?
						cX = glConfig.vidWidth-rad;
					}
					else if (cX < 0)
					{ //cap it off at 0
						cX = 0;
					}

					if (cY+rad > glConfig.vidHeight)
					{ //would it go off screen?
						cY = glConfig.vidHeight-rad;
					}
					else if (cY < 0)
					{ //cap it off at 0
						cY = 0;
					}

					//now copy a portion of the screen to this texture
					qglCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, cX, cY, rad, rad, 0);

					lastPostEnt = pRender->entNum;
				}
			}

			rb_surfaceTable[ *pRender->drawSurf->surface ]( pRender->drawSurf->surface );
			RB_EndSurface();
		}
	}

	// go back to the world modelview matrix
	qglLoadMatrixf( backEnd.viewParms.world.modelMatrix );
	if ( depthRange ) {
		qglDepthRange (0, 1);
	}

#if 0
	RB_DrawSun();
#endif
	if (tr_stencilled && !tr_distortionPrePost)
	{ //draw in the stencil buffer's cutout
		RB_DistortionFill();
	}
	if (!didShadowPass)
	{
		// darken down any stencil shadows
		RB_ShadowFinish();
		didShadowPass = true;
	}

// add light flares on lights that aren't obscured
//	RB_RenderFlares();
}


/*
============================================================================

RENDER BACK END FUNCTIONS

============================================================================
*/

/*
================
RB_SetGL2D

================
*/
void	RB_SetGL2D (void) {
	backEnd.projection2D = qtrue;

	// set 2D virtual screen size
	if (g_vrEnabled)
	{
		float * projectionMatrix;
		float * viewMatrix;
		if (backEnd.stereoLeft)
		{
			projectionMatrix = vrUIProjectionLeftMatrix;
			viewMatrix = vrUIViewLeftMatrix;
		}
		else
		{
			projectionMatrix = vrUIProjectionRightMatrix;
			viewMatrix = vrUIViewRightMatrix;
		}

		qglMatrixMode(GL_PROJECTION);
		qglLoadMatrixf(projectionMatrix);

		qglMatrixMode(GL_MODELVIEW);
		qglLoadMatrixf(viewMatrix);

		static float UI_scale = 0.1f;
		static float UI_z = -381.36f;
		qglScalef(UI_scale, UI_scale, UI_scale); // scale it down
		qglTranslatef(-320.f, 240.f, UI_z); // center X/Y, offset z
		qglScalef(1.f, -1.f, 1.f); // flip Y axis
	}
	else
	{
		qglMatrixMode(GL_PROJECTION);
		qglLoadIdentity ();
		qglOrtho (0, 640, 480, 0, 0, 1);
		qglMatrixMode(GL_MODELVIEW);
		qglLoadIdentity ();
	}

	GL_Viewport(0, 0, glConfig.vidWidth, glConfig.vidHeight);

	GL_StateOverride( GLS_DEPTHTEST_DISABLE, GLS_DEPTHTEST_DISABLE);
	GL_State( GLS_ALPHA );

	qglDisable( GL_CULL_FACE );
	qglDisable( GL_CLIP_PLANE0 );

	// set time for 2D shaders
	backEnd.refdef.time = ri.Milliseconds();
	backEnd.refdef.floatTime = backEnd.refdef.time * 0.001f;
}


/*
=============
RB_SetColor

=============
*/
const void	*RB_SetColor( const void *data ) {
	const setColorCommand_t	*cmd;

	cmd = (const setColorCommand_t *)data;

	backEnd.color2D[0] = cmd->color[0] * 255;
	backEnd.color2D[1] = cmd->color[1] * 255;
	backEnd.color2D[2] = cmd->color[2] * 255;
	backEnd.color2D[3] = cmd->color[3] * 255;

	return (const void *)(cmd + 1);
}

/*
=============
RB_StretchPic
=============
*/
const void *RB_StretchPic ( const void *data ) {
	const stretchPicCommand_t	*cmd;
	shader_t *shader;
	int		numVerts, numIndexes;

	cmd = (const stretchPicCommand_t *)data;

	if ( !backEnd.projection2D ) {
		RB_SetGL2D();
	}

	shader = cmd->shader;
	if ( shader != tess.shader ) {
		if ( tess.numIndexes ) {
			RB_EndSurface();
		}
		backEnd.currentEntity = &backEnd.entity2D;
		RB_BeginSurface( shader, 0 );
	}

	RB_CHECKOVERFLOW( 4, 6 );
	numVerts = tess.numVertexes;
	numIndexes = tess.numIndexes;

	tess.numVertexes += 4;
	tess.numIndexes += 6;

	tess.indexes[ numIndexes ] = numVerts + 3;
	tess.indexes[ numIndexes + 1 ] = numVerts + 0;
	tess.indexes[ numIndexes + 2 ] = numVerts + 2;
	tess.indexes[ numIndexes + 3 ] = numVerts + 2;
	tess.indexes[ numIndexes + 4 ] = numVerts + 0;
	tess.indexes[ numIndexes + 5 ] = numVerts + 1;

	byteAlias_t *baDest = NULL, *baSource = (byteAlias_t *)&backEnd.color2D;
	baDest = (byteAlias_t *)&tess.vertexColors[numVerts + 0]; baDest->ui = baSource->ui;
	baDest = (byteAlias_t *)&tess.vertexColors[numVerts + 1]; baDest->ui = baSource->ui;
	baDest = (byteAlias_t *)&tess.vertexColors[numVerts + 2]; baDest->ui = baSource->ui;
	baDest = (byteAlias_t *)&tess.vertexColors[numVerts + 3]; baDest->ui = baSource->ui;

	tess.xyz[ numVerts ][0] = cmd->x;
	tess.xyz[ numVerts ][1] = cmd->y;
	tess.xyz[ numVerts ][2] = 0;

	tess.texCoords[ numVerts ][0][0] = cmd->s1;
	tess.texCoords[ numVerts ][0][1] = cmd->t1;

	tess.xyz[ numVerts + 1 ][0] = cmd->x + cmd->w;
	tess.xyz[ numVerts + 1 ][1] = cmd->y;
	tess.xyz[ numVerts + 1 ][2] = 0;

	tess.texCoords[ numVerts + 1 ][0][0] = cmd->s2;
	tess.texCoords[ numVerts + 1 ][0][1] = cmd->t1;

	tess.xyz[ numVerts + 2 ][0] = cmd->x + cmd->w;
	tess.xyz[ numVerts + 2 ][1] = cmd->y + cmd->h;
	tess.xyz[ numVerts + 2 ][2] = 0;

	tess.texCoords[ numVerts + 2 ][0][0] = cmd->s2;
	tess.texCoords[ numVerts + 2 ][0][1] = cmd->t2;

	tess.xyz[ numVerts + 3 ][0] = cmd->x;
	tess.xyz[ numVerts + 3 ][1] = cmd->y + cmd->h;
	tess.xyz[ numVerts + 3 ][2] = 0;

	tess.texCoords[ numVerts + 3 ][0][0] = cmd->s1;
	tess.texCoords[ numVerts + 3 ][0][1] = cmd->t2;

	return (const void *)(cmd + 1);
}


/*
=============
RB_StretchScratch
=============
*/
const void *RB_StretchScratch ( const void *data ) {
	const stretchScratchCommand_t	*cmd;

	cmd = (const stretchScratchCommand_t *)data;

	if ( !backEnd.projection2D ) {
		RB_SetGL2D();
	}

	if ( tess.numIndexes ) {
		RB_EndSurface();
	}

	qglPushMatrix();

	qglTranslatef(320.f,240.f,381.f);
	qglScalef(9.f, 9.f, 9.f);
	qglTranslatef(-320.f,-240.f,-381.f);

	GL_Bind( tr.scratchImage[cmd->iClient] );

	qglColor3f( tr.identityLight, tr.identityLight, tr.identityLight );

	qglBegin (GL_QUADS);
	qglTexCoord2f ( 0.5f / cmd->cols,  0.5f / cmd->rows );
	qglVertex2f (cmd->x, cmd->y);
	qglTexCoord2f ( ( cmd->cols - 0.5f ) / cmd->cols ,  0.5f / cmd->rows );
	qglVertex2f (cmd->x+cmd->w, cmd->y);
	qglTexCoord2f ( ( cmd->cols - 0.5f ) / cmd->cols, ( cmd->rows - 0.5f ) / cmd->rows );
	qglVertex2f (cmd->x+cmd->w, cmd->y+cmd->h);
	qglTexCoord2f ( 0.5f / cmd->cols, ( cmd->rows - 0.5f ) / cmd->rows );
	qglVertex2f (cmd->x, cmd->y+cmd->h);
	qglEnd ();
	backEnd.needPresent = qtrue;

	qglPopMatrix();

	return (const void *)(cmd + 1);
}


/*
=============
RB_RotatePic
=============
*/
const void *RB_RotatePic ( const void *data )
{
	const rotatePicCommand_t	*cmd;
	shader_t *shader;

	cmd = (const rotatePicCommand_t *)data;

	shader = cmd->shader;

	if ( !backEnd.projection2D ) {
		RB_SetGL2D();
	}

	shader = cmd->shader;
	if ( shader != tess.shader ) {
		if ( tess.numIndexes ) {
			RB_EndSurface();
		}
		backEnd.currentEntity = &backEnd.entity2D;
		RB_BeginSurface( shader, 0 );
	}

	RB_CHECKOVERFLOW( 4, 6 );
	int numVerts = tess.numVertexes;
	int numIndexes = tess.numIndexes;

	float angle = DEG2RAD( cmd-> a );
	float s = sinf( angle );
	float c = cosf( angle );

	matrix3_t m = {
		{ c, s, 0.0f },
		{ -s, c, 0.0f },
		{ cmd->x + cmd->w, cmd->y, 1.0f }
	};

	tess.numVertexes += 4;
	tess.numIndexes += 6;

	tess.indexes[ numIndexes ] = numVerts + 3;
	tess.indexes[ numIndexes + 1 ] = numVerts + 0;
	tess.indexes[ numIndexes + 2 ] = numVerts + 2;
	tess.indexes[ numIndexes + 3 ] = numVerts + 2;
	tess.indexes[ numIndexes + 4 ] = numVerts + 0;
	tess.indexes[ numIndexes + 5 ] = numVerts + 1;

	byteAlias_t *baDest = NULL, *baSource = (byteAlias_t *)&backEnd.color2D;
	baDest = (byteAlias_t *)&tess.vertexColors[numVerts + 0]; baDest->ui = baSource->ui;
	baDest = (byteAlias_t *)&tess.vertexColors[numVerts + 1]; baDest->ui = baSource->ui;
	baDest = (byteAlias_t *)&tess.vertexColors[numVerts + 2]; baDest->ui = baSource->ui;
	baDest = (byteAlias_t *)&tess.vertexColors[numVerts + 3]; baDest->ui = baSource->ui;

	tess.xyz[ numVerts ][0] = m[0][0] * (-cmd->w) + m[2][0];
	tess.xyz[ numVerts ][1] = m[0][1] * (-cmd->w) + m[2][1];
	tess.xyz[ numVerts ][2] = 0;

	tess.texCoords[ numVerts ][0][0] = cmd->s1;
	tess.texCoords[ numVerts ][0][1] = cmd->t1;

	tess.xyz[ numVerts + 1 ][0] = m[2][0];
	tess.xyz[ numVerts + 1 ][1] = m[2][1];
	tess.xyz[ numVerts + 1 ][2] = 0;

	tess.texCoords[ numVerts + 1 ][0][0] = cmd->s2;
	tess.texCoords[ numVerts + 1 ][0][1] = cmd->t1;

	tess.xyz[ numVerts + 2 ][0] = m[1][0] * (cmd->h) + m[2][0];
	tess.xyz[ numVerts + 2 ][1] = m[1][1] * (cmd->h) + m[2][1];
	tess.xyz[ numVerts + 2 ][2] = 0;

	tess.texCoords[ numVerts + 2 ][0][0] = cmd->s2;
	tess.texCoords[ numVerts + 2 ][0][1] = cmd->t2;

	tess.xyz[ numVerts + 3 ][0] = m[0][0] * (-cmd->w) + m[1][0] * (cmd->h) + m[2][0];
	tess.xyz[ numVerts + 3 ][1] = m[0][1] * (-cmd->w) + m[1][1] * (cmd->h) + m[2][1];
	tess.xyz[ numVerts + 3 ][2] = 0;

	tess.texCoords[ numVerts + 3 ][0][0] = cmd->s1;
	tess.texCoords[ numVerts + 3 ][0][1] = cmd->t2;

	return (const void *)(cmd + 1);
}

/*
=============
RB_RotatePic2
=============
*/
const void *RB_RotatePic2 ( const void *data )
{
	const rotatePicCommand_t	*cmd;
	shader_t *shader;

	cmd = (const rotatePicCommand_t *)data;

	shader = cmd->shader;

	// FIXME is this needed
	if ( shader->numUnfoggedPasses )
	{
		if ( !backEnd.projection2D ) {
			RB_SetGL2D();
		}

		shader = cmd->shader;
		if ( shader != tess.shader ) {
			if ( tess.numIndexes ) {
				RB_EndSurface();
			}
			backEnd.currentEntity = &backEnd.entity2D;
			RB_BeginSurface( shader, 0 );
		}

		RB_CHECKOVERFLOW( 4, 6 );
		int numVerts = tess.numVertexes;
		int numIndexes = tess.numIndexes;

		float angle = DEG2RAD( cmd-> a );
		float s = sinf( angle );
		float c = cosf( angle );

		matrix3_t m = {
			{ c, s, 0.0f },
			{ -s, c, 0.0f },
			{ cmd->x, cmd->y, 1.0f }
		};

		tess.numVertexes += 4;
		tess.numIndexes += 6;

		tess.indexes[ numIndexes ] = numVerts + 3;
		tess.indexes[ numIndexes + 1 ] = numVerts + 0;
		tess.indexes[ numIndexes + 2 ] = numVerts + 2;
		tess.indexes[ numIndexes + 3 ] = numVerts + 2;
		tess.indexes[ numIndexes + 4 ] = numVerts + 0;
		tess.indexes[ numIndexes + 5 ] = numVerts + 1;

		byteAlias_t *baDest = NULL, *baSource = (byteAlias_t *)&backEnd.color2D;
		baDest = (byteAlias_t *)&tess.vertexColors[numVerts + 0]; baDest->ui = baSource->ui;
		baDest = (byteAlias_t *)&tess.vertexColors[numVerts + 1]; baDest->ui = baSource->ui;
		baDest = (byteAlias_t *)&tess.vertexColors[numVerts + 2]; baDest->ui = baSource->ui;
		baDest = (byteAlias_t *)&tess.vertexColors[numVerts + 3]; baDest->ui = baSource->ui;

		tess.xyz[ numVerts ][0] = m[0][0] * (-cmd->w * 0.5f) + m[1][0] * (-cmd->h * 0.5f) + m[2][0];
		tess.xyz[ numVerts ][1] = m[0][1] * (-cmd->w * 0.5f) + m[1][1] * (-cmd->h * 0.5f) + m[2][1];
		tess.xyz[ numVerts ][2] = 0;

		tess.texCoords[ numVerts ][0][0] = cmd->s1;
		tess.texCoords[ numVerts ][0][1] = cmd->t1;

		tess.xyz[ numVerts + 1 ][0] = m[0][0] * (cmd->w * 0.5f) + m[1][0] * (-cmd->h * 0.5f) + m[2][0];
		tess.xyz[ numVerts + 1 ][1] = m[0][1] * (cmd->w * 0.5f) + m[1][1] * (-cmd->h * 0.5f) + m[2][1];
		tess.xyz[ numVerts + 1 ][2] = 0;

		tess.texCoords[ numVerts + 1 ][0][0] = cmd->s2;
		tess.texCoords[ numVerts + 1 ][0][1] = cmd->t1;

		tess.xyz[ numVerts + 2 ][0] = m[0][0] * (cmd->w * 0.5f) + m[1][0] * (cmd->h * 0.5f) + m[2][0];
		tess.xyz[ numVerts + 2 ][1] = m[0][1] * (cmd->w * 0.5f) + m[1][1] * (cmd->h * 0.5f) + m[2][1];
		tess.xyz[ numVerts + 2 ][2] = 0;

		tess.texCoords[ numVerts + 2 ][0][0] = cmd->s2;
		tess.texCoords[ numVerts + 2 ][0][1] = cmd->t2;

		tess.xyz[ numVerts + 3 ][0] = m[0][0] * (-cmd->w * 0.5f) + m[1][0] * (cmd->h * 0.5f) + m[2][0];
		tess.xyz[ numVerts + 3 ][1] = m[0][1] * (-cmd->w * 0.5f) + m[1][1] * (cmd->h * 0.5f) + m[2][1];
		tess.xyz[ numVerts + 3 ][2] = 0;

		tess.texCoords[ numVerts + 3 ][0][0] = cmd->s1;
		tess.texCoords[ numVerts + 3 ][0][1] = cmd->t2;
	}

	return (const void *)(cmd + 1);
}

/*
=============
RB_ScissorPic
=============
*/
const void *RB_Scissor ( const void *data )
{
	const scissorCommand_t	*cmd;

	cmd = (const scissorCommand_t *)data;

	if ( !backEnd.projection2D )
	{
		RB_SetGL2D();
	}

	if (cmd->x >= 0)
	{
		qglScissor( cmd->x,(glConfig.vidHeight - cmd->y - cmd->h),cmd->w,cmd->h);
	}
	else
	{
		qglScissor( 0, 0, glConfig.vidWidth, glConfig.vidHeight);
	}

	return (const void *)(cmd + 1);
}

/*
=============
RB_DrawSurfs

=============
*/
const void	*RB_DrawSurfs( const void *data ) {
	const drawSurfsCommand_t	*cmd;

	// finish any 2D drawing if needed
	if ( tess.numIndexes ) {
		RB_EndSurface();
	}

	cmd = (const drawSurfsCommand_t *)data;

	backEnd.refdef = cmd->refdef;
	backEnd.viewParms = cmd->viewParms;

	RB_RenderDrawSurfList( cmd->drawSurfs, cmd->numDrawSurfs );

	// Dynamic Glow/Flares:
	/*
		The basic idea is to render the glowing parts of the scene to an offscreen buffer, then take
		that buffer and blur it. After it is sufficiently blurred, re-apply that image back to
		the normal screen using a additive blending. To blur the scene I use a vertex program to supply
		four texture coordinate offsets that allow 'peeking' into adjacent pixels. In the register
		combiner (pixel shader), I combine the adjacent pixels using a weighting factor. - Aurelio
	*/

	// Render dynamic glowing/flaring objects.
	if ( !(backEnd.refdef.rdflags & RDF_NOWORLDMODEL) && g_bDynamicGlowSupported && r_DynamicGlow->integer )
	{
		// Copy the normal scene to texture.
		qglDisable( GL_TEXTURE_2D );
		qglEnable( GL_TEXTURE_RECTANGLE_ARB );
		qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, tr.sceneImage );
		qglCopyTexSubImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, backEnd.viewParms.viewportX, backEnd.viewParms.viewportY, backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight);
		qglDisable( GL_TEXTURE_RECTANGLE_ARB );
		qglEnable( GL_TEXTURE_2D );

		// Just clear colors, but leave the depth buffer intact so we can 'share' it.
		qglClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
		qglClear( GL_COLOR_BUFFER_BIT );

		// Render the glowing objects.
		g_bRenderGlowingObjects = true;
		RB_RenderDrawSurfList( cmd->drawSurfs, cmd->numDrawSurfs );
		g_bRenderGlowingObjects = false;
		qglFinish();

		// Copy the glow scene to texture.
		qglDisable( GL_TEXTURE_2D );
		qglEnable( GL_TEXTURE_RECTANGLE_ARB );
		qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, tr.screenGlow );
		qglCopyTexSubImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0,  backEnd.viewParms.viewportX, backEnd.viewParms.viewportY, backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight );
		qglDisable( GL_TEXTURE_RECTANGLE_ARB );
		qglEnable( GL_TEXTURE_2D );

		// Resize the viewport to the blur texture size.
		const int oldViewWidth = backEnd.viewParms.viewportWidth;
		const int oldViewHeight = backEnd.viewParms.viewportHeight;
		backEnd.viewParms.viewportWidth = r_DynamicGlowWidth->integer;
		backEnd.viewParms.viewportHeight = r_DynamicGlowHeight->integer;
		//SetViewportAndScissor();
		qglViewport(0, 0, r_DynamicGlowWidth->integer, r_DynamicGlowHeight->integer);

		// Blur the scene.
		RB_BlurGlowTexture();

		// Copy the finished glow scene back to texture.
		qglDisable( GL_TEXTURE_2D );
		qglEnable( GL_TEXTURE_RECTANGLE_ARB );
		qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, tr.blurImage );
		qglCopyTexSubImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight );
		qglDisable( GL_TEXTURE_RECTANGLE_ARB );
		qglEnable( GL_TEXTURE_2D );

		// Set the viewport back to normal.
		backEnd.viewParms.viewportWidth = oldViewWidth;
		backEnd.viewParms.viewportHeight = oldViewHeight;
		SetViewportAndScissor();
		qglClear( GL_COLOR_BUFFER_BIT );

		// Draw the glow additively over the screen.
		RB_DrawGlowOverlay();
	}

	return (const void *)(cmd + 1);
}


/*
=============
RB_DrawBuffer

=============
*/
const void	*RB_DrawBuffer( const void *data ) {
	const drawBufferCommand_t	*cmd;

	if ( tess.numIndexes ) {
		RB_EndSurface();	//this might change culling and other states
	}

	cmd = (const drawBufferCommand_t *)data;

	GL_DrawBuffer( cmd->buffer );

		// clear screen for debugging
	if (!( backEnd.refdef.rdflags & RDF_NOWORLDMODEL ) && tr.world && tr.refdef.rdflags & RDF_doLAGoggles)
	{
		const fog_t		*fog = &tr.world->fogs[tr.world->numfogs];

		qglClearColor(fog->parms.color[0],  fog->parms.color[1], fog->parms.color[2], 1.0f );
		qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		backEnd.needPresent = qtrue;
	}
	else if (!( backEnd.refdef.rdflags & RDF_NOWORLDMODEL ) && tr.world && tr.world->globalFog != -1 && tr.sceneCount)//don't clear during menus, wait for real scene
	{
		const fog_t		*fog = &tr.world->fogs[tr.world->globalFog];

		qglClearColor(fog->parms.color[0],  fog->parms.color[1], fog->parms.color[2], 1.0f );
		qglClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		backEnd.needPresent = qtrue;
	}
	else if ( r_clear->integer )
	{	// clear screen for debugging
		int i = r_clear->integer;
		if (i == 42) {
			i = Q_irand(0,8);
		}
		switch (i)
		{
		default:
			qglClearColor( 1, 0, 0.5, 1 );
			break;
		case 1:
			qglClearColor( 1.0, 0.0, 0.0, 1.0); //red
			break;
		case 2:
			qglClearColor( 0.0, 1.0, 0.0, 1.0); //green
			break;
		case 3:
			qglClearColor( 1.0, 1.0, 0.0, 1.0); //yellow
			break;
		case 4:
			qglClearColor( 0.0, 0.0, 1.0, 1.0); //blue
			break;
		case 5:
			qglClearColor( 0.0, 1.0, 1.0, 1.0); //cyan
			break;
		case 6:
			qglClearColor( 1.0, 0.0, 1.0, 1.0); //magenta
			break;
		case 7:
			qglClearColor( 1.0, 1.0, 1.0, 1.0); //white
			break;
		case 8:
			qglClearColor( 0.0, 0.0, 0.0, 1.0); //black
			break;
		}
		qglClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		backEnd.needPresent = qtrue;
	}
	else
	{
		qglDepthMask( GL_TRUE );
		qglClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		if ( !GL_IsStateSet( GLS_DEPTHMASK_TRUE, GLS_DEPTHMASK_TRUE ) )
		{
			qglDepthMask( GL_FALSE );
		}
	}

	return (const void *)(cmd + 1);
}

/*
===============
RB_ShowImages

Draw all the images to the screen, on top of whatever
was there.  This is used to test for texture thrashing.

Also called by RE_EndRegistration
===============
*/
void RB_ShowImages( void ) {
	image_t	*image;
	float	x, y, w, h;
	//int		start, end;

	if ( !backEnd.projection2D ) {
		RB_SetGL2D();
	}

	qglFinish();

	//start = ri.Milliseconds();

	int i=0;
//	int iNumImages =
	   				 R_Images_StartIteration();
	while ( (image = R_Images_GetNextIteration()) != NULL)
	{
		w = glConfig.vidWidth / 20;
		h = glConfig.vidHeight / 15;
		x = i % 20 * w;
		y = i / 20 * h;

		// show in proportional size in mode 2
		if ( r_showImages->integer == 2 ) {
			w *= image->width / 512.0;
			h *= image->height / 512.0;
		}

		GL_Bind( image );
		qglBegin (GL_QUADS);
			qglTexCoord2f( 0, 0 );
			qglVertex2f( x, y );
			qglTexCoord2f( 1, 0 );
			qglVertex2f( x + w, y );
			qglTexCoord2f( 1, 1 );
			qglVertex2f( x + w, y + h );
			qglTexCoord2f( 0, 1 );
			qglVertex2f( x, y + h );
		qglEnd();
		i++;
		backEnd.needPresent = qtrue;
	}

	qglFinish();

	//end = ri.Milliseconds();
	//ri.Printf( PRINT_ALL, "%i msec to draw all images\n", end - start );
}


/*
=============
RB_SwapBuffers

=============
*/
extern void RB_RenderWorldEffects( void );
const void	*RB_SwapBuffers( const void *data ) {
	const swapBuffersCommand_t	*cmd;

	// finish any 2D drawing if needed
	if ( tess.numIndexes ) {
		RB_EndSurface();
	}

	// texture swapping test
	if ( r_showImages->integer ) {
		RB_ShowImages();
	}

	cmd = (const swapBuffersCommand_t *)data;

	// we measure overdraw by reading back the stencil buffer and
	// counting up the number of increments that have happened
	if ( r_measureOverdraw->integer ) {
		int i;
		long sum = 0;
		unsigned char *stencilReadback;

		stencilReadback = (unsigned char *) R_Malloc( glConfig.vidWidth * glConfig.vidHeight, TAG_TEMP_WORKSPACE, qfalse );
		qglReadPixels( 0, 0, glConfig.vidWidth, glConfig.vidHeight, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, stencilReadback );

		for ( i = 0; i < glConfig.vidWidth * glConfig.vidHeight; i++ ) {
			sum += stencilReadback[i];
		}

		backEnd.pc.c_overDraw += sum;
		R_Free( stencilReadback );
	}

    if ( !glState.finishCalled ) {
        qglFinish();
	}

    GLimp_LogComment( "***************** RB_SwapBuffers *****************\n\n\n" );

	if (backEnd.needPresent)
	{
		GL_Present();
		backEnd.needPresent = qfalse;
	}

	backEnd.projection2D = qfalse;

	return (const void *)(cmd + 1);
}

const void	*RB_WorldEffects( const void *data )
{
	const setModeCommand_t	*cmd;

	cmd = (const setModeCommand_t *)data;

	// Always flush the tess buffer
	if ( tess.shader && tess.numIndexes )
	{
		RB_EndSurface();
	}
	RB_RenderWorldEffects();

	if(tess.shader)
	{
		RB_BeginSurface( tess.shader, tess.fogNum );
	}

	return (const void *)(cmd + 1);
}

/*
====================
RB_ExecuteRenderCommands
====================
*/
static void ExecuteRenderCommands( const void *data, int buffer ) {
	while ( 1 ) {
		data = PADP(data, sizeof(void *));

		switch ( *(const int *)data ) {
		case RC_SET_COLOR:
			data = RB_SetColor( data );
			break;
		case RC_STRETCH_PIC:
			data = RB_StretchPic( data );
			break;
		case RC_STRETCH_SCRATCH:
			data = RB_StretchScratch( data );
			break;
		case RC_ROTATE_PIC:
			data = RB_RotatePic( data );
			break;
		case RC_ROTATE_PIC2:
			data = RB_RotatePic2( data );
			break;
		case RC_SCISSOR:
			data = RB_Scissor( data );
			break;
		case RC_DRAW_SURFS:
			data = RB_DrawSurfs( data );
			break;
		case RC_PROCESS_DISSOLVE:
			data = RB_ProcessDissolve( data );
			break;
		case RC_DRAW_BUFFER:
			((drawBufferCommand_t *)data)->buffer = buffer;
			data = RB_DrawBuffer( data );
			break;
		case RC_SWAP_BUFFERS:
			if (buffer == QGL_BACK || buffer == QGL_BACK_RIGHT)
			{
				data = RB_SwapBuffers( data );
			}
			else
			{
				data = (const void*)((swapBuffersCommand_t*)data + 1);
			}
			break;
		case RC_WORLD_EFFECTS:
			data = RB_WorldEffects( data );
			break;
		case RC_END_OF_LIST:
		default:
			return;
		}
	}

}

void RB_ExecuteRenderCommands( const void *data, const void *captured ) {
	int		t1, t2;

	t1 = ri.Milliseconds ();

	if (glConfig.stereoEnabled)
	{
		if (captured)
		{
			ExecuteRenderCommands(captured, QGL_OFFSCREEN_LEFT);
		}
		ExecuteRenderCommands(data, QGL_BACK_LEFT);
		if (captured)
		{
			ExecuteRenderCommands(captured, QGL_OFFSCREEN_RIGHT);
		}
		ExecuteRenderCommands(data, QGL_BACK_RIGHT);
	}
	else
	{
		if (captured)
		{
			ExecuteRenderCommands(captured, QGL_OFFSCREEN);
		}
		ExecuteRenderCommands(data, QGL_BACK);
	}

	// stop rendering
	t2 = ri.Milliseconds ();
	backEnd.pc.msec = t2 - t1;
}

// What Pixel Shader type is currently active (regcoms or fragment programs).
GLuint g_uiCurrentPixelShaderType = 0x0;

// Begin using a Pixel Shader.
void BeginPixelShader( GLuint uiType, GLuint uiID )
{
	switch ( uiType )
	{
		// Using Register Combiners, so call the Display List that stores it.
		case GL_REGISTER_COMBINERS_NV:
		{
			// Just in case...
			if ( !qglCombinerParameterfvNV )
				return;

			// Call the list with the regcom in it.
			qglEnable( GL_REGISTER_COMBINERS_NV );
			qglCallList( uiID );

			g_uiCurrentPixelShaderType = GL_REGISTER_COMBINERS_NV;
		}
		return;

		// Using Fragment Programs, so call the program.
		case GL_FRAGMENT_PROGRAM_ARB:
		{
			// Just in case...
			if ( !qglGenProgramsARB )
				return;

			qglEnable( GL_FRAGMENT_PROGRAM_ARB );
			qglBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, uiID );

			g_uiCurrentPixelShaderType = GL_FRAGMENT_PROGRAM_ARB;
		}
		return;
	}
}

// Stop using a Pixel Shader and return states to normal.
void EndPixelShader()
{
	if ( g_uiCurrentPixelShaderType == 0x0 )
		return;

	qglDisable( g_uiCurrentPixelShaderType );
}

// Hack variable for deciding which kind of texture rectangle thing to do (for some
// reason it acts different on radeon! It's against the spec!).
extern bool g_bTextureRectangleHack;

static inline void RB_BlurGlowTexture()
{
	qglDisable( GL_STENCIL_TEST );
	qglDisable (GL_CLIP_PLANE0);
	GL_Cull( CT_TWO_SIDED );

	// Go into orthographic 2d mode.
	qglMatrixMode(GL_PROJECTION);
	qglPushMatrix();
	qglLoadIdentity();
	qglOrtho(0, backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight, 0, -1, 1);
	qglMatrixMode(GL_MODELVIEW);
	qglPushMatrix();
	qglLoadIdentity();

	GL_State(GLS_DEPTHTEST_DISABLE);

	/////////////////////////////////////////////////////////
	// Setup vertex and pixel programs.
	/////////////////////////////////////////////////////////

	// NOTE: The 0.25 is because we're blending 4 textures (so = 1.0) and we want a relatively normalized pixel
	// intensity distribution, but this won't happen anyways if intensity is higher than 1.0.
	float fBlurDistribution = r_DynamicGlowIntensity->value * 0.25f;
	float fBlurWeight[4] = { fBlurDistribution, fBlurDistribution, fBlurDistribution, 1.0f };

	// Enable and set the Vertex Program.
	qglEnable( GL_VERTEX_PROGRAM_ARB );
	qglBindProgramARB( GL_VERTEX_PROGRAM_ARB, tr.glowVShader );

	// Apply Pixel Shaders.
	if ( qglCombinerParameterfvNV )
	{
		BeginPixelShader( GL_REGISTER_COMBINERS_NV, tr.glowPShader );

		// Pass the blur weight to the regcom.
		qglCombinerParameterfvNV( GL_CONSTANT_COLOR0_NV, (float*)&fBlurWeight );
	}
	else if ( qglProgramEnvParameter4fARB )
	{
		BeginPixelShader( GL_FRAGMENT_PROGRAM_ARB, tr.glowPShader );

		// Pass the blur weight to the Fragment Program.
		qglProgramEnvParameter4fARB( GL_FRAGMENT_PROGRAM_ARB, 0, fBlurWeight[0], fBlurWeight[1], fBlurWeight[2], fBlurWeight[3] );
	}

	/////////////////////////////////////////////////////////
	// Set the blur texture to the 4 texture stages.
	/////////////////////////////////////////////////////////

	// How much to offset each texel by.
	float fTexelWidthOffset = 0.1f, fTexelHeightOffset = 0.1f;

	GLuint uiTex = tr.screenGlow;

	qglActiveTextureARB( GL_TEXTURE3_ARB );
	qglEnable( GL_TEXTURE_RECTANGLE_ARB );
	qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, uiTex );

	qglActiveTextureARB( GL_TEXTURE2_ARB );
	qglEnable( GL_TEXTURE_RECTANGLE_ARB );
	qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, uiTex );

	qglActiveTextureARB( GL_TEXTURE1_ARB );
	qglEnable( GL_TEXTURE_RECTANGLE_ARB );
	qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, uiTex );

	qglActiveTextureARB(GL_TEXTURE0_ARB );
	qglDisable( GL_TEXTURE_2D );
	qglEnable( GL_TEXTURE_RECTANGLE_ARB );
	qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, uiTex );

	/////////////////////////////////////////////////////////
	// Draw the blur passes (each pass blurs it more, increasing the blur radius ).
	/////////////////////////////////////////////////////////

	//int iTexWidth = backEnd.viewParms.viewportWidth, iTexHeight = backEnd.viewParms.viewportHeight;
	int iTexWidth = glConfig.vidWidth, iTexHeight = glConfig.vidHeight;

	for ( int iNumBlurPasses = 0; iNumBlurPasses < r_DynamicGlowPasses->integer; iNumBlurPasses++ )
	{
		// Load the Texel Offsets into the Vertex Program.
		qglProgramEnvParameter4fARB( GL_VERTEX_PROGRAM_ARB, 0, -fTexelWidthOffset, -fTexelWidthOffset, 0.0f, 0.0f );
		qglProgramEnvParameter4fARB( GL_VERTEX_PROGRAM_ARB, 1, -fTexelWidthOffset, fTexelWidthOffset, 0.0f, 0.0f );
		qglProgramEnvParameter4fARB( GL_VERTEX_PROGRAM_ARB, 2, fTexelWidthOffset, -fTexelWidthOffset, 0.0f, 0.0f );
		qglProgramEnvParameter4fARB( GL_VERTEX_PROGRAM_ARB, 3, fTexelWidthOffset, fTexelWidthOffset, 0.0f, 0.0f );

		// After first pass put the tex coords to the viewport size.
		if ( iNumBlurPasses == 1 )
		{
			if ( !g_bTextureRectangleHack )
			{
				iTexWidth = backEnd.viewParms.viewportWidth;
				iTexHeight = backEnd.viewParms.viewportHeight;
			}

			uiTex = tr.blurImage;
			qglActiveTextureARB( GL_TEXTURE3_ARB );
			qglDisable( GL_TEXTURE_2D );
			qglEnable( GL_TEXTURE_RECTANGLE_ARB );
			qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, uiTex );
			qglActiveTextureARB( GL_TEXTURE2_ARB );
			qglDisable( GL_TEXTURE_2D );
			qglEnable( GL_TEXTURE_RECTANGLE_ARB );
			qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, uiTex );
			qglActiveTextureARB( GL_TEXTURE1_ARB );
			qglDisable( GL_TEXTURE_2D );
			qglEnable( GL_TEXTURE_RECTANGLE_ARB );
			qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, uiTex );
			qglActiveTextureARB(GL_TEXTURE0_ARB );
			qglDisable( GL_TEXTURE_2D );
			qglEnable( GL_TEXTURE_RECTANGLE_ARB );
			qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, uiTex );

			// Copy the current image over.
			qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, uiTex );
			qglCopyTexSubImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight );
		}

		// Draw the fullscreen quad.
		qglBegin( GL_QUADS );
			qglMultiTexCoord2fARB( GL_TEXTURE0_ARB, 0, iTexHeight );
			qglVertex2f( 0, 0 );

			qglMultiTexCoord2fARB( GL_TEXTURE0_ARB, 0, 0 );
			qglVertex2f( 0, backEnd.viewParms.viewportHeight );

			qglMultiTexCoord2fARB( GL_TEXTURE0_ARB, iTexWidth, 0 );
			qglVertex2f( backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight );

			qglMultiTexCoord2fARB( GL_TEXTURE0_ARB, iTexWidth, iTexHeight );
			qglVertex2f( backEnd.viewParms.viewportWidth, 0 );
		qglEnd();

		qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, tr.blurImage );
		qglCopyTexSubImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight );

		// Increase the texel offsets.
		// NOTE: This is possibly the most important input to the effect. Even by using an exponential function I've been able to
		// make it look better (at a much higher cost of course). This is cheap though and still looks pretty great. In the future
		// I might want to use an actual gaussian equation to correctly calculate the pixel coefficients and attenuates, texel
		// offsets, gaussian amplitude and radius...
		fTexelWidthOffset += r_DynamicGlowDelta->value;
		fTexelHeightOffset += r_DynamicGlowDelta->value;
	}

	// Disable multi-texturing.
	qglActiveTextureARB( GL_TEXTURE3_ARB );
	qglDisable( GL_TEXTURE_RECTANGLE_ARB );

	qglActiveTextureARB( GL_TEXTURE2_ARB );
	qglDisable( GL_TEXTURE_RECTANGLE_ARB );

	qglActiveTextureARB( GL_TEXTURE1_ARB );
	qglDisable( GL_TEXTURE_RECTANGLE_ARB );

	qglActiveTextureARB(GL_TEXTURE0_ARB );
	qglDisable( GL_TEXTURE_RECTANGLE_ARB );
	qglEnable( GL_TEXTURE_2D );

	qglDisable( GL_VERTEX_PROGRAM_ARB );
	EndPixelShader();

	qglMatrixMode(GL_PROJECTION);
	qglPopMatrix();
	qglMatrixMode(GL_MODELVIEW);
	qglPopMatrix();

	qglDisable( GL_BLEND );
	glState.currenttmu = 0;	//this matches the last one we activated
}

// Draw the glow blur over the screen additively.
static inline void RB_DrawGlowOverlay()
{
	qglEnable( GL_STENCIL_TEST );
	qglDisable (GL_CLIP_PLANE0);
	GL_Cull( CT_TWO_SIDED );

	// Go into orthographic 2d mode.
	qglMatrixMode(GL_PROJECTION);
	qglPushMatrix();
	qglLoadIdentity();
	qglOrtho(0, glConfig.vidWidth, glConfig.vidHeight, 0, -1, 1);
	qglMatrixMode(GL_MODELVIEW);
	qglPushMatrix();
	qglLoadIdentity();

	GL_State(GLS_DEPTHTEST_DISABLE);

	qglDisable( GL_TEXTURE_2D );
	qglEnable( GL_TEXTURE_RECTANGLE_ARB );

	// For debug purposes.
	if ( r_DynamicGlow->integer != 2 )
	{
		// Render the normal scene texture.
		qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, tr.sceneImage );
		qglBegin(GL_QUADS);
			qglColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
			qglTexCoord2f( 0, glConfig.vidHeight );
			qglVertex2f( 0, 0 );

			qglTexCoord2f( 0, 0 );
			qglVertex2f( 0, glConfig.vidHeight );

			qglTexCoord2f( glConfig.vidWidth, 0 );
			qglVertex2f( glConfig.vidWidth, glConfig.vidHeight );

			qglTexCoord2f( glConfig.vidWidth, glConfig.vidHeight );
			qglVertex2f( glConfig.vidWidth, 0 );
		qglEnd();
	}

	// One and Inverse Src Color give a very soft addition, while one one is a bit stronger. With one one we can
	// use additive blending through multitexture though.
	if ( r_DynamicGlowSoft->integer )
	{
		qglBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_COLOR );
	}
	else
	{
		qglBlendFunc( GL_ONE, GL_ONE );
	}
	qglEnable( GL_BLEND );

	// Now additively render the glow texture.
	qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, tr.blurImage );
	qglBegin(GL_QUADS);
		qglColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
		qglTexCoord2f( 0, r_DynamicGlowHeight->integer );
		qglVertex2f( 0, 0 );

		qglTexCoord2f( 0, 0 );
		qglVertex2f( 0, glConfig.vidHeight );

		qglTexCoord2f( r_DynamicGlowWidth->integer, 0 );
		qglVertex2f( glConfig.vidWidth, glConfig.vidHeight );

		qglTexCoord2f( r_DynamicGlowWidth->integer, r_DynamicGlowHeight->integer );
		qglVertex2f( glConfig.vidWidth, 0 );
	qglEnd();

	qglDisable( GL_TEXTURE_RECTANGLE_ARB );
	qglEnable( GL_TEXTURE_2D );
	qglBlendFunc( GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR );
	qglDisable( GL_BLEND );

	qglMatrixMode(GL_PROJECTION);
	qglPopMatrix();
	qglMatrixMode(GL_MODELVIEW);
	qglPopMatrix();
}
