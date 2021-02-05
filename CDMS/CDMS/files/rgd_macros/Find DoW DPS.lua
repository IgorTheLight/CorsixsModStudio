-- Run this over the entire DoW/WA/DC Data\attrib folder
output_file = [[dps.html]]
-- ^^ change as needed ^^

-- 0. Set globals
armour_types = setmetatable({},{__index = [(t,k) t[k] = {} | rawget(t,k)]})

-- 1. Get a list of all files
function each_file(rgd)
	local map_part = _G
	local key = "filemap"
	for _,part in ipairs(rgd.path:lower():after"\\attrib\\":split"\\")
		map_part = map_part[key]
		key = part
	end
	map_part[key] = rgd
end

-- 2. Iterate over the RaceBPs folder
function at_end()
	print_info "<!-- Generating unit list ..."
	for filename,rgd in pairs_recursive(filemap.racebps) do
		if rgd:GET("GameData","race_details","playable") then
			print_info("Iterating unit list for " .. UCS(rgd:GET("GameData","race_details","name_id") or filename))
			do_ebps_folder(rgd:GET("GameData","race_path","unit_path"))
			do_ebps_folder(rgd:GET("GameData","race_path","building_path"))
		end
	end
	
	-- Sort armour types by name
	local armour_types_new = {}
	armour_type_names = {}
	for armour_type_name,unit_list in armour_types do
		table.sort(unit_list)
		local nicename = armour_type_name:after"tp_":beforelast".":gsub("_"," ")
		if nicename ~= "armour" then
			table.insert(armour_types_new, {units = unit_list, name = armour_type_name, nice_name = nicename})
			table.insert(armour_type_names, nicename)
		end
	end
	table.sort(armour_types_new, [(a,b) | a.nice_name < b.nice_name ])
	table.sort(armour_type_names)
	armour_types = armour_types_new
	
	-- Start output
	print_info "-->"
	print("<html><head><title>DoW DPS</title>" .. the_css .. "</head><body bgcolor=666666><table border=\"0\" cellpadding=3 cellspacing=1>")
	for _,armour_type in ipairs(armour_types)
		print("\t<tr>\n\t<th valign=top>" .. armour_type.nice_name .. "</th> <td bgcolor=999999>" .. table.concat(armour_type.units,", ") .. "</td>\n\t</tr>")
	end
	print "</table><br/><table border=\"0\" cellpadding=3 cellspacing=1>"
	
	return do_weapons()
end

-- 3. Iterate over the EBPs folder, getting the armour types of units
function do_ebps_folder(path)
	if not path then return nil end
	for filename,rgd in pairs_recursive(filemap_folder("ebps\\" .. path)) do	
		local armour_type = rgd:GET("GameData","type_ext","type_armour")
		if armour_type then
			local armour_table = armour_types[armour_type["$REF"]:lower()]
			local neat_filename = rgd.name:beforelast".":gsub("_", " ")
			local screen_name = rgd:GET("GameData","ui_ext","ui_info","screen_name_id")
			if screen_name and (screen_name ~= "$0") then
				neat_filename = UCS(screen_name) .. " (" .. neat_filename .. ")"
			end
			table.insert(armour_table, neat_filename)
		end
	end
end

