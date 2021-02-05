/*
    This file is part of Corsix's Mod Studio.

    Corsix's Mod Studio is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Corsix's Mod Studio is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Corsix's Mod Studio; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _CONFIG_H_
#define _CONFIG_H_
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
#include <wx/aui/auibook.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif
// ----------------------------
#include <wx/confbase.h>

#define TheConfig wxConfigBase::Get()

wxColour ConfGetColour(wxString keyname, int def_r, int def_g, int def_b);

/*!
	may throw a CRainmanException
*/
wxString ConfGetDoWFolder();

/*!
	may throw a CRainmanException
*/
wxString ConfGetDCFolder();

/*!
	may throw a CRainmanException
*/
wxString ConfGetSSFolder();

/*!
	may throw a CRainmanException
*/
wxString ConfGetCoHFolder();

/*!
	may throw a CRainmanException
*/
wxString ConfGetModToolsFolder();

#endif