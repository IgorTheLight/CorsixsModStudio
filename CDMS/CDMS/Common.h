#ifndef _CDMS_COMMON_H_
#define _CDMS_COMMON_H_

#include "Construct.h"
#include "Utility.h"
#include "strconv.h"
#include <Rainman.h>
#include "strings.h"
#include <wx/wxprec.h>

#ifndef _MEM_DEBUG_H_
#define _MEM_DEBUG_H_
#ifdef _DEBUG
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
#else
#define DEBUG_NEW new
#endif
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#endif

#endif