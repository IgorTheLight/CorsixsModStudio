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

#ifndef _RAINMAN_API_H_
#define _RAINMAN_API_H_
#include "gnuc_defines.h"

//#pragma warning( disable : 4251 )

#ifdef RAINMAN_GNUC
#define RAINMAN_NO_EXPORTS
#endif

#ifdef RAINMAN_NO_EXPORTS
#define RAINMAN_API
#else // RAINMAN_GNUC
#ifdef RAINMAN_EXPORTS
#define RAINMAN_API __declspec(dllexport)
#else // RAINMAN_EXPORTS
#define RAINMAN_API __declspec(dllimport)
#endif // RAINMAN_EXPORTS
#endif // RAINMAN_GNUC
#endif

