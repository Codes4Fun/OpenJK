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

#pragma once

#if defined( __LINT__ )
#	include <GL/gl.h>
#elif defined( _WIN32 )
#	include <windows.h>
#	include <GL/gl.h>
#elif defined(MACOS_X)
// Prevent OS X headers from including its own glext header
#	define GL_GLEXT_LEGACY
#	include <OpenGL/gl.h>
#elif defined( __linux__ )
#	include <GL/gl.h>
#	include <GL/glx.h>
// bk001129 - from cvs1.17 (mkv)
#	if defined(__FX__)
#		include <GL/fxmesa.h>
#	endif
#elif defined( __FreeBSD__ ) // rb010123
#	include <GL/gl.h>
#	include <GL/glx.h>
#	if defined(__FX__)
#		include <GL/fxmesa.h>
#	endif
#else
#	include <gl.h>
#endif

#include "glext.h"

#define qglAccum glAccum
#define qglAlphaFunc glAlphaFunc
#define qglAreTexturesResident glAreTexturesResident
#define qglArrayElement glArrayElement
#define qglBegin glBegin
#define qglBindTexture glBindTexture
#define qglBitmap glBitmap
#define qglBlendFunc glBlendFunc
#define qglCallList glCallList
#define qglCallLists glCallLists
#define qglClear glClear
#define qglClearAccum glClearAccum
#define qglClearColor glClearColor
#define qglClearDepth glClearDepth
#define qglClearIndex glClearIndex
#define qglClearStencil glClearStencil
#define qglClipPlane glClipPlane
#define qglColor3b glColor3b
#define qglColor3bv glColor3bv
#define qglColor3d glColor3d
#define qglColor3dv glColor3dv
#define qglColor3f glColor3f
#define qglColor3fv glColor3fv
#define qglColor3i glColor3i
#define qglColor3iv glColor3iv
#define qglColor3s glColor3s
#define qglColor3sv glColor3sv
#define qglColor3ub glColor3ub
#define qglColor3ubv glColor3ubv
#define qglColor3ui glColor3ui
#define qglColor3uiv glColor3uiv
#define qglColor3us glColor3us
#define qglColor3usv glColor3usv
#define qglColor4b glColor4b
#define qglColor4bv glColor4bv
#define qglColor4d glColor4d
#define qglColor4dv glColor4dv
#define qglColor4f glColor4f
#define qglColor4fv glColor4fv
#define qglColor4i glColor4i
#define qglColor4iv glColor4iv
#define qglColor4s glColor4s
#define qglColor4sv glColor4sv
#define qglColor4ub glColor4ub
#define qglColor4ubv glColor4ubv
#define qglColor4ui glColor4ui
#define qglColor4uiv glColor4uiv
#define qglColor4us glColor4us
#define qglColor4usv glColor4usv
#define qglColorMask glColorMask
#define qglColorMaterial glColorMaterial
#define qglColorPointer glColorPointer
#define qglCopyPixels glCopyPixels
#define qglCopyTexImage1D glCopyTexImage1D
#define qglCopyTexImage2D glCopyTexImage2D
#define qglCopyTexSubImage1D glCopyTexSubImage1D
#define qglCopyTexSubImage2D glCopyTexSubImage2D
#define qglCullFace glCullFace
#define qglDeleteLists glDeleteLists
#define qglDeleteTextures glDeleteTextures
#define qglDepthFunc glDepthFunc
#define qglDepthMask glDepthMask
#define qglDepthRange glDepthRange
#define qglDisable glDisable
#define qglDisableClientState glDisableClientState
#define qglDrawArrays glDrawArrays
#define qglDrawBuffer glDrawBuffer
#define qglDrawElements glDrawElements
#define qglDrawPixels glDrawPixels
#define qglEdgeFlag glEdgeFlag
#define qglEdgeFlagPointer glEdgeFlagPointer
#define qglEdgeFlagv glEdgeFlagv
#define qglEnable glEnable
#define qglEnableClientState glEnableClientState
#define qglEnd glEnd
#define qglEndList glEndList
#define qglEvalCoord1d glEvalCoord1d
#define qglEvalCoord1dv glEvalCoord1dv
#define qglEvalCoord1f glEvalCoord1f
#define qglEvalCoord1fv glEvalCoord1fv
#define qglEvalCoord2d glEvalCoord2d
#define qglEvalCoord2dv glEvalCoord2dv
#define qglEvalCoord2f glEvalCoord2f
#define qglEvalCoord2fv glEvalCoord2fv
#define qglEvalMesh1 glEvalMesh1
#define qglEvalMesh2 glEvalMesh2
#define qglEvalPoint1 glEvalPoint1
#define qglEvalPoint2 glEvalPoint2
#define qglFeedbackBuffer glFeedbackBuffer
#define qglFinish glFinish
#define qglFlush glFlush
#define qglFogf glFogf
#define qglFogfv glFogfv
#define qglFogi glFogi
#define qglFogiv glFogiv
#define qglFrontFace glFrontFace
#define qglFrustum glFrustum
#define qglGenLists glGenLists
#define qglGenTextures glGenTextures
#define qglGetBooleanv glGetBooleanv
#define qglGetClipPlane glGetClipPlane
#define qglGetDoublev glGetDoublev
#define qglGetError glGetError
#define qglGetFloatv glGetFloatv
#define qglGetIntegerv glGetIntegerv
#define qglGetLightfv glGetLightfv
#define qglGetLightiv glGetLightiv
#define qglGetMapdv glGetMapdv
#define qglGetMapfv glGetMapfv
#define qglGetMapiv glGetMapiv
#define qglGetMaterialfv glGetMaterialfv
#define qglGetMaterialiv glGetMaterialiv
#define qglGetPixelMapfv glGetPixelMapfv
#define qglGetPixelMapuiv glGetPixelMapuiv
#define qglGetPixelMapusv glGetPixelMapusv
#define qglGetPointerv glGetPointerv
#define qglGetPolygonStipple glGetPolygonStipple
#define qglGetString glGetString
#define qglGetTexGendv glGetTexGendv
#define qglGetTexGenfv glGetTexGenfv
#define qglGetTexGeniv glGetTexGeniv
#define qglGetTexImage glGetTexImage
#define qglGetTexLevelParameterfv glGetTexLevelParameterfv
#define qglGetTexLevelParameteriv glGetTexLevelParameteriv
#define qglGetTexParameterfv glGetTexParameterfv
#define qglGetTexParameteriv glGetTexParameteriv
#define qglHint glHint
#define qglIndexMask glIndexMask
#define qglIndexPointer glIndexPointer
#define qglIndexd glIndexd
#define qglIndexdv glIndexdv
#define qglIndexf glIndexf
#define qglIndexfv glIndexfv
#define qglIndexi glIndexi
#define qglIndexiv glIndexiv
#define qglIndexs glIndexs
#define qglIndexsv glIndexsv
#define qglIndexub glIndexub
#define qglIndexubv glIndexubv
#define qglInitNames glInitNames
#define qglInterleavedArrays glInterleavedArrays
#define qglIsEnabled glIsEnabled
#define qglIsList glIsList
#define qglIsTexture glIsTexture
#define qglLightModelf glLightModelf
#define qglLightModelfv glLightModelfv
#define qglLightModeli glLightModeli
#define qglLightModeliv glLightModeliv
#define qglLightf glLightf
#define qglLightfv glLightfv
#define qglLighti glLighti
#define qglLightiv glLightiv
#define qglLineStipple glLineStipple
#define qglLineWidth glLineWidth
#define qglListBase glListBase
#define qglLoadIdentity glLoadIdentity
#define qglLoadMatrixd glLoadMatrixd
#define qglLoadMatrixf glLoadMatrixf
#define qglLoadName glLoadName
#define qglLogicOp glLogicOp
#define qglMap1d glMap1d
#define qglMap1f glMap1f
#define qglMap2d glMap2d
#define qglMap2f glMap2f
#define qglMapGrid1d glMapGrid1d
#define qglMapGrid1f glMapGrid1f
#define qglMapGrid2d glMapGrid2d
#define qglMapGrid2f glMapGrid2f
#define qglMaterialf glMaterialf
#define qglMaterialfv glMaterialfv
#define qglMateriali glMateriali
#define qglMaterialiv glMaterialiv
#define qglMatrixMode glMatrixMode
#define qglMultMatrixd glMultMatrixd
#define qglMultMatrixf glMultMatrixf
#define qglNewList glNewList
#define qglNormal3b glNormal3b
#define qglNormal3bv glNormal3bv
#define qglNormal3d glNormal3d
#define qglNormal3dv glNormal3dv
#define qglNormal3f glNormal3f
#define qglNormal3fv glNormal3fv
#define qglNormal3i glNormal3i
#define qglNormal3iv glNormal3iv
#define qglNormal3s glNormal3s
#define qglNormal3sv glNormal3sv
#define qglNormalPointer glNormalPointer
#define qglOrtho glOrtho
#define qglPassThrough glPassThrough
#define qglPixelMapfv glPixelMapfv
#define qglPixelMapuiv glPixelMapuiv
#define qglPixelMapusv glPixelMapusv
#define qglPixelStoref glPixelStoref
#define qglPixelStorei glPixelStorei
#define qglPixelTransferf glPixelTransferf
#define qglPixelTransferi glPixelTransferi
#define qglPixelZoom glPixelZoom
#define qglPointSize glPointSize
#define qglPolygonMode glPolygonMode
#define qglPolygonOffset glPolygonOffset
#define qglPolygonStipple glPolygonStipple
#define qglPopAttrib glPopAttrib
#define qglPopClientAttrib glPopClientAttrib
#define qglPopMatrix glPopMatrix
#define qglPopName glPopName
#define qglPrioritizeTextures glPrioritizeTextures
#define qglPushAttrib glPushAttrib
#define qglPushClientAttrib glPushClientAttrib
#define qglPushMatrix glPushMatrix
#define qglPushName glPushName
#define qglRasterPos2d glRasterPos2d
#define qglRasterPos2dv glRasterPos2dv
#define qglRasterPos2f glRasterPos2f
#define qglRasterPos2fv glRasterPos2fv
#define qglRasterPos2i glRasterPos2i
#define qglRasterPos2iv glRasterPos2iv
#define qglRasterPos2s glRasterPos2s
#define qglRasterPos2sv glRasterPos2sv
#define qglRasterPos3d glRasterPos3d
#define qglRasterPos3dv glRasterPos3dv
#define qglRasterPos3f glRasterPos3f
#define qglRasterPos3fv glRasterPos3fv
#define qglRasterPos3i glRasterPos3i
#define qglRasterPos3iv glRasterPos3iv
#define qglRasterPos3s glRasterPos3s
#define qglRasterPos3sv glRasterPos3sv
#define qglRasterPos4d glRasterPos4d
#define qglRasterPos4dv glRasterPos4dv
#define qglRasterPos4f glRasterPos4f
#define qglRasterPos4fv glRasterPos4fv
#define qglRasterPos4i glRasterPos4i
#define qglRasterPos4iv glRasterPos4iv
#define qglRasterPos4s glRasterPos4s
#define qglRasterPos4sv glRasterPos4sv
#define qglReadBuffer glReadBuffer
#define qglReadPixels glReadPixels
#define qglRectd glRectd
#define qglRectdv glRectdv
#define qglRectf glRectf
#define qglRectfv glRectfv
#define qglRecti glRecti
#define qglRectiv glRectiv
#define qglRects glRects
#define qglRectsv glRectsv
#define qglRenderMode glRenderMode
#define qglRotated glRotated
#define qglRotatef glRotatef
#define qglScaled glScaled
#define qglScalef glScalef
#define qglScissor glScissor
#define qglSelectBuffer glSelectBuffer
#define qglShadeModel glShadeModel
#define qglStencilFunc glStencilFunc
#define qglStencilMask glStencilMask
#define qglStencilOp glStencilOp
#define qglTexCoord1d glTexCoord1d
#define qglTexCoord1dv glTexCoord1dv
#define qglTexCoord1f glTexCoord1f
#define qglTexCoord1fv glTexCoord1fv
#define qglTexCoord1i glTexCoord1i
#define qglTexCoord1iv glTexCoord1iv
#define qglTexCoord1s glTexCoord1s
#define qglTexCoord1sv glTexCoord1sv
#define qglTexCoord2d glTexCoord2d
#define qglTexCoord2dv glTexCoord2dv
#define qglTexCoord2f glTexCoord2f
#define qglTexCoord2fv glTexCoord2fv
#define qglTexCoord2i glTexCoord2i
#define qglTexCoord2iv glTexCoord2iv
#define qglTexCoord2s glTexCoord2s
#define qglTexCoord2sv glTexCoord2sv
#define qglTexCoord3d glTexCoord3d
#define qglTexCoord3dv glTexCoord3dv
#define qglTexCoord3f glTexCoord3f
#define qglTexCoord3fv glTexCoord3fv
#define qglTexCoord3i glTexCoord3i
#define qglTexCoord3iv glTexCoord3iv
#define qglTexCoord3s glTexCoord3s
#define qglTexCoord3sv glTexCoord3sv
#define qglTexCoord4d glTexCoord4d
#define qglTexCoord4dv glTexCoord4dv
#define qglTexCoord4f glTexCoord4f
#define qglTexCoord4fv glTexCoord4fv
#define qglTexCoord4i glTexCoord4i
#define qglTexCoord4iv glTexCoord4iv
#define qglTexCoord4s glTexCoord4s
#define qglTexCoord4sv glTexCoord4sv
#define qglTexCoordPointer glTexCoordPointer
#define qglTexEnvf glTexEnvf
#define qglTexEnvfv glTexEnvfv
#define qglTexEnvi glTexEnvi
#define qglTexEnviv glTexEnviv
#define qglTexGend glTexGend
#define qglTexGendv glTexGendv
#define qglTexGenf glTexGenf
#define qglTexGenfv glTexGenfv
#define qglTexGeni glTexGeni
#define qglTexGeniv glTexGeniv
#define qglTexImage1D glTexImage1D
#define qglTexImage2D glTexImage2D
#define qglTexParameterf glTexParameterf
#define qglTexParameterfv glTexParameterfv
#define qglTexParameteri glTexParameteri
#define qglTexParameteriv glTexParameteriv
#define qglTexSubImage1D glTexSubImage1D
#define qglTexSubImage2D glTexSubImage2D
#define qglTranslated glTranslated
#define qglTranslatef glTranslatef
#define qglVertex2d glVertex2d
#define qglVertex2dv glVertex2dv
#define qglVertex2f glVertex2f
#define qglVertex2fv glVertex2fv
#define qglVertex2i glVertex2i
#define qglVertex2iv glVertex2iv
#define qglVertex2s glVertex2s
#define qglVertex2sv glVertex2sv
#define qglVertex3d glVertex3d
#define qglVertex3dv glVertex3dv
#define qglVertex3f glVertex3f
#define qglVertex3fv glVertex3fv
#define qglVertex3i glVertex3i
#define qglVertex3iv glVertex3iv
#define qglVertex3s glVertex3s
#define qglVertex3sv glVertex3sv
#define qglVertex4d glVertex4d
#define qglVertex4dv glVertex4dv
#define qglVertex4f glVertex4f
#define qglVertex4fv glVertex4fv
#define qglVertex4i glVertex4i
#define qglVertex4iv glVertex4iv
#define qglVertex4s glVertex4s
#define qglVertex4sv glVertex4sv
#define qglVertexPointer glVertexPointer
#define qglViewport glViewport

