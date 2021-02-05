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

#ifndef _STRINGS_H_
#define _STRINGS_H_
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif
// ----------------------------
#define AppStr(name) g_sAppStr_AutoDef_ ## name
#define AppStrS(name, S) wxString(wxT("")).Append(AppStr(name ## _pre)).Append(S).Append(AppStr(name ## _post))
#ifndef _S_CPP_
#define S(name, val) extern wxString AppStr(name);
#define SE(name, val) extern wxString AppStr(name);
#else
#define S(name, val) wxString AppStr(name)(wxT(val));
#define SE(name, val) wxString AppStr(name)(val);
#endif
// Put the application's strings here
// _menu = text on menu
// _help = message in statusbar / tooltip
// _tabname = name of tab
// _caption = message at top of form/tab

/* General */
#ifdef _DEBUG
S(app_title, "Corsix\'s Mod Studio DEBUG")
#else
S(app_title, "Corsix\'s Mod Studio")
#endif
S(app_version, "0.5.5")
SE(app_name, wxString().Append(AppStr(app_title)).Append(wxT(" ")).Append(AppStr(app_version)))
S(app_filespath, ".\\Mod_Studio_Files\\")
SE(app_dictionariespath, wxString().Append(AppStr(app_filespath)).Append(wxT("dictionaries\\")))
SE(app_macrospath, wxString().Append(AppStr(app_filespath)).Append(wxT("rgd_macros\\")))
SE(app_luareffile, wxString().Append(AppStr(app_filespath)).Append(wxT("lua.dat")))
SE(app_bfxmapfile, wxString().Append(AppStr(app_filespath)).Append(wxT("bfx.dat")))
SE(app_scarreffile, wxString().Append(AppStr(app_filespath)).Append(wxT("scardoc.dat")))
SE(app_cohscarreffile, wxString().Append(AppStr(app_filespath)).Append(wxT("scardoc_coh.dat")))
SE(app_squishfile, wxString().Append(AppStr(app_filespath)).Append(wxT("squish")))
SE(app_lua5file, wxString().Append(AppStr(app_filespath)).Append(wxT("lua512")))
#ifdef _DEBUG
SE(app_lua51file, wxString().Append(AppStr(app_filespath)).Append(wxT("debug\\lua512d.dll")))
#else
SE(app_lua51file, wxString().Append(AppStr(app_filespath)).Append(wxT("lua512p.dll")))
#endif
S(app_helpurl, ".\\Mod_Studio_Files\\docs\\general-1.html")
S(app_backuperror, "The file could not be backed up. In the unlikely event that the save operation corrupts the file, a backup would be useful. If you wish to make a backup, do it manually now.")
S(app_backuperror_title, "Backup before Saving")
S(app_backupcannotrestore, "The backup of the file could not be restored. You will have to restore it manually\nBackup file: ")
S(err_memory, "Memory allocation error")
S(err_couldnotopenoutput, "File could not be opened for writing")
S(err_write, "Failed to save file")
S(err_unknown, "Unknown error")
S(err_read, "Failed to read file")
S(err_readmem, "Failed to read memory")
S(err_luasyntax, "Invalid Lua syntax")
S(err_luaapi, "Lua API error")
S(err_lualoop, "Circular reference detected")
S(err_luarun, "Unexpected Lua runtime exception")
S(err_luaprocess, "Error processing Lua")
S(err_nogamedata, "No GameData table was found")
S(err_cannotcopy, "Error transferring data from Lua format to RGD format")

/* RGD Macros */
S(rgdmacro_title, "Run RGD Macro")
S(rgdmacro_caption, "Enter macro to be run over RGD files:")
S(rgdmacro_captionout, "Output of macro:")
S(rgdmacro_cancel, "Cancel")
S(rgdmacro_run, "Run Macro")
S(rgdmacro_loadmacro, "Load Macro")
S(rgdmacro_loadmacromsg, "Choose a macro to load")
S(rgdmacro_savemacromsg, "Choose where to save macro")
S(rgdmacro_saveoutputmsg, "Choose where to save output")
S(rgdmacro_loadmacrofilter, "RGD Macros (*.lua)|*.lua|All files (*.*)|*.*")
S(rgdmacro_saveoutputfilter, "Text Files (*.txt)|*.txt|X/HTML Files (*.html;*.htm;*.xml)|*.html;*.htm;*.xml|All files (*.*)|*.*")
S(rgdmacro_savemacro, "Save Macro")
S(rgdmacro_saveoutput, "Save Output")
S(rgdmacro_gotooutput, "Show Output")
S(rgdmacro_gotomacro, "Show Macro")