-- 4. Iterate over the weapons folder
function do_weapons()
	local header_row_n = 23
	local weapons = {}
	for filename,rgd in pairs_recursive(filemap.weapon) do
		table.insert(weapons, rgd)
	end
	table.sort(weapons, [(a,b) | a.path < b.path ])
	
	for _,rgd in ipairs(weapons) do
		local armour_damage = rgd:GET("GameData","area_effect","weapon_damage","armour_damage")
		if armour_damage then
			local weapon = {}
			weapon.reload_time = rgd.GameData.reload_time
			if weapon.reload_time and weapon.reload_time > 0.0001 then
				weapon.accuracy = rgd.GameData.accuracy
				weapon.min_damage = armour_damage.min_damage_value
				weapon.damage = (armour_damage.max_damage + armour_damage.min_damage) / 2
				weapon.armour_piercing = armour_damage.armour_piercing
				weapon.ap = {}
				for key,ap_entry in armour_damage.armour_piercing_types do
					if key ~= "$REF" then
						weapon.ap[ap_entry.armour_type["$REF"]:lower()] = ap_entry.armour_piercing_value
					end
				end
				for key,armour_type in ipairs(armour_types) do
					local apval = math.clamp(weapon.ap[armour_type.name:lower()] or weapon.armour_piercing, 0, 100) / 100
					local damage = weapon.damage * apval
					if weapon.min_damage > damage then damage = weapon.min_damage end
					weapon.ap[key] = string.format("%.3f",(damage * weapon.accuracy) / weapon.reload_time)
				end
				
				-- Print
				header_row_n = header_row_n + 1
				if header_row_n == 24 then
					header_row_n = 0
					print("\t<tr>\n\t<th valign=top>File</th> <th valign=top>" .. table.concat(armour_type_names, "</th> <th valign=top>") .. "</th> \n\t</tr>")
				end
				local td = (((header_row_n % 2) == 1) and "<td bgcolor=aaaaaa>") or "<td bgcolor=999999>"
				print("\t<tr>\n\t" .. td .. rgd.name .. "</td> " .. td .. table.concat(weapon.ap, "</td> " .. td) .. "</td> \n\t</tr>")
			end
		end
	end
	print "</table></body></html>"
	filemap = nil
	if print_file
		print_file:close()
	end
	print_info "\n<!-- done -->"
end

-- Util: output
print_info = print
print_file = io.open(output_file, "w")
if print_file then
	print = [ | print_file:write(...,"\n") ]
else
	print("<!-- Unable to write output to " .. output_file .. "; writing here instead -->\n")
end

-- Util: filemap
function filemap_autotable(t,k)
	if filemap_autotable
		t[k] = setmetatable({}, {__index = filemap_autotable})
		return t[k]
	end
end
filemap_autotable(_G, "filemap")
 
function filemap_folder(path)
	local map_part = filemap
	for k,part in ipairs(path:lower():split"[/\\]")
		map_part = map_part[part]
	end
	return map_part
end

-- Util: tables
function pairs_recursive(t)
	| [ (state, v1)
		local k,v = next(state[#state], v1)
		local f = [
		if type(v) == "table"
			state[#state] = {state[#state], k}
			state[1+#state] = v
			k,v = next(state[#state])
			| 1
		elseif type(v) == "nil"
			state[#state] = nil
			if not state[#state] | nil end
			v1 = state[#state][2]
			state[#state] = state[#state][1]
			k,v = next(state[#state], v1)
			| 1
		end ]
		while f() end
		| k,v
	], {t}
end

-- Util : css
the_css = [==[
<style type="text/css">
<!-- 
a:link { color: #ffff00; text-decoration: none;}
a:visited { color: #ffff00; text-decoration: none;}
a:active { color: #ffff00; text-decoration: none;}
a:hover { color: #ffffff; text-decoration: underline;}

#bg {
	background: #000000;
	}

th {
	font-face: arial; font-size: 12px; color:ffffff;
      }

td { font-family: arial,helvetica; font-size: 11px; color: 000000; }


BODY {
	scrollbar-face-color: #333333;
	scrollbar-highlight-color: #666666;
	scrollbar-shadow-color: #000000;
	scrollbar-3dlight-color: #888888;
	scrollbar-arrow-color:  #F8A406;
	scrollbar-track-color: #060606;
	scrollbar-darkshadow-color: #000000;
font-family: arial,helvetica; font-size: 10px; color: cccccc;
-moz-scrollbar-face-color:#ffffff; -moz-scrollbar-arrow-color:#000000; -moz-scrollbar-track-color:#ffffff; -moz-scrollbar-shadow-color:#ffffff; -moz-scrollbar-highlight-color:#ffffff; -moz-scrollbar-3dlight-color:#ffffff; -moz-scrollbar-darkshadow-color:#000000; }

</style>
]==]