extern PFNGLACTIVETEXTUREARBPROC qglActiveTextureARB;
extern PFNGLCLIENTACTIVETEXTUREARBPROC qglClientActiveTextureARB;
extern PFNGLMULTITEXCOORD2FARBPROC qglMultiTexCoord2fARB;

extern PFNGLCOMBINERPARAMETERFVNVPROC qglCombinerParameterfvNV;
extern PFNGLCOMBINERPARAMETERIVNVPROC qglCombinerParameterivNV;
extern PFNGLCOMBINERPARAMETERFNVPROC qglCombinerParameterfNV;
extern PFNGLCOMBINERPARAMETERINVPROC qglCombinerParameteriNV;
extern PFNGLCOMBINERINPUTNVPROC qglCombinerInputNV;
extern PFNGLCOMBINEROUTPUTNVPROC qglCombinerOutputNV;

extern PFNGLFINALCOMBINERINPUTNVPROC qglFinalCombinerInputNV;
extern PFNGLGETCOMBINERINPUTPARAMETERFVNVPROC qglGetCombinerInputParameterfvNV;
extern PFNGLGETCOMBINERINPUTPARAMETERIVNVPROC qglGetCombinerInputParameterivNV;
extern PFNGLGETCOMBINEROUTPUTPARAMETERFVNVPROC qglGetCombinerOutputParameterfvNV;
extern PFNGLGETCOMBINEROUTPUTPARAMETERIVNVPROC qglGetCombinerOutputParameterivNV;
extern PFNGLGETFINALCOMBINERINPUTPARAMETERFVNVPROC qglGetFinalCombinerInputParameterfvNV;
extern PFNGLGETFINALCOMBINERINPUTPARAMETERIVNVPROC qglGetFinalCombinerInputParameterivNV;