/* New mod */
S(newmod_name, "Name:")
S(newmod_namehelp, "Name of the new mod")
S(newmod_base, "Mod is for:")
S(newmod_basehelp, "What game to make this mod for")
S(newmod_destination, "Game folder:")
S(newmod_destinationhelp, "Where the mod will be created")
S(newmod_create, "Create")
S(newmod_cancel, "Cancel")
S(newmod_error, "Error while creating new mod")

/* Config */
SE(configfile_name, wxString().Append(AppStr(app_filespath)).Append(wxT("config.txt")))
S(config_colour_rpost, "/r")
S(config_colour_gpost, "/g")
S(config_colour_bpost, "/b")
S(config_firstrun, "/general/firstrun")
S(config_dowfolder, "/general/last_dow_folder")
S(config_dcfolder, "/general/last_dc_folder")
S(config_ssfolder, "/general/last_ss_folder")
S(config_cohfolder, "/general/last_coh_folder")
S(config_cohoffolder, "/general/last_cohof_folder")
S(config_donate, "/general/show_donate")
S(config_modtoolsfolder, "/general/mod_tools_folder")
S(config_sashposition, "/general/sash_position")
S(config_initialpath, "/null")
S(config_mod_locale, "locale")
S(config_mod_localeremember, "remember_locale")
S(config_mod_ucsrangeremember, "remember_outofucsrange")

/* RGD Editor */
S(rgd_errortitle, "Error")
S(rgd_namecantchange, "Name could not be changed")
S(rgd_dtcantchange, "Data type could not be changed")
S(rgd_valuecantchange, "Value could not be changed")
S(rgd_hashname, "Name cannot be changed because it is a hex value\nPlease update an RGD dictionary if you want to rename this")
S(rgd_cantaddchild, "Child could not be added")
S(rgd_cantdelete, "Could not delete item")
S(rgd_cannotcopy, "Cannot copy item")
S(rgd_cannotopenclip, "Cannot open clipboard")
S(rgd_save, "Save")
S(rgd_savegood, "File saved")
S(rgd_burngood, "The Lua file has been converted to RGD")
S(rgd_massburngood, "Lua files have been converted to RGD")

/* BFX / deburning */
S(bfx_convertgood, "BFX has been un-burned to LUA file")
S(bfxrgt_convertgood, "BFX and RGT files have been un-burned to LUA file")
S(rgodeburn_good, "RGO has been unburnt")

/* RGT / TGA / DDS */
S(rgt_convertgood, "RGT has been converted to: ")
S(rgt_makefromddsgood, "DDS converted to RGT")

/* Refresh files tool */
S(refreshfiles_name, "Refresh file list")

/* UCS to DAT tool */
S(aesetup_name, "UCS to DAT for CoH AE")
S(aesetup_help, "Converts a mod\'s UCS files to DAT files for use with the CoH/OF Attribute Editor")
S(aesetup_outselect, "DAT to create:")
S(aesetup_outselect_help, "Name of the DAT file to create or overwrite")
S(aesetup_browse, "Browse...")
S(aesetup_range1, "Range start:")
S(aesetup_range2, "Range end:")
S(aesetup_range_help, "All UCS entries within the given range will be saved to a DAT file")
S(aesetup_filter, "DAT files (*.dat)|*.dat|All files (*.*)|*.*")
S(aesetup_novalue, "The output filename is blank")
S(aesetup_norange, "Invalid range specified")
S(aesetup_message, "The DAT file is now being created...\nThis could take a long time depending on the number and size of UCS entries.")
S(aesetup_done, "DAT has been created")

/* SCAR Editor */
S(scar_checklua, "Check Lua")
S(scar_save, "Save")
S(scar_luagood, "No errors in Lua syntax")
S(scar_bad, "Error around line %lu: %S")
S(scar_savegood, "File saved")
S(scar_funcdrop, "Jump to function")

/* Welcome Form */
S(welcome_tabname, "Welcome")
SE(welcome_caption, wxString(wxT("Welcome to ")).Append(AppStr(app_title)).Append(wxT(". What would you like to do?")))
S(donate_help, "Please donate to ensure continued development :)")
S(welcome_firstrun, "This is the first time you\'ve launched the application. It is strongly recommended to read the documentation so that you know how to use the program, and any important changes since previous versions.\n Would you like to read the documentation?")

