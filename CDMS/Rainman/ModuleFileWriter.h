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

#ifndef _MODULEFILEWRITER_H_
#define _MODULEFILEWRITER_H_

#include <string>

/*!
	Updates the values the Mod Studio GUI can edit (UIName, Description,
	TextureFE, TextureIcon and ModVersion) inside an existing .module file.

	Every other line is preserved exactly - comments, directives this build does
	not model, and the [data:...]/[attrib:...] sections of Company of Heroes
	modules - so rewriting a module cannot silently drop settings. Keys the file
	does not have yet are inserted right after [global] when the new value is not
	empty.

	The file is written to a temporary sibling first and swapped in only after a
	complete write, so a failure cannot leave a truncated module behind.

	\return true on success. On failure returns false and puts a message in sError.
*/
bool Rainman_UpdateModuleFile(const char* sFile,
	const char* sUIName, const char* sDescription,
	const char* sTextureFE, const char* sTextureIcon,
	long iVersionMajor, long iVersionMinor, long iVersionRevision,
	std::string& sError);

#endif