extern PFNGLPROGRAMSTRINGARBPROC qglProgramStringARB;
extern PFNGLBINDPROGRAMARBPROC qglBindProgramARB;
extern PFNGLDELETEPROGRAMSARBPROC qglDeleteProgramsARB;
extern PFNGLGENPROGRAMSARBPROC qglGenProgramsARB;
extern PFNGLPROGRAMENVPARAMETER4DARBPROC qglProgramEnvParameter4dARB;
extern PFNGLPROGRAMENVPARAMETER4DVARBPROC qglProgramEnvParameter4dvARB;
extern PFNGLPROGRAMENVPARAMETER4FARBPROC qglProgramEnvParameter4fARB;
extern PFNGLPROGRAMENVPARAMETER4FVARBPROC qglProgramEnvParameter4fvARB;
extern PFNGLPROGRAMLOCALPARAMETER4DARBPROC qglProgramLocalParameter4dARB;
extern PFNGLPROGRAMLOCALPARAMETER4DVARBPROC qglProgramLocalParameter4dvARB;
extern PFNGLPROGRAMLOCALPARAMETER4FARBPROC qglProgramLocalParameter4fARB;
extern PFNGLPROGRAMLOCALPARAMETER4FVARBPROC qglProgramLocalParameter4fvARB;
extern PFNGLGETPROGRAMENVPARAMETERDVARBPROC qglGetProgramEnvParameterdvARB;
extern PFNGLGETPROGRAMENVPARAMETERFVARBPROC qglGetProgramEnvParameterfvARB;
extern PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC qglGetProgramLocalParameterdvARB;
extern PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC qglGetProgramLocalParameterfvARB;
extern PFNGLGETPROGRAMIVARBPROC qglGetProgramivARB;
extern PFNGLGETPROGRAMSTRINGARBPROC qglGetProgramStringARB;
extern PFNGLISPROGRAMARBPROC qglIsProgramARB;