/* Files Form */
S(file_filestabname, "Files")
S(file_toolstabname, "Tools")
S(file_folder, " (Folder)\nCannot be put into a\nsingle mod or source")
S(file_modname, "Part of mod: ")
S(file_sourcename, "Location: ")
S(file_colourthismod, "/files/ColourForFilesInThisMod")
S(file_colourothermod, "/files/ColourForFilesInOtherMods")
S(file_colourengine, "/files/ColourForFilesInEngine")
S(file_newucs_title, "New UCS")
S(file_newucs_dup_caption, "A UCS file with that name already exists")
S(file_newucs_errormake_caption, "The UCS file could not be created")

/* Locale Select */
S(localeselect_caption, "Select a locale to use for this mod: \n (Changes only take effect when the mod is loaded)")
S(localeselect_use, "Use Locale")
S(localeselect_remember, "Remember for this mod")
S(localeselect_title, "Locale")
S(localeselect_default, "English")

/* SGA Packer */
S(sgapack_title, "SGA Packer")
S(sgapack_dirselect, "Select an input directory:")
S(sgapack_dirselect_label, "Input directory:")
S(sgapack_browse, "Browse ...")
S(sgapack_dirselect_label_help, "The entire contents of this directory is turned into an SGA")
S(sgapack_outselect, "SGA to create:")
S(sgapack_outselect_help, "Name of the SGA file to create or overwrite")
S(sgapack_filter, "SGA Archives (*.sga)|*.sga|All files (*.*)|*.*")
S(sgapack_message, "SGA is now being created...\nThis could take a long time depending on the number and size of files.")
S(sgapack_done, "SGA has been created")
S(sgapack_toc, "TOC name:")
S(sgapack_toc_help, "The table of contents name for the created SGA")
S(sgapack_novalue, "One or more fields are blank")

/* DPS Calculator */
S(dpscalculator_title, "DoW DPS Calculator")
S(dpscalculator_message, "DPS is now being calculated...\nThis could take several minutes.")
S(dpscalculator_outselect, "Choose where to save results")
S(dpscalculator_filter, "HTML File (*.html)|*.html")
S(dpscalculator_done, "DPS has been calculated")

/* UCS Editor Form */
S(ucsedit_tabname, "UCS")
S(ucsedit_newentry, "New Entry")
S(ucsedit_save, "Save")
S(ucsedit_readonlyerror, "This UCS file is read-only; try loading (or creating) a different UCS file")
S(ucsedit_newentrycaption, "Enter ID of new entry:")
// Used when UCS file has no entries
// Set to a currently unassigned value in the modder's range
S(ucsedit_newentrydefault, "$18010000")
S(ucsedit_newentryduptitle, "Duplicate Entry")
S(ucsedit_newentrydupcaption, "That entry is already in this file.")
S(ucsedit_save_title, "Save")
S(ucsedit_load_title, "Load / create UCS file")
S(ucsedit_cannotsave_caption, "File could not be saved; check that the file is not read-only and that you have sufficient user rights.\nA backup of the original file has been created which will now be restored from")
S(ucsedit_save_caption, "File saved")
S(ucsedit_rgdcancel, "Back to RGD")
S(ucsedit_rgdapply, "Send selected entry to RGD")

/* UCS Out of Range Form */
S(ucsrange_caption, "$%lu is outside of the modders range ($15000000 to $20000000).\nCreate entry anyway?")
S(ucsrange_remember, "This mod is an official mod or TC mod")
S(ucsrange_title, "New UCS Entry")
S(ucsrange_yes, "Yes")
S(ucsrange_no, "No")

/* UCS Selector Form */
S(ucsselect_caption, "Select a UCS file to open: ")
S(ucsselect_title, "UCS Selector")
S(ucsselect_new, "New")
S(ucsselect_open, "Load")
S(ucsselect_close, "Cancel")
S(ucsselect_newcaption, "Enter name of new file:")

