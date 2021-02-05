function tweak_squad(squad)
	if squad.from and #squad.from > 0
		local abilities = table()
		local buildings = table()
		local others = table()
		
		for _, filename in squad.from
			if filename_index[filename].object_type == "Ability"
				abilities:insert(filename)
			elseif filename_index[filename].object_type == "Building"
				buildings:insert(filename)
			else
				others:insert(filename)
			end
		end
		
		if #others == 0
			if #buildings == 0 and #abilities == 1
				local ability = filename_index[abilities[1]]
				squad.cost_title = "Creation Cost (from " .. ability.name .. " ability)"
				squad.cost = ability.cost
			elseif #buildings == 1
				local building = filename_index[buildings[1]]
				squad.cost_title = "Creation Cost (from " .. building.name .. " building)"
			end
		end
	end
end
