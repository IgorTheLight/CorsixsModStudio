function link_to_filename(filename, obj)
	local text = ((filename_index[filename] and filename_index[filename].name) or filename)
	if text == ""
		text = "(blank)"
	end
	local object_name = obj or ((filename_index[filename] and filename_index[filename].object_type) or "")
	if object_name != ""
		object_name = " " .. object_name
	end
	return "<a href=\"" .. filename .. "\">" .. text .. object_name .. "</a>"
end

template = {}

breadcrumbs_name = {}
breadcrumbs_name["sbps.html"] = "Squads"
breadcrumbs_name["sbps_races.html"] = ""
breadcrumbs_name["ebps.html"] = "Entities"
breadcrumbs_name["ebps_races.html"] = ""
breadcrumbs_name["racebps.html"] = "Armies"
breadcrumbs_name["type_cover.html"] = "Type (cover)"
breadcrumbs_name["type_unit_class.html"] = "Type (unit)"
breadcrumbs_name["type_target_weapon.html"] = "Type (armour)"
breadcrumbs_todo = {}

function template:head(title, category)
	local file = io.open(output_folder .. self.filename, "w")
	if not file
		print("Unable to open " .. output_folder .. self.filename)
		return
	end
	file:write([[
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">

<head>
<title>]] .. title .. [[</title>
    <link rel="stylesheet" type="text/css" href="style.css" media="screen" />
</head>

<body class="]] .. category .. [[">

<div id="header">
	<div id="navbar">
		<h1>
			<img src="coh-of-logo.png" alt="Company of Heroes: Opposing Fronts" />
		</h1>
		<ul>
			<li class="home"><a href="index.html">Home</a></li>
			<li class="armies"><a href="racebps.html">Armies</a></li>
			<li class="squads"><a href="sbps.html">Squads</a></li>
			<li class="buildings"><a href="type_unit_class_building.html">Buildings</a></li>
			<li class="vehicles"><a href="type_unit_class_vehicle.html">Vehicles</a></li>
		</ul>
	</div>
	<div id="breadcrumbs">]])
	local breadcrumbs = table(self.filepath:split"\\")
	breadcrumbs[#breadcrumbs] = breadcrumbs[#breadcrumbs]:beforelast"."
	breadcrumbs_new = table()
	local j = 0
	for i,part in ipairs(breadcrumbs)
		local url = breadcrumbs:concat("_",1,i) .. ".html"
		if breadcrumbs_name[url] == ""
			j = j + 1
		elseif breadcrumbs_name[url]
			breadcrumbs_todo[url] = breadcrumbs:concat("\\",1,i)
			breadcrumbs_new[i-j] = [[<a href="]] .. url .. [[">]] .. breadcrumbs_name[url] .. [[</a>]]
		elseif filename_index[url] and filename_index[url].name
			breadcrumbs_todo[url] = breadcrumbs:concat("\\",1,i)
			breadcrumbs_new[i-j] = [[<a href="]] .. url .. [[">]] .. filename_index[url].name .. [[</a>]]
		else
			breadcrumbs_todo[url] = breadcrumbs:concat("\\",1,i)
			breadcrumbs_new[i-j] = [[<a href="]] .. url .. [[">]] .. part:sub(1,1):upper() .. part:sub(2) .. [[</a>]]
		end
	end
	breadcrumbs_new:insert(1, [[<a href="index.html">Home</a>]])
	file:write(breadcrumbs_new:concat" » " .. [[</div>
</div>

<div id="content">
<div class="column" id="column_triple">
	<h2>]] .. title .. [[</h2>
]])
	return file
end
function template:foot(file)
	file:write([[</div>
<div id="footer">The information found on this page is covered by the Opposing Fronts Beta NDA; refer to the Opposing Fronts Beta EULA for details.
No guarantee is made that the information found on this page is correct or up to date.<br/>Page generated: ]] .. os.date() .. [[</div>
</div>

</body>

</html>]])
	file:close()
end

function template:race()
	local file = template.head(self, "Army: " .. self.name, "armies")
	file:write("<h3>Starts with (This list is bugged)</h3>")
	file:write(link_to_filename(self.hq) .. "<br/>")
	for _,v in self.starting_squads
		file:write(link_to_filename(v) .. "<br/>")
	end
	for k,v in self.starting_resources
		file:write(tostring(v) .. " " .. k .. "<br/>")
	end
	file:write("<h3>Commander Trees</h3>")
	for _,v in self.commander_trees
		file:write(link_to_filename(v,"") .. "<br/>")
	end
	file:write("<h3>Possible Command Abilities</h3>")
	for _,v in self.command_abilities
		file:write(link_to_filename(v,"") .. "<br/>")
	end
	template.foot(self, file)
end

function template:std(file)
	if self.cost and next(self.cost)
		file:write("<h3>" .. ( self.cost_title or "Cost" ) .. "</h3>")
		local is_free = true
		for k,v in self.cost
			if v > 0
				file:write(string.format("%.1f", v) .. " " .. k .. "<br/>")
				is_free = false
			end
		end
		if is_free
			file:write "Free<br/>"
		end
	end
	if self.abilities and #self.abilities > 0
		file:write("<h3>Possible Abilities</h3>")
		for _,val in self.abilities
			file:write(link_to_filename(val) .. "<br/>")
		end
	end
	if self.from and #self.from > 0
		file:write("<h3>" .. (self.from_title or "Can be created by") .. "</h3>")
		for _,v in self.from
			file:write(link_to_filename(v,self.from_ext) .. "<br/>")
		end
	end
	if self.requirements then template.requirements(self.requirements, file) end
	if self.requirement_of then template.requirement_of(self.requirement_of, file) end
	if self.member_of and #self.member_of > 0
		file:write("<h3>Member of</h3>")
		for _,val in self.member_of
			file:write(link_to_filename(val) .. "<br/>")
		end
	end
	if self.related_to and #self.related_to > 0
		file:write("<h3>Affected by</h3>")
		local dupes = {}
		for _,v in self.related_to
			local content = link_to_filename(v)
			if not dupes[content]
				file:write(content .. "<br/>")
				dupes[content] = true
			end
		end
	end
	if self.types and #self.types > 0
		file:write("<h3>Types</h3>")
		for _,val in self.types
			file:write("Is " .. link_to_filename(val) .. "<br/>")
		end
		if self.health["Armour Type"]
			file:write("Is " .. link_to_filename(self.health["Armour Type"]) .. "<br/>")
		end
	end
	local same_names = table()
	for filename, obj in filename_index
		if obj.name == self.name and obj != self
			if obj.object_type == self.object_type
				same_names:insert(filename)
			end
		end
	end
	if #same_names > 0
		local common_root = 0
		local common_good = true
		while common_good
			common_root = common_root + 1
			local root = self.filename:sub(1, common_root)
			for _,name in same_names
				if name:sub(1, common_root) != root
					common_good = false
					break
				end
			end
		end
		file:write("<h3>Do not confuse with</h3>This is \"~" .. self.filename:gsub("_"," "):beforelast".":sub(common_root) .. "\"<br/>")
		for _,name in same_names
			file:write(link_to_filename(name,"") .. " is \"~" .. name:gsub("_"," "):beforelast".":sub(common_root) .. "\"<br/>")
		end
	end
end

function template:commander_tree()
	local file = template.head(self, "Commander Tree: " .. self.name, "armies")
	if self.requirements then template.requirements(self.requirements, file) end
	if self.requirement_of then template.requirement_of(self.requirement_of, file) end
	if self.armies
		for _,army in self.armies
			file:write("For " .. link_to_filename(army) .. "<br/>")
		end
	end
	for _,branch in self.branches
		for i,item in ipairs(branch)
			file:write(link_to_filename(item,""))
			if filename_index[item]
				file:write(" (" .. filename_index[item].cost["Command Points"] .. ")")
			end
			if branch[i+1]
				file:write " &gt; "
			end
		end
		file:write "<br/>"
	end
	template.foot(self, file)
end
function template:upgrade()
	local file = template.head(self, "Upgrade: " .. self.name, "misc")
	template.std(self, file)
	if self.actions and #self.actions > 0
		template.actions(self.actions, file, "Actions")
	end
	template.foot(self, file)
end
function template:squad(write_to)
	local file = write_to or template.head(self, "Squad: " .. self.name, "squads")
	if write_to
		file:write("<h2>Squad: " .. self.name .. "</h2>")
	end
	template.std(self, file)
	if self.comprised_of and #self.comprised_of > 0
		file:write("<h3>Comprised of</h3>")
		for _,val in self.comprised_of
			if (val.max < val.num) or (val.max > val.num)
				file:write("" .. val.num .. "x " .. link_to_filename(val.entity) .. " (can reinforce up to " .. val.max .. ")<br/>")
			else
				file:write("" .. val.num .. "x " .. link_to_filename(val.entity) .. "<br/>")
			end
		end
	end
	if self.veterancy and #self.veterancy > 0
		for rank,info in ipairs(self.veterancy)
			file:write("<h3>Veterancy Rank " .. rank .. " (" .. info.required_xp .. " xp)</h3>")
			template.actions(info.actions, file, "")
		end
	end
	if self.reinforce_requirements then template.requirements(self.reinforce_requirements, file, "Creation Requirements") end
	
	if not write_to
		if self.comprised_of
			for _,val in self.comprised_of
				local entity = filename_index[val.entity]
				if entity and entity.name
					template[entity.template](entity, file)
				end
			end
		end
	
		template.foot(self, file)
	end
end
function template:type()
	local cat = "misc"
	if self.name:find"vehicle" or self.name:find"armour"
		cat = "vehicles"
	elseif self.name:find"infantry"
		cat = "squads"
	elseif self.name:find"building" or self.name:find"defenses"
		cat = "buildings"
	end
	local file = template.head(self, "Type: " .. self.name, cat)
	template.std(self, file)
	if self.examples and #self.examples > 0
		file:write("<h3>Instances</h3>")
		for _,val in self.examples
			file:write(link_to_filename(val) .. "<br/>")
		end
	end
	if self.affects and #self.affects > 0
		file:write("<h3>Affects</h3>")
		for _,val in self.affects
			file:write(link_to_filename(val) .. "<br/>")
		end
	end
	template.foot(self, file)
end

function template:entity_std(file)
	if self.health
		file:write("<h3>Health</h3>")
		file:write("HP: " .. self.health.HP .. "<br/>")
		if self.health["HP Regeneration"] and self.health["HP Regeneration"] > 0
			file:write("HP regen rate: " .. string.format("%.2f",self.health["HP Regeneration"]) .. "<br/>")
		end
		if self.health["Armour Type"]
			file:write("Armour Type: " .. link_to_filename(self.health["Armour Type"],"") .. "<br/>")
		end
		if self.health.Repairable
			file:write "Can be repaired<br/>"
		end
		if self.health.Cuttable
			file:write "Can be cut<br/>"
		end
		if self.wreck or (self.wreck_of and #self.wreck_of > 0)
			if self.wreck
				file:write("When destroyed, becomes " .. link_to_filename(self.wreck) .. "<br/>")
			end
			if self.wreck_of
				for _,val in self.wreck_of
					file:write("Is a wreck of " .. link_to_filename(val) .. "<br/>")
				end
			end
		end
	end
	
	if self.weapons and #self.weapons > 0
		file:write("<h3>Possible Weapons</h3>")
		for _,val in self.weapons
			file:write(link_to_filename(val,"") .. "<br/>")
		end
	end
	
	if self.mobility
		file:write("<h3>Movement</h3>")
		for k,val in self.mobility
			if val >= 0
				file:write(k .. ": " .. string.format("%.2f",val) .. "<br/>")
			end
		end
	end
	
	if self.can_construct and #self.can_construct > 0
		file:write("<h3>Can Construct</h3>")
		for _,val in self.can_construct
			file:write(link_to_filename(val) .. "<br/>")
		end
	end
	
	if self.spawns and #self.spawns > 0
		if self.spawns_at_map_edge
			file:write("<h3>Can Create (At Map Entry Point)</h3>")
		else
			file:write("<h3>Can Create</h3>")
		end
		
		for _,val in self.spawns
			file:write(link_to_filename(val) .. "<br/>")
		end
	end
	
	if self.upgrades and #self.upgrades > 0
		if self.upgrages_max
			file:write("<h3>Upgrades (maximum " .. self.upgrages_max .. ")</h3>")
		else
			file:write("<h3>Upgrades</h3>")
		end
		for _,val in self.upgrades
			file:write(link_to_filename(val) .. "<br/>")
		end
	end
	if self.addons and #self.addons > 0
		if self.addons_max
			file:write("<h3>Addons (maximum " .. self.addons_max .. ")</h3>")
		else
			file:write("<h3>Addons</h3>")
		end
		for _,val in self.addons
			file:write(link_to_filename(val) .. "<br/>")
		end
	end
	if self.transferrable_upgrades and #self.transferrable_upgrades > 0
		if self.transferrable_upgrades_max
			file:write("<h3>Transferrable Upgrades (maximum " .. self.transferrable_upgrades_max .. ")</h3>")
		else
			file:write("<h3>Transferrable Upgrades</h3>")
		end
		for _,val in self.transferrable_upgrades
			file:write(link_to_filename(val) .. "<br/>")
		end
	end
end

function template:building()
	local file = template.head(self, "Building: " .. self.name, "buildings")
	self.cost_title = "Construction Cost"
	if self.cost and self.cost.Seconds
		self.cost["Seconds (divided by num builders)"] = self.cost.Seconds
		self.cost.Seconds = nil
	end
	template.std(self, file)
	template.entity_std(self, file)
	template.foot(self, file)
end
function template:soldier(write_to)
	local file = write_to or template.head(self, "Soldier: " .. self.name, "squads")
	if write_to
		file:write("<h2>Soldier: " .. self.name .. "</h2>")
	end
	self.cost_title = "Creation Cost (from a building)"
	template.std(self, file)
	template.entity_std(self, file)
	if not write_to
		if self.member_of
			for _,val in self.member_of
				local squad = filename_index[val]
				if squad and squad.name
					template[squad.template](squad, file)
				end
			end
		end
	
		template.foot(self, file)
	end
end
function template:entity_generic(write_to)
	local file = write_to or template.head(self, "Entity: " .. self.name, "misc")
	if write_to
		file:write("<h2>Entity: " .. self.name .. "</h2>")
	end
	
	template.std(self, file)
	template.entity_std(self, file)
	
	if not write_to
		if self.member_of
			for _,val in self.member_of
				local squad = filename_index[val]
				if squad and squad.name
					template[squad.template](squad, file)
				end
			end
		end
		template.foot(self, file)
	end
end
function template:mine()
	local file = template.head(self, "Mine: " .. self.name, "misc")
	self.cost_title = "Creation Cost"
	template.std(self, file)
	template.entity_std(self, file)
	template.foot(self, file)
end
function template:slot_item()
	local file = template.head(self, "Slot Item: " .. self.name, "misc")
	template.std(self, file)
	if self.used_by and #self.used_by > 0
		file:write("<h3>Can be used by</h3>")
		for _,v in self.used_by
			file:write(link_to_filename(v) .. "<br/>")
		end
	end
	template.foot(self, file)
end
function template:vehicle(write_to)
	local file = write_to or template.head(self, "Vehicle: " .. self.name, "vehicles")
	if write_to
		file:write("<h2>Vehicle: " .. self.name .. "</h2>")
	end
	self.cost_title = "Creation Cost (from a building)"
	template.std(self, file)
	template.entity_std(self, file)
	if not write_to
		if self.member_of
			for _,val in self.member_of
				local squad = filename_index[val]
				if squad and squad.name
					template[squad.template](squad, file)
				end
			end
		end
		template.foot(self, file)
	end
end
function template:weapon()
	local file = template.head(self, "Weapon: " .. self.name, "misc")
	template.std(self, file)
	if self.used_by and #self.used_by > 0
		file:write("<h3>Can be used by</h3>")
		for _,v in self.used_by
			file:write(link_to_filename(v) .. "<br/>")
		end
	end
	file:write("<h3>Base stats</h3>")
	file:write("Damage: " .. self.damage_base_min .. " to " .. self.damage_base_max .. "<br/>")
	file:write("Setup time: " .. self.setup_time .. " seconds<br/>")
	
	
	for _,range_name in ipairs{"short", "medium", "long", "distant"}
		local range = self.distances[range_name]
		if range
			file:write("<h3>" .. range_name:sub(1,1):upper() .. range_name:sub(2) .. " range (" .. range.min_range .. " to " .. range.max_range .. ")</h3>")
			file:write("Accuracy: " .. string.format("%.2f",range.accuracy*100) .. "%<br/>")
			file:write("Cooldown: " .. string.format("%.2f",range.cooldown_min) .. " to " .. string.format("%.2f",range.cooldown_max) .. " seconds<br/>")
			file:write("Armour Penetration: " .. string.format("%.2f",range.penetration*100) .. "%<br/>")
			file:write("Reload: " .. string.format("%.2f",range.reload_min) .. " to " .. string.format("%.2f",range.reload_max) .. " seconds<br/>")
			file:write("Fire Aim Time: " .. string.format("%.2f",range.fire_aim_time_min) .. " to " .. string.format("%.2f",range.fire_aim_time_max) .. " seconds<br/>")
			file:write("Suppression chance: " .. string.format("%.2f",range.suppression*100) .. "%<br/>")
		end
	end
	
	local function do_mult(val)
		if val < 1
			return "Loses " .. string.format("%.2f",(1-val)*100) .. "% of its"
		elseif val > 1
			return "Gains an additional " .. string.format("%.2f",(val-1)*100) .. "% of its"
		else
			return "Gains no additional"
		end
	end
	
	for cover_name, cover in self.cover
		file:write("<h3>Vs. units in " .. link_to_filename(cover_name) .. "</h3>")
		if cover.accuracy then file:write(do_mult(cover.accuracy) .. " accuracy<br/>") end
		if cover.damage then file:write(do_mult(cover.damage) .. " damage<br/>") end
		if cover.penetration then file:write(do_mult(cover.penetration) .. " armour penetration<br/>") end
		if cover.suppression then file:write(do_mult(cover.suppression) .. " suppression chance<br/>") end
	end
	
	for armour_name, armour in self.armour
		local armour_obj = filename_index[armour_name]
		if armour_obj.examples and #armour_obj.examples > 0
			file:write("<h3>Vs. units with " .. link_to_filename(armour_name) .. "</h3>")
			if armour.accuracy then file:write(do_mult(armour.accuracy) .. " accuracy when stationary<br/>") end
			if armour.moving_accuracy then file:write(do_mult(armour.moving_accuracy) .. " accuracy when moving<br/>") end
			if armour.damage then file:write(do_mult(armour.damage) .. " damage<br/>") end
			if armour.penetration then file:write(do_mult(armour.penetration) .. " armour penetration vs. sides & front<br/>") end
			if armour.rear_penetration then file:write(do_mult(armour.rear_penetration) .. " armour penetration vs. rear<br/>") end
			if armour.suppression then file:write(do_mult(armour.suppression) .. " suppression chance<br/>") end
		end
	end
	
	template.foot(self, file)
end
function template:ability()
	local file = template.head(self, "Ability: " .. self.name, "misc")
	template.std(self, file)
	
	file:write("<h3>Duration</h3>")
	for k,v in self.duration
		if type(v) == "string"
			file:write(k .. ": " .. v .. "<br/>")
		else
			if v > 0
				file:write(k .. ": " .. v .. " seconds<br/>")
			end
		end
	end
	
	if self.is_command_ability
		file:write("<h3>Command Ability</h3>")
		for _,v in self.is_command_ability
			file:write("For " .. link_to_filename(v) .. "<br/>")
		end
	end
	if self.used_by and #self.used_by > 0
		file:write("<h3>Can be done by</h3>")
		for _,v in self.used_by
			file:write(link_to_filename(v) .. "<br/>")
		end
	end
	if self.actions_target_start and #self.actions_target_start > 0
		template.actions(self.actions_target_start, file, "Actions to target")
	end
	if self.actions_target_end and #self.actions_target_end > 0
		template.actions(self.actions_target_end, file, "Actions to target at end")
	end
	if self.actions_self_start and #self.actions_self_start > 0
		template.actions(self.actions_self_start, file, "Actions to self")
	end
	if self.actions_self_end and #self.actions_self_end > 0
		template.actions(self.actions_self_end, file, "Actions to self at end")
	end
	template.foot(self, file)
end

function template:actions(file, title)
	if next(self)
		title = title or "Actions"
		if title != ""
			file:write("<h3>" .. title .. "</h3>")
		end
		file:write("<ul>")
		local function do_actions(t)
			for _,action in t
				if action.type == "paradrop"
					if action.entity
						file:write("<li>Paradrops " .. link_to_filename(action.entity) .. "</li>")
					end
					if action.squad
						file:write("<li>Paradrops " .. link_to_filename(action.squad) .. "</li>")
					end
				elseif action.type == "give_xp"
					file:write("<li>Give " .. action.amount .. " veterancy xp</li>")
				elseif action.type == "cut_wires"
					file:write("<li>Cuts wire over " .. action.time .. " seconds</li>")
				elseif action.type == "modify_resources"
					for k,val in action
						if k != "type"
							if val > 0
								file:write("<li>Gives " .. val .. " " .. k .. "</li>")
							elseif val < 0
								file:write("<li>Takes " .. -val .. " " .. k .. "</li>")
							end
						end
					end
				elseif action.type == "filter"
					local thing_is = "thing is "
					local _if_ = " if "
					if action.filters and #action.filters == 1
						thing_is = ""
						_if_ = ""
						file:write("<li>Apply to " .. action.when .. " ")
					else
						file:write("<li>Apply to " .. action.when .. " things belonging to " .. action.who)
					end
					if action.filters and #action.filters > 0
						for i,filter in action.filters
							if i == 1
								file:write(_if_)
							else
								file:write(" and ")
							end
							if filter.type == "entity"
								file:write(thing_is .. link_to_filename(filter.entity))
							elseif filter.type == "squad"
								file:write(thing_is .. link_to_filename(filter.squad))
							elseif filter.type == "unit_class_type"
								file:write(thing_is .. link_to_filename(filter.unit_class))
							elseif filter.type == "target_type"
								file:write(thing_is .. link_to_filename(filter.target_type))
							end
						end
						if #action.filters == 1
							file:write("s belonging to " .. action.who)
						end
					end
					file:write(": ")
					template.actions(action.actions, file, "")
					file:write("</li>")
				elseif action.type == "air_recon"
					file:write("<li>Performs air recon over distance of " .. action.distance .. " and radius " .. action.radius .. "</li>")
				elseif action.type == "retreat"
					if action.required_suppression_level and action.required_suppression_level > 0
						file:write("<li>Forces retreat (must be at least " .. string.format("%.2f",action.required_suppression_level*100) .. "% surpressed)</li>")
					else
						file:write("<li>Forces retreat</li>")
					end
				elseif action.type == "use_accessory_weapon"
					file:write("<li>Uses " .. link_to_filename(action.item) .. "</li>")
				elseif action.type == "slot_item_add"
					file:write("<li>Adds " .. link_to_filename(action.item) .. "</li>")
				elseif action.type == "change_armour_type"
					file:write("<li>Changes armour type from " .. link_to_filename(action.from,"") .. " to " .. link_to_filename(action.to,"") .. "</li>")
				elseif action.type == "salvage"
					file:write("<li>Salvages wreck</li>")
				elseif action.type == "assault"
					file:write("<li>Assaults</li>")
				elseif action.type == "suppression"
					file:write("<li>Becomes " .. (action.value*100) .. "% suppressed</li>")
				elseif action.type == "reinforcements"
					file:write("<li>Summons reinforcement group from the following list (min. pop cap " .. action.min_pop
						.. ", max " .. action.max_points .. " points)<ul>")
					for _,squad in action.squads
						file:write("<li>" .. link_to_filename(squad) .. "</li>")
					end
					file:write("</ul></li")
				elseif action.type == "modifiers"
					if action.permanently
						file:write("<li>Applies the following permanently:")
						template.modifiers(action.modifiers, file, "")
						file:write "</li>"
					elseif action.duration > 0
						file:write("<li>Applies the following for " .. action.duration .. " seconds:")
						template.modifiers(action.modifiers, file, "")
						file:write "</li>"
					else
						template.modifiers(action.modifiers, file, "", true)
					end
				elseif action.type == "custom"
					file:write("<li>" .. action.text .. "</li>")
				else
					print("Unable to describe action type: "  .. action.type)
				end
			end
		end
		do_actions(self)
		file:write("</ul>")
	end
end

function template:requirements(file, title)
	local function make_dupes_table()
		local t = {}
		setmetatable(t, {__newindex = function (t, k, v)
			rawset(t, k, v)
			file:write("<li>" .. k .. "</li>")
		end } )
		return t
	end
	local duplicate_prevention = make_dupes_table()
	if next(self)
		title = title or "Requirements"
		if title != ""
			file:write("<h3>" .. title .. "</h3>")
		end
		file:write("<ul>")
		local function do_requirements(t)
			for _,requirement in t
				if requirement.type == "upgrade"
					duplicate_prevention[link_to_filename(requirement.upgrade)] = true
				elseif requirement.type == "race"
					duplicate_prevention[link_to_filename(requirement.race)] = true
				elseif requirement.type == "in_territory"
					duplicate_prevention["Must be in territory"] = true
				elseif requirement.type == "entity"
					if requirement.e_name
						duplicate_prevention[link_to_filename(requirement.e_name)] = true
					end
					if requirement.e_type
						duplicate_prevention["At least one building/soldier/vehicle with " .. link_to_filename("type_unit_class_" .. requirement.e_type .. ".html")] = true
					end
				elseif requirement.type == "cover"
					file:write("<li>Unit must be one of the following types of cover:<ul>")
					local old_duplicate_prevention = duplicate_prevention
					duplicate_prevention = make_dupes_table()
					for _,filename in requirement.cover
						duplicate_prevention[link_to_filename(filename,"")] = true
					end
					duplicate_prevention = old_duplicate_prevention
					file:write("</ul></li>")
				elseif requirement.type == "binary_or"
					if #requirement.list_1 < 1 or #requirement.list_2 < 1
						do_requirements(requirement.list_1)
						do_requirements(requirement.list_2)
					elseif (#requirement.list_1 + #requirement.list_2) == 2
						file:write("<li>Any of:<ul>")
						local old_duplicate_prevention = duplicate_prevention
						duplicate_prevention = make_dupes_table()
						do_requirements(requirement.list_1)
						do_requirements(requirement.list_2)
						duplicate_prevention = old_duplicate_prevention
						file:write("</ul></li>")
					else
						file:write("<li>Either:<ul><li>All of:<ul>")
						local old_duplicate_prevention = duplicate_prevention
						duplicate_prevention = make_dupes_table()
						do_requirements(requirement.list_1)
						file:write("</ul></li><li>Of all of:<ul>")
						duplicate_prevention = make_dupes_table()
						do_requirements(requirement.list_2)
						file:write("</ul></li></ul></li>")
						duplicate_prevention = old_duplicate_prevention
					end
				elseif requirement.type == "custom"
					duplicate_prevention[requirement.text] = true
				elseif requirement.type == "not"
					file:write("<li>None of:<ul>")
					local old_duplicate_prevention = duplicate_prevention
					duplicate_prevention = make_dupes_table()
					do_requirements(requirement.list)
					duplicate_prevention = old_duplicate_prevention
					file:write("</ul></li>")
				elseif requirement.type == "structure"
					duplicate_prevention[link_to_filename(requirement.structure)] = true
				else
					print("Unable to describe requirement type: "  .. requirement.type)
				end
			end
		end
		do_requirements(self)
		file:write("</ul>")
	end
end

function template:modifiers(file, title, existing_list)
	if next(self)
		title = title or "Modifiers"
		if title != ""
			file:write("<h3>" .. title .. "</h3>")
		end
		if not existing_list then file:write("<ul>") end
		local function do_math(modifier, positive_format, negative_format)
			if modifier.operator == "multiplication"
				if modifier.value > 1
					return string.format(positive_format, string.format("%.2f",(modifier.value-1)*100) .. "%")
				else
					return string.format(negative_format, string.format("%.2f",(1-modifier.value)*100) .. "%")
				end
			elseif modifier.operator == "addition"
				if modifier.value > 0
					return string.format(positive_format, string.format("%.2f",modifier.value))
				else
					return string.format(negative_format, string.format("%.2f",-modifier.value))
				end
			elseif modifier.operator == "enable"
				if modifier.value > 0
					return positive_format
				else
					return negative_format
				end
			end
		end
		local function do_modifiers(t)
			for _,modifier in t
				if modifier.type == "weapon_accuracy"		
					file:write("<li>Weapon accuracy " .. do_math(modifier,"increased by %s", "decreased by %s") .. "</li>")
				elseif modifier.type == "received_accuracy"
					file:write("<li>Things shooting at us " .. do_math(modifier,"get a %s accuracy boost", "have %s less accuracy") .. "</li>")
				elseif modifier.type == "weapon_cooldown"
					file:write("<li>Weapon cooldown " .. do_math(modifier,"increased by %s", "decreased by %s") .. "</li>")
				elseif modifier.type == "received_suppression"
					file:write("<li>" .. do_math(modifier,"%s easier", "%s harder") .. " to suppress</li>")
				elseif modifier.type == "weapon_damage"
					file:write("<li>Weapon damage " .. do_math(modifier,"increased by %s", "decreased by %s") .. "</li>")
				elseif modifier.type == "weapon_range"
					file:write("<li>Weapon range " .. do_math(modifier,"increased by %s", "decreased by %s") .. "</li>")
				elseif modifier.type == "weapon_reload"
					file:write("<li>Weapon reload time " .. do_math(modifier,"increased by %s", "decreased by %s") .. "</li>")
				elseif modifier.type == "received_damage"
					file:write("<li>Things shooting at us " .. do_math(modifier,"get a %s damage boost", "do %s less damage") .. "</li>")
				elseif modifier.type == "decamouflage"
					file:write("<li>Radius for detecting camouflaged units " .. do_math(modifier,"increased by %s", "decreased by %s") .. "</li>")
				elseif modifier.type == "camouflage_radius"
					file:write("<li>Radius for decamouflaging us " .. do_math(modifier,"increased by %s", "decreased by %s") .. "</li>")
				elseif modifier.type == "posture_speed"
					file:write("<li>Posture speed " .. do_math(modifier,"increased by %s", "decreased by %s") .. "</li>")
				elseif modifier.type == "health_regeneration"
					file:write("<li>Health regeneration " .. do_math(modifier,"increased by %s", "decreased by %s") .. "</li>")
				elseif modifier.type == "health_maximum"
					file:write("<li>Maximum HP " .. do_math(modifier,"increased by %s", "decreased by %s") .. "</li>")
				elseif modifier.type == "received_penetration"
					file:write("<li>Things shooting at us " .. do_math(modifier,"get a %s armour penetration boost", "have %s less armour penetration") .. "</li>")
				elseif modifier.type == "weapon_penetration"
					file:write("<li>Weapon armour penetration " .. do_math(modifier,"increased by %s", "decreased by %s") .. "</li>")
				elseif modifier.type == "speed_maximum"
					file:write("<li>Maximum speed " .. do_math(modifier,"increased by %s", "decreased by %s") .. "</li>")
				elseif modifier.type == "income_manpower_player"
					file:write("<li>Manpower generation " .. do_math(modifier,"increased by %s", "decreased by %s") .. "</li>")
				elseif modifier.type == "camouflage_enable"
					file:write("<li>" .. do_math(modifier,"Camouflage", "Decamouflage") .. "</li>")
				elseif modifier.type == "capture_enable_squad"
					if modifier.target
						file:write("<li>" .. do_math(modifier,"Allows " .. link_to_filename(modifier.target) .. "s to capture", "Prevents " .. link_to_filename(modifier.target) .. "s from capturing") .. " VPs and resource points</li>")
					else
						file:write("<li>" .. do_math(modifier,"Allows the squad to capture", "Prevents the squad from capturing") .. " VPs and resource points</li>")
					end
				elseif modifier.type == "custom"
					file:write("<li>" .. modifier.text .. "</li>")
				else
					print("Unable to describe modifier type: "  .. modifier.type)
				end
			end
		end
		do_modifiers(self)
		if not existing_list then file:write("</ul>") end
	end
end

function template:requirement_of(file)
	if #self > 0
		file:write("<h3>Requirement Of</h3>")
		local links_done = {}
		for _,of in self
			local link = link_to_filename(of)
			if not links_done[link]
				file:write(link .. "<br/>")
				links_done[link] = true
			end
		end
	end
end

function make_breadcrumbs(filename,path)
	local path_parts = table(path:split"\\")
	local title = "Folder: "
	if breadcrumbs_name[filename]
		title = title .. breadcrumbs_name[filename]
	elseif filename_index[filename] and filename_index[filename].name
		title = title .. filename_index[filename].name
	else
		local part = path_parts[#path_parts]
		title = title .. part:sub(1,1):upper() .. part:sub(2)
	end
	
	local section = "misc"
	if path_parts[1] == "sbps"
		section = "squads"
	elseif path_parts[1] == "racebps"
		section = "armies"
	elseif path_parts[1] == "ebps" and path_parts[2] == "races" and path_parts[4]
		if path_parts[4] == "buildings"
			section = "buildings"
		elseif path_parts[4] == "vehicles"
			section = "vehicles"
		end
	end
	
	local self = {}
	self.filename = filename
	self.filepath = path .. "."
	local file = template.head(self, title, section)
	
	if not file
		print("Unable to lay crumbs")
		return
	end
	
	local folder = filemap
	for _,part in ipairs(path_parts)
		folder = folder[part]
	end
	
	local folder_paths = table{path}
	local folders = table{folder}
	local subfolders = false
	for i,folder in ipairs(folders)		
		for part,item in folder
			if type(item) == "table"
				local pre = ""
				if not subfolders
					pre = "<h3>Subfolders</h3>"
				end
				local url = folder_paths[i]:gsub("[\\/]","_") .. "_" .. part .. ".html"
				if breadcrumbs_name[url] == ""
					folders:insert(item)
					folder_paths:insert(path .. "\\" .. part)
				elseif breadcrumbs_name[url]
					subfolders = true
					file:write(pre .. [[<a href="]] .. url .. [[">]] .. breadcrumbs_name[url] .. [[</a><br/>]])
				elseif filename_index[url] and filename_index[url].name
					subfolders = true
					file:write(pre .. [[<a href="]] .. url .. [[">]] .. filename_index[url].name .. [[</a><br/>]])
				elseif breadcrumbs_todo[url]
					subfolders = true
					file:write(pre .. [[<a href="]] .. url .. [[">]] .. part:sub(1,1):upper() .. part:sub(2) .. [[</a><br/>]])
				end
			end
		end
	end
	
	local contents = false
	for i,folder in ipairs(folders)		
		for part,item in folder
			if type(item) == "userdata"
				local pre = ""
				if not contents
					pre = "<h3>Contents</h3>"
				end
				local url = folder_paths[i]:gsub("[\\/]","_") .. "_" .. part:beforelast"." .. ".html"
				if filename_index[url]
					contents = true
					file:write(pre .. link_to_filename(url) .. "<br/>")
				end
			end
		end
	end
	
	template.foot(self, file)
end