/* Main Menu */
S(file_menu, "&File")
	S(new_mod, "New Mod")
	S(new_mod_menu, "&New Mod\tCTRL+N")
	S(new_mod_help, "Create a new mod for DoW / WA / DC / SS / CoH / OF")
	S(open_mod, "Load DoW/WA Mod")
	S(open_mod_menu, "&Open DoW/WA Mod\tCTRL+O")
	S(open_mod_help, "Open an existing mod for Dawn of War / Winter Assault")
	S(open_sga, "Load Single SGA Archive")
	S(open_sga_menu, "Open &SGA Archive")
	S(open_sga_help, "Open a single SGA data archive")
	S(open_moddc, "Load DoW:DC Mod")
	S(open_moddc_menu, "Open &DoW:DC Mod\tCTRL+SHIFT+D")
	S(open_moddc_help, "Open an existing mod for Dawn of War: Dark Crusade")
	S(open_modss, "Load DoW:SS Mod")
	S(open_modss_menu, "Open &DoW:SS Mod")
	S(open_modss_help, "Open an existing mod for Dawn of War: Soulstorm")
	S(open_modcoh, "Load CoH/OF Mod")
	S(open_modcoh_menu, "Open &CoH/OF Mod\tCTRL+SHIFT+O")
	S(open_modcoh_help, "Open an existing mod for Company of Heroes / Opposing Fronts")

	S(close_mod_menu, "&Close Mod")
	S(close_mod_help, "Close the current mod")
	S(exit, "Quit")
	S(exit_menu, "E&xit")
	SE(exit_help, wxString(wxT("Close ")).Append(AppStr(app_title)))
S(tools_menu, "&Tools")
	S(tools_help, "Helpful utilities")
	S(locale, "Locale")
	S(locale_menu, "&Locale")
	S(locale_help, "Change which set of UCS files to load")
	S(ucs_editor, "UCS Editor")
	S(ucs_editor_menu, "&UCS Editor")
	S(ucs_editor_help, "Edit UCS (localization text) files")
	S(xml_export, "Attrib Snapshot")
	S(xml_export_menu, "Attrib Snapshot")
	S(xml_export_help, "Save a snapshot of RGDs to XML")
S(relic_tools_menu, "&Relic\'s Tools")
	S(attr_editor_menu, "&Attribute Editor")
	S(attr_editor_help, "Relic\'s LUA editor and burner")
	S(audio_editor_menu, "A&udio Editor")
	S(audio_editor_help, "Relic\'s audio and sound editor")
	S(chunky_view_menu, "&Chunky Viewer")
	S(chunky_view_help, "Basic viewer for most \"Relic Chunky\" files")
	S(fx_tools_menu, "&FX Tools")
	S(fx_tools_help, "Relic\'s (special) effects tool")
	S(mission_edit_menu, "&Mission Editor")
	S(mission_edit_help, "Relic\'s map and mission editor")
	S(mod_packager_menu, "Mod &Packager")
	S(mod_packager_help, "Relic\'s SGA (file archive) utility")
	S(object_editor_menu, "&Object Editor")
	S(object_editor_help, "Relic\'s ingame object editor")
S(play_menu, "&Play")
	S(play_coh, "Play in CoH/OF")
	S(play_coh_help, "Play using RelicCOH.exe")
	S(play_w40k, "Play in DoW")
	S(play_w40k_help, "Play using W40k.exe")
	S(play_wxp, "Play in DoW:WA")
	S(play_wxp_help, "Play using W40kWA.exe")
	S(play_dc, "Play in DoW:DC")
	S(play_dc_help, "Play using DarkCrusade.exe")
	S(play_ss, "Play in DoW:SS")
	S(play_ss_help, "Play using Soulstorm.exe")
	S(play_warn, "View warnings.log")
	S(play_warn_help, "Open the warnings log in notepad")
	S(play_dev, "Run in dev mode")
	S(play_dev_help, "Play with the -dev switch enabled")
	S(play_nomov, "Skip movies")
	S(play_nomov_help, "Play with the -nomovies switch enabled")
