/*
Rainman Library
Copyright (C) 2006 Corsix <corsix@gmail.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _CALLBACKS_H_
#define _CALLBACKS_H_

#include "gnuc_defines.h"
#include "Api.h"

//! A callback function
/*!
	\param[in] sMessage The message to be displayed by the function
	\param[in] pToken A token passed by the application
	\return No return value, cannot throw a CRainmanException
*/
#ifdef RAINMAN_GNUC
typedef void(*pfnStatusCallback)(const char*, void*);
#else
typedef void(__cdecl *pfnStatusCallback)(const char*, void*);
#endif

//! Macro to be placed in a function description that wants a callback
#define CALLBACK_ARG pfnStatusCallback fnStatusCallback, void* fnStatusCallbackTag

//! Macro to be placed in a function description that wants an optional callback
#define CALLBACK_ARG_OPT pfnStatusCallback fnStatusCallback = 0, void* fnStatusCallbackTag = 0

//! Easy way to call a callback with a formatted string
/*!
	\param[in] fnStatusCallback The callback function
	\param[in] fnStatusCallbackTag A token to be passed to the callback function
	\param[in] sFormat A format string to be given to sprintf()
	\param[in] ... Any values referenced in sFormat
	\return Returns nothing, but may throw a CRainmanException on error
	Example: <code>CallCallback(THE_CALLBACK, "Something said \'%s\'", "boo")</code>
*/
RAINMAN_API void CallCallback(CALLBACK_ARG, const char* sFormat, ...);

//! Macro to be used to pass a callback to another function (eg. to CallCallback() )
#define THE_CALLBACK fnStatusCallback, fnStatusCallbackTag

#endif