extern PFNGLLOCKARRAYSEXTPROC qglLockArraysEXT;
extern PFNGLUNLOCKARRAYSEXTPROC qglUnlockArraysEXT;


#define QGLDECLARE(x) extern PFN##x##PROC q##x
#define QGLDEFINE(x) PFN##x##PROC q##x
#define QGLGETPROC(x) q##x## = (PFN##x##PROC)ri.GL_GetProcAddress(#x)

// OpenGL 2.0

#define PFNglBlendEquationSeparatePROC PFNGLBLENDEQUATIONSEPARATEPROC
#define PFNglDrawBuffersPROC PFNGLDRAWBUFFERSPROC
#define PFNglStencilOpSeparatePROC PFNGLSTENCILOPSEPARATEPROC
#define PFNglStencilFuncSeparatePROC PFNGLSTENCILFUNCSEPARATEPROC
#define PFNglStencilMaskSeparatePROC PFNGLSTENCILMASKSEPARATEPROC
#define PFNglAttachShaderPROC PFNGLATTACHSHADERPROC
#define PFNglBindAttribLocationPROC PFNGLBINDATTRIBLOCATIONPROC
#define PFNglCompileShaderPROC PFNGLCOMPILESHADERPROC
#define PFNglCreateProgramPROC PFNGLCREATEPROGRAMPROC
#define PFNglCreateShaderPROC PFNGLCREATESHADERPROC
#define PFNglDeleteProgramPROC PFNGLDELETEPROGRAMPROC
#define PFNglDeleteShaderPROC PFNGLDELETESHADERPROC
#define PFNglDetachShaderPROC PFNGLDETACHSHADERPROC
#define PFNglDisableVertexAttribArrayPROC PFNGLDISABLEVERTEXATTRIBARRAYPROC
#define PFNglEnableVertexAttribArrayPROC PFNGLENABLEVERTEXATTRIBARRAYPROC
#define PFNglGetActiveAttribPROC PFNGLGETACTIVEATTRIBPROC
#define PFNglGetActiveUniformPROC PFNGLGETACTIVEUNIFORMPROC
#define PFNglGetAttachedShadersPROC PFNGLGETATTACHEDSHADERSPROC
#define PFNglGetAttribLocationPROC PFNGLGETATTRIBLOCATIONPROC
#define PFNglGetProgramivPROC PFNGLGETPROGRAMIVPROC
#define PFNglGetProgramInfoLogPROC PFNGLGETPROGRAMINFOLOGPROC
#define PFNglGetShaderivPROC PFNGLGETSHADERIVPROC
#define PFNglGetShaderInfoLogPROC PFNGLGETSHADERINFOLOGPROC
#define PFNglGetShaderSourcePROC PFNGLGETSHADERSOURCEPROC
#define PFNglGetUniformLocationPROC PFNGLGETUNIFORMLOCATIONPROC
#define PFNglGetUniformfvPROC PFNGLGETUNIFORMFVPROC
#define PFNglGetUniformivPROC PFNGLGETUNIFORMIVPROC
#define PFNglGetVertexAttribdvPROC PFNGLGETVERTEXATTRIBDVPROC
#define PFNglGetVertexAttribfvPROC PFNGLGETVERTEXATTRIBFVPROC
#define PFNglGetVertexAttribivPROC PFNGLGETVERTEXATTRIBIVPROC
#define PFNglGetVertexAttribPointervPROC PFNGLGETVERTEXATTRIBPOINTERVPROC
#define PFNglIsProgramPROC PFNGLISPROGRAMPROC
#define PFNglIsShaderPROC PFNGLISSHADERPROC
#define PFNglLinkProgramPROC PFNGLLINKPROGRAMPROC
#define PFNglShaderSourcePROC PFNGLSHADERSOURCEPROC
#define PFNglUseProgramPROC PFNGLUSEPROGRAMPROC
#define PFNglUniform1fPROC PFNGLUNIFORM1FPROC
#define PFNglUniform2fPROC PFNGLUNIFORM2FPROC
#define PFNglUniform3fPROC PFNGLUNIFORM3FPROC
#define PFNglUniform4fPROC PFNGLUNIFORM4FPROC
#define PFNglUniform1iPROC PFNGLUNIFORM1IPROC
#define PFNglUniform2iPROC PFNGLUNIFORM2IPROC
#define PFNglUniform3iPROC PFNGLUNIFORM3IPROC
#define PFNglUniform4iPROC PFNGLUNIFORM4IPROC
#define PFNglUniform1fvPROC PFNGLUNIFORM1FVPROC
#define PFNglUniform2fvPROC PFNGLUNIFORM2FVPROC
#define PFNglUniform3fvPROC PFNGLUNIFORM3FVPROC
#define PFNglUniform4fvPROC PFNGLUNIFORM4FVPROC
#define PFNglUniform1ivPROC PFNGLUNIFORM1IVPROC
#define PFNglUniform2ivPROC PFNGLUNIFORM2IVPROC
#define PFNglUniform3ivPROC PFNGLUNIFORM3IVPROC
#define PFNglUniform4ivPROC PFNGLUNIFORM4IVPROC
#define PFNglUniformMatrix2fvPROC PFNGLUNIFORMMATRIX2FVPROC
#define PFNglUniformMatrix3fvPROC PFNGLUNIFORMMATRIX3FVPROC
#define PFNglUniformMatrix4fvPROC PFNGLUNIFORMMATRIX4FVPROC
#define PFNglValidateProgramPROC PFNGLVALIDATEPROGRAMPROC
#define PFNglVertexAttrib1dPROC PFNGLVERTEXATTRIB1DPROC
#define PFNglVertexAttrib1dvPROC PFNGLVERTEXATTRIB1DVPROC
#define PFNglVertexAttrib1fPROC PFNGLVERTEXATTRIB1FPROC
#define PFNglVertexAttrib1fvPROC PFNGLVERTEXATTRIB1FVPROC
#define PFNglVertexAttrib1sPROC PFNGLVERTEXATTRIB1SPROC
#define PFNglVertexAttrib1svPROC PFNGLVERTEXATTRIB1SVPROC
#define PFNglVertexAttrib2dPROC PFNGLVERTEXATTRIB2DPROC
#define PFNglVertexAttrib2dvPROC PFNGLVERTEXATTRIB2DVPROC
#define PFNglVertexAttrib2fPROC PFNGLVERTEXATTRIB2FPROC
#define PFNglVertexAttrib2fvPROC PFNGLVERTEXATTRIB2FVPROC
#define PFNglVertexAttrib2sPROC PFNGLVERTEXATTRIB2SPROC
#define PFNglVertexAttrib2svPROC PFNGLVERTEXATTRIB2SVPROC
#define PFNglVertexAttrib3dPROC PFNGLVERTEXATTRIB3DPROC
#define PFNglVertexAttrib3dvPROC PFNGLVERTEXATTRIB3DVPROC
#define PFNglVertexAttrib3fPROC PFNGLVERTEXATTRIB3FPROC
#define PFNglVertexAttrib3fvPROC PFNGLVERTEXATTRIB3FVPROC
#define PFNglVertexAttrib3sPROC PFNGLVERTEXATTRIB3SPROC
#define PFNglVertexAttrib3svPROC PFNGLVERTEXATTRIB3SVPROC
#define PFNglVertexAttrib4NbvPROC PFNGLVERTEXATTRIB4NBVPROC
#define PFNglVertexAttrib4NivPROC PFNGLVERTEXATTRIB4NIVPROC
#define PFNglVertexAttrib4NsvPROC PFNGLVERTEXATTRIB4NSVPROC
#define PFNglVertexAttrib4NubPROC PFNGLVERTEXATTRIB4NUBPROC
#define PFNglVertexAttrib4NubvPROC PFNGLVERTEXATTRIB4NUBVPROC
#define PFNglVertexAttrib4NuivPROC PFNGLVERTEXATTRIB4NUIVPROC
#define PFNglVertexAttrib4NusvPROC PFNGLVERTEXATTRIB4NUSVPROC
#define PFNglVertexAttrib4bvPROC PFNGLVERTEXATTRIB4BVPROC
#define PFNglVertexAttrib4dPROC PFNGLVERTEXATTRIB4DPROC
#define PFNglVertexAttrib4dvPROC PFNGLVERTEXATTRIB4DVPROC
#define PFNglVertexAttrib4fPROC PFNGLVERTEXATTRIB4FPROC
#define PFNglVertexAttrib4fvPROC PFNGLVERTEXATTRIB4FVPROC
#define PFNglVertexAttrib4ivPROC PFNGLVERTEXATTRIB4IVPROC
#define PFNglVertexAttrib4sPROC PFNGLVERTEXATTRIB4SPROC
#define PFNglVertexAttrib4svPROC PFNGLVERTEXATTRIB4SVPROC
#define PFNglVertexAttrib4ubvPROC PFNGLVERTEXATTRIB4UBVPROC
#define PFNglVertexAttrib4uivPROC PFNGLVERTEXATTRIB4UIVPROC
#define PFNglVertexAttrib4usvPROC PFNGLVERTEXATTRIB4USVPROC
#define PFNglVertexAttribPointerPROC PFNGLVERTEXATTRIBPOINTERPROC