S(help_menu, "&Help")
	S(help_index, "View Help")
	S(help_index_menu, "&Contents\tF1")
	SE(help_index_help, wxString().Append(AppStr(app_title)).Append(wxT(" help pages")))
	S(lua_ref_menu, "&LUA Reference")
	S(lua_ref_help, "Reference for LUAs in the attrib folder")
	S(forum_dow_menu, "DoW/WA/DC modding forum")
	S(forum_dow_help, "The Adeptus Modificatus forum at http://forums.relicnews.com")
	S(forum_coh_menu, "CoH modding forum")
	S(forum_coh_help, "The Armoury forum at http://forums.relicnews.com")
	S(rdn_new_wiki_menu, "New RDN Wiki")
	S(rdn_new_wiki_help, "The Relic CoH/DoW modding wiki at http://wiki.relicrank.com")
	S(rdn_wiki_menu, "&Old RDN Wiki")
	S(rdn_wiki_help, "The Relic Developer Network's DoW modding wiki at http://www.relic.com/rdn/wiki")
	S(kresjah_wiki_menu, "&Kresjah\'s Wiki")
	S(kresjah_wiki_help, "Kresjah's DoW modding wiki")
	S(about_menu, "&About")
	SE(about_help, wxString(wxT("About ")).Append(AppStr(app_title)))
	S(credits_menu, "&Credits")
	SE(credits_help, wxString(wxT("The people who brought you ")).Append(AppStr(app_title)))
	S(hidedonate_menu, "Hide Donate Button")
	S(hidedonate_help, "Hide the donation button when the program starts")
	S(hidedonate_text, "Thankyou for donating, the donate button will be hidden after program restart.\n(Of course, if you hide the button but haven\'t donated.....)")

/* Module */
S(mod_menu, "&Mod")
S(mod_tabname, "Module")
S(mod_select_file, "Select a mod to open")
S(mod_select_sga, "Select an archive to open")
S(mod_loading, "Loading Mod")
S(mod_load_error, "Mod could not be loaded")
S(mod_file_filter, "Module Files (*.module)|*.module|All Files|*")
S(mod_sga_filter, "SGA Archives (*.sga)|*.sga|All Files|*")
S(mod_properties_menu, "&Properties")
S(mod_properties_help, "Name, DLL, RequiredMods ...")
S(mod_description, "Description: ")
S(mod_description_help, "Short description of the mod")
S(mod_dll, "DLL Name: ")
S(mod_dll_help, "The DLL that this mod should use")
S(mod_dll_filter, "DLL files (*.dll)|*.dll|EXE files (*.exe)|*.exe|All files (*.*)|*")
S(mod_select_dll, "Select a DLL or EXE to find the hashes in")
S(mod_folder, "Mod Folder: ")
S(mod_folder_help, "The folder in which all of the mod\'s files are in")
S(mod_texturefe, "Texture FE: ")
S(mod_texturefe_help, "The front end (menu) texture")
S(mod_textureicon, "Texture Icon: ")
S(mod_textureicon_help, "The icon texture")
S(mod_uiname, "UI Name: ")
S(mod_uiname_help, "The name of mod, as shown in the mod manager menu")
S(mod_version, "Version: ")
S(mod_version_help, "The version number (AAA.BBB.CCC) of the mod - numbers only")
S(mod_datafolders_caption, "Data folders used by this mod:")
S(mod_datafolder, "data folder")
S(mod_add_a_new_pre, "Add a new ")
S(mod_add_a_new_post, "")
S(mod_remove_an_existing_pre, "Remove an existing ")
S(mod_remove_an_existing_post, "")
S(mod_move_up_pre, "Move the selected ")
S(mod_move_up_post, " up")
S(mod_move_down_pre, "Move the selected ")
S(mod_move_down_post, " down")
S(mod_changes_to_pre, "Changes to ")
S(mod_changes_to_post, "s will not take affect\nuntil you save the mod")
S(mod_dataarchives_caption, "Data archives used by this mod:")
S(mod_dataarchive, "data archive")
S(mod_requiredmods_caption, "Other mods required by this mod:")
S(mod_requiredmod, "required mod")
S(mod_compatiblemods_caption, "Other mods compatible with this mod:")
S(mod_compatiblemod, "compatible mod")

/* Misc */
SE(statusbar_message_default, wxString(wxT("Welcome to ")).Append(AppStr(app_title)))
S(decimal_point, ".")
S(question_mark, "?")

/* Extract tool */
S(extract_failed, "Failed to extract file")
S(extract_good, "File extracted")
S(extract_already, "File is already extracted")
S(massext_title, "Extract files")
S(massext_advanced_show, "Advanced")
S(massext_advanced_hide, "Simple")
S(massext_go, "Go!")
S(massext_cancel, "Cancel")
S(massext_selectall, "Select all")
S(massext_done, "Files extracted")
S(massext_caption, "All files will be extracted")
S(massext_caption_adv, "Only files from specified locations will be extracted")
S(massext_toolname, "Extract all")

/* RGD -> Lua tool */
S(luadump_failed, "Failed to dump RGD to lua")
S(luadump_good, "RDG dumped to lua")

/* Red button */
S(redbutton_toolname, "Developer\'s Red Button")

// End
#undef S
#endif