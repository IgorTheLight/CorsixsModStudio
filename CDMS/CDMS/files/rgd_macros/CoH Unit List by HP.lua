unit_list = table()
campaign_file = "[/_\\]sp[/_%.\\]"
 
function each_file(rgd)
	if (not rgd.path:find(campaign_file)) and
		rgd:GET("GameData","health_ext","hitpoints") then
		unit_list:insert(rgd)
	end
end

function at_end()
	unit_list:sort( [ (a,b)
		| a.GameData.health_ext.hitpoints
		> b.GameData.health_ext.hitpoints
	] )
	local pos = 0
	local prev_hp
	for _,rgd in ipairs(unit_list)
		local unit_name = rgd.path:after "ebps\\"
		screen_name = rgd.GameData:GET("ui_ext","screen_name")
		local hp = rgd.GameData.health_ext.hitpoints
		local unique = true
		if screen_name and screen_name != "$0"
			screen_name = UCS(screen_name)
			unit_name = screen_name.." ("..unit_name..")"
			if unit_list[screen_name] == hp
				unique = false
			else
				unit_list[screen_name] = hp
			end
		end
		if unique
			if hp ~= prev_hp
				pos = pos + 1
			end
			prev_hp = hp
			print("#" .. pos .. ": " .. unit_name .. " ("
			.. rgd.GameData.health_ext.hitpoints .. " hp)")
		end
	end
end