QGLDECLARE(glBlendEquationSeparate);
QGLDECLARE(glDrawBuffers);
QGLDECLARE(glStencilOpSeparate);
QGLDECLARE(glStencilFuncSeparate);
QGLDECLARE(glStencilMaskSeparate);
QGLDECLARE(glAttachShader);
QGLDECLARE(glBindAttribLocation);
QGLDECLARE(glCompileShader);
QGLDECLARE(glCreateProgram);
QGLDECLARE(glCreateShader);
QGLDECLARE(glDeleteProgram);
QGLDECLARE(glDeleteShader);
QGLDECLARE(glDetachShader);
QGLDECLARE(glDisableVertexAttribArray);
QGLDECLARE(glEnableVertexAttribArray);
QGLDECLARE(glGetActiveAttrib);
QGLDECLARE(glGetActiveUniform);
QGLDECLARE(glGetAttachedShaders);
QGLDECLARE(glGetAttribLocation);
QGLDECLARE(glGetProgramiv);
QGLDECLARE(glGetProgramInfoLog);
QGLDECLARE(glGetShaderiv);
QGLDECLARE(glGetShaderInfoLog);
QGLDECLARE(glGetShaderSource);
QGLDECLARE(glGetUniformLocation);
QGLDECLARE(glGetUniformfv);
QGLDECLARE(glGetUniformiv);
QGLDECLARE(glGetVertexAttribdv);
QGLDECLARE(glGetVertexAttribfv);
QGLDECLARE(glGetVertexAttribiv);
QGLDECLARE(glGetVertexAttribPointerv);
QGLDECLARE(glIsProgram);
QGLDECLARE(glIsShader);
QGLDECLARE(glLinkProgram);
QGLDECLARE(glShaderSource);
QGLDECLARE(glUseProgram);
QGLDECLARE(glUniform1f);
QGLDECLARE(glUniform2f);
QGLDECLARE(glUniform3f);
QGLDECLARE(glUniform4f);
QGLDECLARE(glUniform1i);
QGLDECLARE(glUniform2i);
QGLDECLARE(glUniform3i);
QGLDECLARE(glUniform4i);
QGLDECLARE(glUniform1fv);
QGLDECLARE(glUniform2fv);
QGLDECLARE(glUniform3fv);
QGLDECLARE(glUniform4fv);
QGLDECLARE(glUniform1iv);
QGLDECLARE(glUniform2iv);
QGLDECLARE(glUniform3iv);
QGLDECLARE(glUniform4iv);
QGLDECLARE(glUniformMatrix2fv);
QGLDECLARE(glUniformMatrix3fv);
QGLDECLARE(glUniformMatrix4fv);
QGLDECLARE(glValidateProgram);
QGLDECLARE(glVertexAttrib1d);
QGLDECLARE(glVertexAttrib1dv);
QGLDECLARE(glVertexAttrib1f);
QGLDECLARE(glVertexAttrib1fv);
QGLDECLARE(glVertexAttrib1s);
QGLDECLARE(glVertexAttrib1sv);
QGLDECLARE(glVertexAttrib2d);
QGLDECLARE(glVertexAttrib2dv);
QGLDECLARE(glVertexAttrib2f);
QGLDECLARE(glVertexAttrib2fv);
QGLDECLARE(glVertexAttrib2s);
QGLDECLARE(glVertexAttrib2sv);
QGLDECLARE(glVertexAttrib3d);
QGLDECLARE(glVertexAttrib3dv);
QGLDECLARE(glVertexAttrib3f);
QGLDECLARE(glVertexAttrib3fv);
QGLDECLARE(glVertexAttrib3s);
QGLDECLARE(glVertexAttrib3sv);
QGLDECLARE(glVertexAttrib4Nbv);
QGLDECLARE(glVertexAttrib4Niv);
QGLDECLARE(glVertexAttrib4Nsv);
QGLDECLARE(glVertexAttrib4Nub);
QGLDECLARE(glVertexAttrib4Nubv);
QGLDECLARE(glVertexAttrib4Nuiv);
QGLDECLARE(glVertexAttrib4Nusv);
QGLDECLARE(glVertexAttrib4bv);
QGLDECLARE(glVertexAttrib4d);
QGLDECLARE(glVertexAttrib4dv);
QGLDECLARE(glVertexAttrib4f);
QGLDECLARE(glVertexAttrib4fv);
QGLDECLARE(glVertexAttrib4iv);
QGLDECLARE(glVertexAttrib4s);
QGLDECLARE(glVertexAttrib4sv);
QGLDECLARE(glVertexAttrib4ubv);
QGLDECLARE(glVertexAttrib4uiv);
QGLDECLARE(glVertexAttrib4usv);
QGLDECLARE(glVertexAttribPointer);

// OpenGL 2.1

#define PFNglUniformMatrix2x3fvPROC PFNGLUNIFORMMATRIX2X3FVPROC
#define PFNglUniformMatrix3x2fvPROC PFNGLUNIFORMMATRIX3X2FVPROC
#define PFNglUniformMatrix2x4fvPROC PFNGLUNIFORMMATRIX2X4FVPROC
#define PFNglUniformMatrix4x2fvPROC PFNGLUNIFORMMATRIX4X2FVPROC
#define PFNglUniformMatrix3x4fvPROC PFNGLUNIFORMMATRIX3X4FVPROC
#define PFNglUniformMatrix4x3fvPROC PFNGLUNIFORMMATRIX4X3FVPROC

QGLDECLARE(glUniformMatrix2x3fv);
QGLDECLARE(glUniformMatrix3x2fv);
QGLDECLARE(glUniformMatrix2x4fv);
QGLDECLARE(glUniformMatrix4x2fv);
QGLDECLARE(glUniformMatrix3x4fv);
QGLDECLARE(glUniformMatrix4x3fv);

// OpenGL 3.0

#define PFNglColorMaskiPROC PFNGLCOLORMASKIPROC
#define PFNglGetBooleani_vPROC PFNGLGETBOOLEANI_VPROC
#define PFNglGetIntegeri_vPROC PFNGLGETINTEGERI_VPROC
#define PFNglEnableiPROC PFNGLENABLEIPROC
#define PFNglDisableiPROC PFNGLDISABLEIPROC
#define PFNglIsEnablediPROC PFNGLISENABLEDIPROC
#define PFNglBeginTransformFeedbackPROC PFNGLBEGINTRANSFORMFEEDBACKPROC
#define PFNglEndTransformFeedbackPROC PFNGLENDTRANSFORMFEEDBACKPROC
#define PFNglBindBufferRangePROC PFNGLBINDBUFFERRANGEPROC
#define PFNglBindBufferBasePROC PFNGLBINDBUFFERBASEPROC
#define PFNglTransformFeedbackVaryingsPROC PFNGLTRANSFORMFEEDBACKVARYINGSPROC
#define PFNglGetTransformFeedbackVaryingPROC PFNGLGETTRANSFORMFEEDBACKVARYINGPROC
#define PFNglClampColorPROC PFNGLCLAMPCOLORPROC
#define PFNglBeginConditionalRenderPROC PFNGLBEGINCONDITIONALRENDERPROC
#define PFNglEndConditionalRenderPROC PFNGLENDCONDITIONALRENDERPROC
#define PFNglVertexAttribIPointerPROC PFNGLVERTEXATTRIBIPOINTERPROC
#define PFNglGetVertexAttribIivPROC PFNGLGETVERTEXATTRIBIIVPROC
#define PFNglGetVertexAttribIuivPROC PFNGLGETVERTEXATTRIBIUIVPROC
#define PFNglVertexAttribI1iPROC PFNGLVERTEXATTRIBI1IPROC
#define PFNglVertexAttribI2iPROC PFNGLVERTEXATTRIBI2IPROC
#define PFNglVertexAttribI3iPROC PFNGLVERTEXATTRIBI3IPROC
#define PFNglVertexAttribI4iPROC PFNGLVERTEXATTRIBI4IPROC
#define PFNglVertexAttribI1uiPROC PFNGLVERTEXATTRIBI1UIPROC
#define PFNglVertexAttribI2uiPROC PFNGLVERTEXATTRIBI2UIPROC
#define PFNglVertexAttribI3uiPROC PFNGLVERTEXATTRIBI3UIPROC
#define PFNglVertexAttribI4uiPROC PFNGLVERTEXATTRIBI4UIPROC
#define PFNglVertexAttribI1ivPROC PFNGLVERTEXATTRIBI1IVPROC
#define PFNglVertexAttribI2ivPROC PFNGLVERTEXATTRIBI2IVPROC
#define PFNglVertexAttribI3ivPROC PFNGLVERTEXATTRIBI3IVPROC
#define PFNglVertexAttribI4ivPROC PFNGLVERTEXATTRIBI4IVPROC
#define PFNglVertexAttribI1uivPROC PFNGLVERTEXATTRIBI1UIVPROC
#define PFNglVertexAttribI2uivPROC PFNGLVERTEXATTRIBI2UIVPROC
#define PFNglVertexAttribI3uivPROC PFNGLVERTEXATTRIBI3UIVPROC
#define PFNglVertexAttribI4uivPROC PFNGLVERTEXATTRIBI4UIVPROC
#define PFNglVertexAttribI4bvPROC PFNGLVERTEXATTRIBI4BVPROC
#define PFNglVertexAttribI4svPROC PFNGLVERTEXATTRIBI4SVPROC
#define PFNglVertexAttribI4ubvPROC PFNGLVERTEXATTRIBI4UBVPROC
#define PFNglVertexAttribI4usvPROC PFNGLVERTEXATTRIBI4USVPROC
#define PFNglGetUniformuivPROC PFNGLGETUNIFORMUIVPROC
#define PFNglBindFragDataLocationPROC PFNGLBINDFRAGDATALOCATIONPROC
#define PFNglGetFragDataLocationPROC PFNGLGETFRAGDATALOCATIONPROC
#define PFNglUniform1uiPROC PFNGLUNIFORM1UIPROC
#define PFNglUniform2uiPROC PFNGLUNIFORM2UIPROC
#define PFNglUniform3uiPROC PFNGLUNIFORM3UIPROC
#define PFNglUniform4uiPROC PFNGLUNIFORM4UIPROC
#define PFNglUniform1uivPROC PFNGLUNIFORM1UIVPROC
#define PFNglUniform2uivPROC PFNGLUNIFORM2UIVPROC
#define PFNglUniform3uivPROC PFNGLUNIFORM3UIVPROC
#define PFNglUniform4uivPROC PFNGLUNIFORM4UIVPROC
#define PFNglTexParameterIivPROC PFNGLTEXPARAMETERIIVPROC
#define PFNglTexParameterIuivPROC PFNGLTEXPARAMETERIUIVPROC
#define PFNglGetTexParameterIivPROC PFNGLGETTEXPARAMETERIIVPROC
#define PFNglGetTexParameterIuivPROC PFNGLGETTEXPARAMETERIUIVPROC
#define PFNglClearBufferivPROC PFNGLCLEARBUFFERIVPROC
#define PFNglClearBufferuivPROC PFNGLCLEARBUFFERUIVPROC
#define PFNglClearBufferfvPROC PFNGLCLEARBUFFERFVPROC
#define PFNglClearBufferfiPROC PFNGLCLEARBUFFERFIPROC
#define PFNglGetStringiPROC PFNGLGETSTRINGIPROC
#define PFNglIsRenderbufferPROC PFNGLISRENDERBUFFERPROC
#define PFNglBindRenderbufferPROC PFNGLBINDRENDERBUFFERPROC
#define PFNglDeleteRenderbuffersPROC PFNGLDELETERENDERBUFFERSPROC
#define PFNglGenRenderbuffersPROC PFNGLGENRENDERBUFFERSPROC
#define PFNglRenderbufferStoragePROC PFNGLRENDERBUFFERSTORAGEPROC
#define PFNglGetRenderbufferParameterivPROC PFNGLGETRENDERBUFFERPARAMETERIVPROC
#define PFNglIsFramebufferPROC PFNGLISFRAMEBUFFERPROC
#define PFNglBindFramebufferPROC PFNGLBINDFRAMEBUFFERPROC
#define PFNglDeleteFramebuffersPROC PFNGLDELETEFRAMEBUFFERSPROC
#define PFNglGenFramebuffersPROC PFNGLGENFRAMEBUFFERSPROC
#define PFNglCheckFramebufferStatusPROC PFNGLCHECKFRAMEBUFFERSTATUSPROC
#define PFNglFramebufferTexture1DPROC PFNGLFRAMEBUFFERTEXTURE1DPROC
#define PFNglFramebufferTexture2DPROC PFNGLFRAMEBUFFERTEXTURE2DPROC
#define PFNglFramebufferTexture3DPROC PFNGLFRAMEBUFFERTEXTURE3DPROC
#define PFNglFramebufferRenderbufferPROC PFNGLFRAMEBUFFERRENDERBUFFERPROC
#define PFNglGetFramebufferAttachmentParameterivPROC PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC
#define PFNglGenerateMipmapPROC PFNGLGENERATEMIPMAPPROC
#define PFNglBlitFramebufferPROC PFNGLBLITFRAMEBUFFERPROC
#define PFNglRenderbufferStorageMultisamplePROC PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC
#define PFNglFramebufferTextureLayerPROC PFNGLFRAMEBUFFERTEXTURELAYERPROC
#define PFNglMapBufferRangePROC PFNGLMAPBUFFERRANGEPROC
#define PFNglFlushMappedBufferRangePROC PFNGLFLUSHMAPPEDBUFFERRANGEPROC
#define PFNglBindVertexArrayPROC PFNGLBINDVERTEXARRAYPROC
#define PFNglDeleteVertexArraysPROC PFNGLDELETEVERTEXARRAYSPROC
#define PFNglGenVertexArraysPROC PFNGLGENVERTEXARRAYSPROC
#define PFNglIsVertexArrayPROC PFNGLISVERTEXARRAYPROC

QGLDECLARE(glColorMaski);
QGLDECLARE(glGetBooleani_v);
QGLDECLARE(glGetIntegeri_v);
QGLDECLARE(glEnablei);
QGLDECLARE(glDisablei);
QGLDECLARE(glIsEnabledi);
QGLDECLARE(glBeginTransformFeedback);
QGLDECLARE(glEndTransformFeedback);
QGLDECLARE(glBindBufferRange);
QGLDECLARE(glBindBufferBase);
QGLDECLARE(glTransformFeedbackVaryings);
QGLDECLARE(glGetTransformFeedbackVarying);
QGLDECLARE(glClampColor);
QGLDECLARE(glBeginConditionalRender);
QGLDECLARE(glEndConditionalRender);
QGLDECLARE(glVertexAttribIPointer);
QGLDECLARE(glGetVertexAttribIiv);
QGLDECLARE(glGetVertexAttribIuiv);
QGLDECLARE(glVertexAttribI1i);
QGLDECLARE(glVertexAttribI2i);
QGLDECLARE(glVertexAttribI3i);
QGLDECLARE(glVertexAttribI4i);
QGLDECLARE(glVertexAttribI1ui);
QGLDECLARE(glVertexAttribI2ui);
QGLDECLARE(glVertexAttribI3ui);
QGLDECLARE(glVertexAttribI4ui);
QGLDECLARE(glVertexAttribI1iv);
QGLDECLARE(glVertexAttribI2iv);
QGLDECLARE(glVertexAttribI3iv);
QGLDECLARE(glVertexAttribI4iv);
QGLDECLARE(glVertexAttribI1uiv);
QGLDECLARE(glVertexAttribI2uiv);
QGLDECLARE(glVertexAttribI3uiv);
QGLDECLARE(glVertexAttribI4uiv);
QGLDECLARE(glVertexAttribI4bv);
QGLDECLARE(glVertexAttribI4sv);
QGLDECLARE(glVertexAttribI4ubv);
QGLDECLARE(glVertexAttribI4usv);
QGLDECLARE(glGetUniformuiv);
QGLDECLARE(glBindFragDataLocation);
QGLDECLARE(glGetFragDataLocation);
QGLDECLARE(glUniform1ui);
QGLDECLARE(glUniform2ui);
QGLDECLARE(glUniform3ui);
QGLDECLARE(glUniform4ui);
QGLDECLARE(glUniform1uiv);
QGLDECLARE(glUniform2uiv);
QGLDECLARE(glUniform3uiv);
QGLDECLARE(glUniform4uiv);
QGLDECLARE(glTexParameterIiv);
QGLDECLARE(glTexParameterIuiv);
QGLDECLARE(glGetTexParameterIiv);
QGLDECLARE(glGetTexParameterIuiv);
QGLDECLARE(glClearBufferiv);
QGLDECLARE(glClearBufferuiv);
QGLDECLARE(glClearBufferfv);
QGLDECLARE(glClearBufferfi);
QGLDECLARE(glGetStringi);
QGLDECLARE(glIsRenderbuffer);
QGLDECLARE(glBindRenderbuffer);
QGLDECLARE(glDeleteRenderbuffers);
QGLDECLARE(glGenRenderbuffers);
QGLDECLARE(glRenderbufferStorage);
QGLDECLARE(glGetRenderbufferParameteriv);
QGLDECLARE(glIsFramebuffer);
QGLDECLARE(glBindFramebuffer);
QGLDECLARE(glDeleteFramebuffers);
QGLDECLARE(glGenFramebuffers);
QGLDECLARE(glCheckFramebufferStatus);
QGLDECLARE(glFramebufferTexture1D);
QGLDECLARE(glFramebufferTexture2D);
QGLDECLARE(glFramebufferTexture3D);
QGLDECLARE(glFramebufferRenderbuffer);
QGLDECLARE(glGetFramebufferAttachmentParameteriv);
QGLDECLARE(glGenerateMipmap);
QGLDECLARE(glBlitFramebuffer);
QGLDECLARE(glRenderbufferStorageMultisample);
QGLDECLARE(glFramebufferTextureLayer);
QGLDECLARE(glMapBufferRange);
QGLDECLARE(glFlushMappedBufferRange);
QGLDECLARE(glBindVertexArray);
QGLDECLARE(glDeleteVertexArrays);
QGLDECLARE(glGenVertexArrays);
QGLDECLARE(glIsVertexArray);
