function mine_core(name, filename, filepath)
	local obj = {}
	obj.name = name
	obj.filename = filename
	obj.filepath = filepath
	filename_index[obj.filename] = obj

	return obj
end

miners_index = {}

function mine_ability(filename, rgd, filepath)
	local name = UCS(rgd.GameData.ability_bag.ui_info.screen_name)
	if name == "$0 no key"
		name = rgd.name:beforelast".":gsub("_"," ")
		name = name:sub(1,1):upper() .. name:sub(2)
	end
	local ability = mine_core(name, filename, filepath)
	
	ability.object_type = "Ability"
	ability.template = "ability"
	ability.cost = {}
	ability.cost.Manpower = rgd.GameData.ability_bag.cost.manpower
	ability.cost.Fuel = rgd.GameData.ability_bag.cost.fuel
	ability.cost.Munitions = rgd.GameData.ability_bag.cost.munition
	ability.cost["Command Points"] = rgd.GameData.ability_bag.cost.command
	ability.requirements = mine_requirements(rgd.GameData.ability_bag.requirements, filename)
	ability.actions_target_start = mine_actions(rgd.GameData.ability_bag.action_list.start_target_actions, filename)
	ability.actions_target_end = mine_actions(rgd.GameData.ability_bag.action_list.end_target_actions, filename)
	ability.actions_self_start = mine_actions(rgd.GameData.ability_bag.action_list.start_self_actions, filename)
	ability.actions_self_end = mine_actions(rgd.GameData.ability_bag.action_list.end_self_actions, filename)
	
	ability.duration = {}
	ability.duration.Duration = rgd.GameData.ability_bag.duration_time
	ability.duration.Recharge = rgd.GameData.ability_bag.recharge_time
	
	if rgd.GameData.ability_bag.activation == "always_on"
		ability.duration.Duration = "Always on"
	end
	
	return ability
end
miners_index.abilities = mine_ability

function mine_modifiers(actions, our_filename, append_to)
	local results = append_to or table()
	for k,v in actions do
		if k != "$REF"
			if v["$REF"] != "modifiers\\no_modifier.lua"
				if v["$REF"] == "modifiers\\accuracy_weapon_modifier.lua"
					results:insert{type = "weapon_accuracy", operator = v.usage_type, value = v.value}
				elseif v["$REF"] == "modifiers\\received_accuracy_modifier.lua"
					results:insert{type = "received_accuracy", operator = v.usage_type, value = v.value}
				elseif v["$REF"] == "modifiers\\cooldown_weapon_modifier.lua"
					results:insert{type = "weapon_cooldown", operator = v.usage_type, value = v.value}
				elseif v["$REF"] == "modifiers\\received_suppression_squad_modifier.lua"
					results:insert{type = "received_suppression", operator = v.usage_type, value = v.value}
				elseif v["$REF"] == "modifiers\\received_damage_modifier.lua"
					results:insert{type = "received_damage", operator = v.usage_type, value = v.value}
				elseif v["$REF"] == "modifiers\\damage_weapon_modifier.lua"
					results:insert{type = "weapon_damage", operator = v.usage_type, value = v.value}
				elseif v["$REF"] == "modifiers\\range_weapon_modifier.lua"
					results:insert{type = "weapon_range", operator = v.usage_type, value = v.value}
				elseif v["$REF"] == "modifiers\\sight_radius_modifier.lua"
					results:insert{type = "decamouflage", operator = v.usage_type, value = v.value}
				elseif v["$REF"] == "modifiers\\camouflage_conceal_radius.lua"
					results:insert{type = "camouflage_radius", operator = v.usage_type, value = v.value}
				elseif v["$REF"] == "modifiers\\posture_speed_modifier.lua"
					results:insert{type = "posture_speed", operator = v.usage_type, value = v.value}
				elseif v["$REF"] == "modifiers\\health_regeneration_modifier.lua"
					results:insert{type = "health_regeneration", operator = v.usage_type, value = v.value}
				elseif v["$REF"] == "modifiers\\health_maximum_modifier.lua"
					results:insert{type = "health_maximum", operator = v.usage_type, value = v.value}
				elseif v["$REF"] == "modifiers\\received_penetration_modifier.lua"
					results:insert{type = "received_penetration", operator = v.usage_type, value = v.value}
				elseif v["$REF"] == "modifiers\\speed_maximum_modifier.lua"
					results:insert{type = "speed_maximum", operator = v.usage_type, value = v.value}
				elseif v["$REF"] == "modifiers\\weapon_penetration_modifier.lua"
					results:insert{type = "weapon_penetration", operator = v.usage_type, value = v.value}
				elseif v["$REF"] == "modifiers\\camouflage_enable.lua"
					results:insert{type = "camouflage_enable", operator = v.usage_type, value = v.value}
				elseif v["$REF"] == "modifiers\\reload_weapon_modifier.lua"
					results:insert{type = "weapon_reload", operator = v.usage_type, value = v.value}
				elseif v["$REF"] == "modifiers\\capture_enable_squad_modifier.lua"
					local who = v.target_type_name
					if who == ""
						who = nil
					else
						local squad = mine_file_now(nil, who)
						if not squad.related_to
							squad.related_to = table()
						end
						squad.related_to:insert(our_filename)
						
						who = filepath_to_filename(who)
					end
					results:insert{type = "capture_enable_squad", operator = v.usage_type, value = v.value, target = who}
				elseif v["$REF"] == "modifiers\\income_manpower_player_modifier.lua"
					results:insert{type = "income_manpower_player", operator = v.usage_type, value = v.value}
				else
					results:insert{type = "custom", text = "Unable to describe \'" .. v["$REF"] .. "\'"}
					print("Unknown modifier: " .. v["$REF"])
				end
			end
		end
	end
	return results
end

function mine_actions(actions, our_filename, append_to)
	local results = append_to or table()
	for k,v in actions do
		if k != "$REF"
			if not v["$REF"]:find "\\no_action."
				if v["$REF"] == "action\\ability_action\\paradrop_action.lua"
					local entity_path = v.entity_to_spawn
					if entity_path == ""
						entity_path = nil
					else
						local entity = mine_file_now(nil, entity_path)
						if not entity.from
							entity.from = table()
						end
						entity.from:insert(our_filename)
						entity_path = filepath_to_filename(entity_path)
					end
					local squad_path = v.squad_to_spawn
					if squad_path == ""
						squad_path = nil
					else
						local squad = mine_file_now(nil, squad_path)
						if not squad.from
							squad.from = table()
						end
						squad.from:insert(our_filename)
						squad_path = filepath_to_filename(squad_path)
					end
					results:insert{type = "paradrop", entity = entity_path, squad = squad_path}
				elseif v["$REF"] == "action\\ability_action\\modify_resources.lua"
					results:insert{type = "modify_resources", Fuel = v.fuel, Manpower = v.manpower, Munitions = v.munition}
				elseif v["$REF"] == "action\\upgrade_action\\veterancy_action.lua"
					results:insert{type = "give_xp", amount = v.number_of_experience_points}
				elseif v["$REF"] == "action\\upgrade_action\\filter_action.lua"
					or v["$REF"] == "action\\ability_action\\filter_action.lua"
					local filter = {type = "filter", when = "current", who = "us"}
					if v.apply_to_future_units
						if v["$REF"] == "action\\ability_action\\filter_action.lua"
							filter.when = "all"
						else
							filter.when = "current and future"
						end
					end
					if not v.apply_to_local_player
						filter.who = "other players"
					end
					filter.filters = table()
					if v.entity_type and v.entity_type != ""
						filter.filters:insert{type = "entity", entity = filepath_to_filename(v.entity_type)}
						local entity = mine_file_now(nil, v.entity_type)
						if not entity.related_to
							entity.related_to = table()
						end
						entity.related_to:insert(our_filename)
					end
					if v.squad_type and v.squad_type != ""
						filter.filters:insert{type = "squad", squad = filepath_to_filename(v.squad_type)}
						local squad = mine_file_now(nil, v.squad_type)
						if not squad.related_to
							squad.related_to = table()
						end
						squad.related_to:insert(our_filename)
					end
					if v.target_info["$REF"] and v.target_info["$REF"] != "" and v.target_info["$REF"] != "type_ability_target_type\\all.lua"
						filter.filters:insert{type = "target_type", target_type = filepath_to_filename(v.target_info["$REF"])}
					end
					if v.unit_class_type and v.unit_class_type != ""
						local unit_class_type = v.unit_class_type
						if not unit_class_type:find"%." then
							unit_class_type = "type_unit_class\\" .. unit_class_type .. ".rgd"
						end
						filter.filters:insert{type = "unit_class_type", unit_class = filepath_to_filename(unit_class_type)}
						local unit_class = mine_file_now(nil, unit_class_type)
						if not unit_class.related_to
							unit_class.related_to = table()
						end
						unit_class.related_to:insert(our_filename)
					end
					filter.actions = mine_actions(v.action_table.upgrade_actions, our_filename)
					mine_actions(v.action_table.ability_actions, our_filename, filter.actions)
					mine_actions(v.action_table.critical_actions, our_filename, filter.actions)
					results:insert(filter)
				elseif v["$REF"] == "action\\ability_action\\target.lua"
					mine_actions(v.subactions, our_filename, results)
				elseif v["$REF"] == "action\\critical_action\\rout_action.lua"
					results:insert{type = "retreat", required_suppression_level = v.required_suppression_level}
				elseif v["$REF"] == "action\\upgrade_action\\change_weapon_target_type.lua"
					local type_target_weapon = mine_file_now(nil, v.new_type)
					if not type_target_weapon.examples
						type_target_weapon.examples = table()
					end
					type_target_weapon.examples:insert(our_filename)
					results:insert{type = "change_armour_type", from = filepath_to_filename(v.original_type), to = filepath_to_filename(v.new_type)}
				elseif v["$REF"] == "action\\ability_action\\air_recon_action.lua"
					results:insert{type = "air_recon", distance = v.distance_between_reveals, radius = v.reveal_radius}
				elseif v["$REF"] == "action\\upgrade_action\\slot_item_add.lua"
					local item = mine_file_now(nil, v.slot_item)
					results:insert{type = "slot_item_add", item = filepath_to_filename(v.slot_item)}
					if not item.related_to
						item.related_to = table()
					end
					item.related_to:insert(our_filename)
				elseif v["$REF"] == "action\\ability_action\\use_accessory_weapon.lua"
					local item = mine_file_now(nil, v.accessory_weapon_item)
					results:insert{type = "use_accessory_weapon", item = filepath_to_filename(v.accessory_weapon_item)}
					if not item.used_by
						item.used_by = table()
					end
					item.used_by:insert(our_filename)
				elseif v["$REF"] == "action\\ability_action\\trigger_dca_action.lua"
					local do_nothing = true
				elseif v["$REF"] == "action\\ability_action\\animator_set_state.lua"
					or v["$REF"] == "action\\upgrade_action\\animator_set_state.lua"
					or v["$REF"] == "action\\ability_action\\animator_set_event.lua"
					local do_nothing = true
				elseif v["$REF"] == "action\\upgrade_action\\alter_squad_ui_info_action.lua"
					local do_nothing = true
				elseif v["$REF"] == "action\\ability_action\\delay.lua"
					mine_actions(v.delayed_actions, our_filename, results)
				elseif v["$REF"] == "action\\ability_action\\salvage_from_wreck_action.lua"
					results:insert{type = "salvage"}
				elseif v["$REF"] == "action\\ability_action\\assault_action.lua"
					results:insert{type = "assault"}
				elseif v["$REF"] == "action\\ability_action\\suppression.lua"
					results:insert{type = "suppression", value = v.suppression}
				elseif v["$REF"] == "action\\ability_action\\cut_wires.lua"
					results:insert{type = "cut_wires", time = v.time_to_cut}
				elseif v["$REF"] == "action\\upgrade_action\\apply_modifiers_action.lua"
					or v["$REF"] == "action\\ability_action\\apply_modifiers_action.lua"
					results:insert{type = "modifiers", duration = v.duration, permanent = v.permanent, modifiers = mine_modifiers(v.modifiers, our_filename)}
				elseif v["$REF"] == "action\\ability_action\\reinforcements_action.lua"
					local action = {type = "reinforcements", min_pop = v.minimum_pop_used, max_points = v.points_total, squads = table()}
					for k,val in v.squad_table
						if k != "$REF" and val != ""
							action.squads:insert(filepath_to_filename(val))
							local squad = mine_file_now(nil, val)
							if not squad.from
								squad.from = table()
							end
							squad.from:insert(our_filename)
						end
					end
					results:insert(action)
				else
					results:insert{type = "custom", text = "Unable to describe \'" .. v["$REF"] .. "\'"}
					print("Unknown action: " .. v["$REF"])
				end
			end
		end
	end
	return results
end

function mine_requirements(requirements, our_filename, append_to)
	local results = append_to or table()
	for k,v in requirements do
		if k != "$REF"
			if v["$REF"] != "requirements\\required_none.lua"
				if v["$REF"] == "requirements\\required_binary_expr.lua"
					if v.operation == "and"
						mine_requirements(v.requirement_table_1, our_filename, results)
						mine_requirements(v.requirement_table_2, our_filename, results)
					elseif v.operation == "or"
						results:insert( {type = "binary_or"
							,list_1 = mine_requirements(v.requirement_table_1, our_filename)
							,list_2 = mine_requirements(v.requirement_table_2, our_filename) } )
					end
				elseif v["$REF"] == "requirements\\required_in_territory.lua"
					if v.in_supply
						results:insert{type = "in_territory"}
					end
				elseif v["$REF"] == "requirements\\required_race.lua"
					results:insert {type = "race", race = filepath_to_filename(v.race)}
					local race = mine_file_now(nil, v.race)
					if not race.requirement_of
						race.requirement_of = table()
					end
					race.requirement_of:insert(our_filename)
				elseif v["$REF"] == "requirements\\required_in_cover.lua"
					local cover = {type = "cover", cover = table()}
					for k,val in v.cover_type_table	
						if k != "$REF" and val != ""
							cover.cover:insert(filepath_to_filename("type_cover\\" .. val .. ".rgd"))
							local obj = mine_file_now(nil, "type_cover\\" .. val .. ".rgd")
							if not obj.requirement_of
								obj.requirement_of = table()
							end
							obj.requirement_of:insert(our_filename)
						end
					end
					results:insert(cover)
				elseif v["$REF"] == "requirements\\required_player_upgrade.lua"
					local upgradehtml = v.upgrade_name:beforelast".":gsub("[\\/]","_") .. ".html"
					results:insert( {type = "upgrade", upgrade = upgradehtml} )
					local upgrade = mine_file_now(upgradehtml, v.upgrade_name:beforelast".lua" .. ".rgd")
					if not upgrade.requirement_of
						upgrade.requirement_of = table()
					end
					upgrade.requirement_of:insert(our_filename)
				elseif v["$REF"] == "requirements\\required_unary_expr.lua"
					results:insert( {type = "not"
							,list = mine_requirements(v.requirement_table, our_filename) } )
				elseif v["$REF"] == "requirements\\required_structure.lua"
					local entity_name = v.structure_name
					if entity_name == ""
						entity_name = nil
					else
						local entity_html = entity_name:beforelast".":gsub("[\\/]","_") .. ".html"
						local entity = mine_file_now(entity_html, entity_name:beforelast".lua" .. ".rgd")
						if not entity.requirement_of
							entity.requirement_of = table()
						end
						entity.requirement_of:insert(our_filename)
					end
					results:insert{type = "structure", structure = filepath_to_filename(entity_name)}
				elseif v["$REF"] == "requirements\\required_entity.lua"
					local entity_name = v.entity_blueprint_name
					if entity_name == ""
						entity_name = nil
					else
						local entity_html = entity_name:beforelast".":gsub("[\\/]","_") .. ".html"
						local entity = mine_file_now(entity_html, entity_name:beforelast".lua" .. ".rgd")
						if not entity.requirement_of
							entity.requirement_of = table()
						end
						entity.requirement_of:insert(our_filename)
					end
					local entity_type = v.entity_type
					if entity_type == ""
						entity_type = nil
					else
						local type_html = "type_unit_class_" .. entity_type .. ".html"
						local type = mine_file_now(type_html, "type_unit_class\\" .. entity_type .. ".rgd" )
						if not type.requirement_of
							type.requirement_of = table()
						end
						type.requirement_of:insert(our_filename)
					end
					results:insert( {type = "entity", e_name = entity_name, e_type = entity_type} )
				else
					if v.ui_name and v.ui_name != "" and v.ui_name != "$0"
						results:insert{type = "custom", text = UCS(v.ui_name)}
					else
						results:insert{type = "custom", text = "Unable to describe \'" .. v["$REF"] .. "\'"}
					end
					print("Unknown requirement type: " .. v["$REF"])
				end
			end
		end
	end
	return results
end

function mine_type_target_weapon(filename, rgd, filepath)
	local type  = mine_core(filepath:afterlast"\\tp_":beforelast".", filename, filepath)
	type.template = "type"
	type.object_type = "Armour Type"
	
	return type
end
miners_index.type_target_weapon = mine_type_target_weapon

function mine_type_cover(filename, rgd, filepath)
	local type  = mine_core(filepath:afterlast"\\tp_":beforelast".", filename, filepath)
	type.template = "type"
	type.object_type = "Cover Type"
	
	return type
end
miners_index.type_cover = mine_type_cover

function mine_type_unit_class(filename, rgd, filepath)
	local type  = mine_core(filepath:afterlast"\\":beforelast".", filename, filepath)
	type.template = "type"
	type.object_type = "Unit Type"
	
	return type
end
miners_index.type_unit_class = mine_type_unit_class

function mine_weapon(filename, rgd, filepath)
	local name = UCS(rgd.GameData.weapon_bag.ui_name)
	if name == "$0 no key"
		name = rgd.GameData.weapon_bag.name
	end
	if name == ""
		name = rgd.name:beforelast".":gsub("_"," ")
	end
	local weapon = mine_core(name, filename, filepath)
	weapon.template = "weapon"
	weapon.object_type = "Weapon"
	
	weapon.damage_base_min = rgd.GameData.weapon_bag.damage.min
	weapon.damage_base_max = rgd.GameData.weapon_bag.damage.max
	weapon.setup_time = rgd.GameData.weapon_bag.setup.duration
	
	weapon.moving = {}
	weapon.moving.accuracy = rgd.GameData.weapon_bag.moving.accuracy_multiplier
	weapon.moving.cooldown = rgd.GameData.weapon_bag.moving.cooldown_multiplier
	weapon.moving.can_fire = not rgd.GameData.weapon_bag.moving.disable_moving_firing
	
	weapon.distances = {}
	
	local function check_range(min, max)
		if math.abs(max - min) > 0.01
			return {min_range = min, max_range = max}
		end
		return nil
	end
	
	weapon.distances.short = check_range(rgd.GameData.weapon_bag.range.min, rgd.GameData.weapon_bag.range.mid.short)
	weapon.distances.medium = check_range(rgd.GameData.weapon_bag.range.mid.short, rgd.GameData.weapon_bag.range.mid.medium)
	weapon.distances.long = check_range(rgd.GameData.weapon_bag.range.mid.medium, rgd.GameData.weapon_bag.range.mid.long)
	weapon.distances.distant = check_range(rgd.GameData.weapon_bag.range.mid.long, rgd.GameData.weapon_bag.range.mid.distant)
	
	for d_name, d_table in weapon.distances
		d_table.accuracy = rgd.GameData.weapon_bag.accuracy[d_name]
		d_table.fire_aim_time_min = rgd.GameData.weapon_bag.aim.fire_aim_time.min * rgd.GameData.weapon_bag.aim.fire_aim_time_multiplier[d_name]
		d_table.fire_aim_time_max = rgd.GameData.weapon_bag.aim.fire_aim_time.max * rgd.GameData.weapon_bag.aim.fire_aim_time_multiplier[d_name]
		d_table.cooldown_min = rgd.GameData.weapon_bag.cooldown.duration.min * rgd.GameData.weapon_bag.cooldown.duration_multiplier[d_name]
		d_table.cooldown_max = rgd.GameData.weapon_bag.cooldown.duration.max * rgd.GameData.weapon_bag.cooldown.duration_multiplier[d_name]
		d_table.penetration = rgd.GameData.weapon_bag.penetration[d_name]
		d_table.reload_min = rgd.GameData.weapon_bag.reload.duration.min * rgd.GameData.weapon_bag.reload.duration_multiplier[d_name]
		d_table.reload_max = rgd.GameData.weapon_bag.reload.duration.max * rgd.GameData.weapon_bag.reload.duration_multiplier[d_name]
		d_table.suppression = rgd.GameData.weapon_bag.suppression.suppression[d_name]
	end
	
	local function check_multiplier(val)
		if (val < 1) or (val > 1)
			return val
		end
		return nil
	end
	
	weapon.cover = {}
	for c_name, c_details in rgd.GameData.weapon_bag.cover_table
		if c_name != "$REF"
			local details = {}
			details.accuracy = check_multiplier(c_details.accuracy_multiplier)
			details.damage = check_multiplier(c_details.damage_multiplier)
			details.penetration = check_multiplier(c_details.penetration_multiplier)
			details.suppression = check_multiplier(c_details.suppression_multiplier)
			if next(details)
				weapon.cover[filepath_to_filename("type_cover\\" .. c_name .. ".rgd")] = details
				local cover_type = mine_file_now(nil, "type_cover\\" .. c_name .. ".rgd")
				if not cover_type.affects
					cover_type.affects = table()
				end
				cover_type.affects:insert(filename)
			end
		end
	end
	
	weapon.armour = {}
	for a_name, a_details in rgd.GameData.weapon_bag.target_table
		if a_name != "$REF"
			local details = {}
			details.accuracy = check_multiplier(a_details.accuracy_multiplier)
			details.moving_accuracy = check_multiplier(a_details.moving_accuracy_multiplier)
			details.damage = check_multiplier(a_details.damage_multiplier)
			details.penetration = check_multiplier(a_details.penetration_multiplier)
			details.rear_penetration = check_multiplier(a_details.rear_penetration_multiplier)
			details.suppression = check_multiplier(a_details.suppression_multiplier)
			if next(details)
				weapon.armour[filepath_to_filename("type_target_weapon\\" .. a_name .. ".rgd")] = details
				local armour_type = mine_file_now(nil, "type_target_weapon\\" .. a_name .. ".rgd")
				if not armour_type.affects
					armour_type.affects = table()
				end
				armour_type.affects:insert(filename)
			end
		end
	end
	
	return weapon
end
miners_index.weapon = mine_weapon

function mine_entity(filename, rgd, filepath)
	local entity = mine_core(UCS(rgd.GameData.ui_ext.screen_name), filename, filepath)
	
	entity.template = "entity_generic"
	if filepath:find"\\buildings\\"
		entity.template = "building"
		entity.from_title = "Can be built by"
		entity.object_type = "Building"
		entity.from_ext = ""
	elseif filepath:find"\\mines\\"
		entity.template = "mine"
	elseif filepath:find"\\soldiers\\"
		entity.template = "soldier"
		entity.from_title = "Can be obtained from"
		entity.object_type = "Soldier"
	elseif filepath:find"\\vehicles\\"
		entity.template = "vehicle"
		entity.from_title = "Can be obtained from"
		entity.object_type = "Vehicle"
	end
	
	if rgd.GameData.combat_ext and rgd.GameData.combat_ext.hardpoints
		entity.weapons = table()
		for k,hardpoint in rgd.GameData.combat_ext.hardpoints
			if k != "$REF"
				for k,weapon in hardpoint.weapon_table
					if k != "$REF"
						if weapon.weapon and weapon.weapon != ""
							entity.weapons:insert(filepath_to_filename(weapon.weapon))
							local weapon = mine_file_now(nil, weapon.weapon)
							if not weapon.used_by
								weapon.used_by = table()
							end
							weapon.used_by:insert(filename)
						end
					end
				end
			end
		end
	end
	
	if rgd.GameData.ability_ext and rgd.GameData.ability_ext.abilities
		entity.abilities = table()
		for k,val in rgd.GameData.ability_ext.abilities
			if k != "$REF" and val != ""
				entity.abilities:insert(filepath_to_filename(val))
				local ability = mine_file_now(nil, val)
				if not ability.used_by
					ability.used_by = table()
				end
				ability.used_by:insert(filename)
			end
		end
	end
	
	if (rgd.GameData.cost_ext and rgd.GameData.cost_ext.time_cost) or rgd.GameData.population_ext
		entity.cost = {}
		if (rgd.GameData.cost_ext and rgd.GameData.cost_ext.time_cost)
			entity.cost.Manpower = rgd.GameData.cost_ext.time_cost.cost.manpower
			entity.cost.Fuel = rgd.GameData.cost_ext.time_cost.cost.fuel
			entity.cost.Munitions = rgd.GameData.cost_ext.time_cost.cost.munition
			entity.cost.Seconds = rgd.GameData.cost_ext.time_cost.time_seconds
		end
		if rgd.GameData.population_ext
			entity.cost.Popcap = rgd.GameData.population_ext.personnel_pop
			entity.cost["Medic Popcap"] = rgd.GameData.population_ext.medic_pop
			entity.cost["Vehicle Popcap"] = rgd.GameData.population_ext.vehicle_pop
		end
	end
	
	if rgd.GameData.engineer_ext and rgd.GameData.engineer_ext.construction_menus
		entity.can_construct = table()
		for _,entity_rgd in pairs_recursive(filemap.ebps.races) do
			if (not entity_rgd.path:find "[/_\\]sp[/_%.\\]")
				if entity_rgd.GameData.construction_ext and entity_rgd.GameData.construction_ext.construction_menus
					for engineer_k,engineer_val in rgd.GameData.engineer_ext.construction_menus
						if engineer_k != "$REF" and engineer_val.construction_type != ""
							for entity_k,entity_val in entity_rgd.GameData.construction_ext.construction_menus
								if entity_k != "$REF" and engineer_val.construction_type == entity_val.construction_type
									local entity_rgd_path = entity_rgd.path:after"\\attrib\\"
									entity.can_construct:insert(filepath_to_filename(entity_rgd_path))
									local other = mine_file_now(nil, entity_rgd_path)
									if not other.from
										other.from = table()
									end
									other.from:insert(filename)
								end
							end
						end
					end
				end
			end
		end
	end
	
	if rgd.GameData.requirement_ext and rgd.GameData.requirement_ext.requirement_table
		entity.creation_requirements = mine_requirements(rgd.GameData.requirement_ext.requirement_table, filename)
	end
	
	if rgd.GameData.moving_ext
		entity.mobility = {}
		entity.mobility["Acceleration rate"] = rgd.GameData.moving_ext.acceleration
		entity.mobility["Deceleration rate"] = rgd.GameData.moving_ext.deceleration
		entity.mobility["Rotation rate"] = rgd.GameData.moving_ext.rotation_rate
		entity.mobility["Maximum speed"] = rgd.GameData.moving_ext.speed_max
	end
	
	if rgd.GameData.health_ext
		entity.health = {}
		entity.health.HP = rgd.GameData.health_ext.hitpoints
		if rgd.GameData.health_ext.is_invincible
			entity.health.HP = "Invincible"
		end
		entity.health["HP Regeneration"] = rgd.GameData.health_ext.regeneration
		entity.health.Cuttable = rgd.GameData.health_ext.cuttable
		entity.health.Repairable = rgd.GameData.health_ext.can_repair
	end
	
	if entity.health and rgd.GameData.type_ext
		entity.health["Armour Type"] = filepath_to_filename(rgd.GameData.type_ext.type_target_weapon["$REF"])
		local type_target_weapon = mine_file_now(nil, rgd.GameData.type_ext.type_target_weapon["$REF"])
		if not type_target_weapon.examples
			type_target_weapon.examples = table()
		end
		type_target_weapon.examples:insert(filename)
	end
	
	if rgd.GameData.upgrade_ext
		local function mine_upgrade_table(t)
			local upgrade_list = table()
			for k,val in t
				if k != "$REF" and val != ""
					upgrade_list:insert(filepath_to_filename(val))
					local upgrade = mine_file_now(nil, val)
					if not upgrade.from
						upgrade.from = table()
					end
					upgrade.from:insert(filename)
				end
			end
			if #upgrade_list > 0
				return upgrade_list
			end
		end
		entity.upgrades = mine_upgrade_table(rgd.GameData.upgrade_ext.standard_upgrades)
		if rgd.GameData.upgrade_ext.number_of_standard_slots >= 0
			entity.upgrages_max = rgd.GameData.upgrade_ext.number_of_standard_slots
		end
		entity.addons = mine_upgrade_table(rgd.GameData.upgrade_ext.addon_upgrades)
		if rgd.GameData.upgrade_ext.number_of_addon_slots >= 0
			entity.addons_max = rgd.GameData.upgrade_ext.number_of_addon_slots
		end
		entity.transferrable_upgrades = mine_upgrade_table(rgd.GameData.upgrade_ext.transferrable_upgrades)
		if rgd.GameData.upgrade_ext.number_of_transferrable_slots >= 0
			entity.transferrable_upgrades_max = rgd.GameData.upgrade_ext.number_of_transferrable_slots
		end
	end
	
	entity.spawns = table()
	if rgd.GameData.spawner_ext and rgd.GameData.spawner_ext.squad_table
		entity.spawns_at_map_edge = rgd.GameData.spawner_ext.spawn_at_map_entry_point
		for k,val in rgd.GameData.spawner_ext.squad_table
			if k != "$REF" and val != ""
				local valhtml = val:beforelast".lua":gsub("[\\/]","_") .. ".html"
				entity.spawns:insert(valhtml)
				local spawned = mine_file_now(valhtml, val:beforelast".lua" .. ".rgd")
				if not spawned.from
					spawned.from = table()
				end
				spawned.from:insert(filename)
			end
		end
	end
	if entity.health and rgd.GameData.health_ext.wreck_entity and rgd.GameData.health_ext.wreck_entity != ""
		local wreckhtml = rgd.GameData.health_ext.wreck_entity:beforelast".lua":gsub("[\\/]","_") .. ".html"
		local wreckname = rgd.GameData.health_ext.wreck_entity:beforelast".lua" .. ".rgd"
		entity.wreck = wreckhtml
		
		local wreck = mine_file_now(wreckhtml, wreckname)
		if not wreck.wreck_of
			wreck.wreck_of = table()
		end
		wreck.wreck_of:insert(filename)
		if wreck.name == "$0 no key"
			wreck.name = "Generic Wreck"
		end
	end
	if rgd.GameData.type_ext and rgd.GameData.type_ext.unit_type_list
		entity.types = table()
		for k,type in rgd.GameData.type_ext.unit_type_list
			if k != "$REF" and type != ""
				entity.types:insert("type_unit_class_" .. type .. ".html")
				local type_unit_class = mine_file_now("type_unit_class_" .. type .. ".html", "type_unit_class\\" .. type .. ".rgd")
				if not type_unit_class.examples
					type_unit_class.examples = table()
				end
				type_unit_class.examples:insert(filename)
			end
		end
	end
	
	return entity
end
miners_index.ebps = mine_entity

function mine_squad(filename, rgd, filepath)
	local squad = mine_core(UCS(rgd.GameData.squad_ui_ext.screen_name), filename, filepath)
	squad.template = "squad"
	squad.object_type = "Squad"
	squad.cost_title = "Creation Cost (from a building)"
	squad.tweaker = tweak_squad
	
	if rgd.GameData.squad_requirement_ext and rgd.GameData.squad_requirement_ext.requirement_table
		squad.reinforce_requirements = mine_requirements(rgd.GameData.squad_requirement_ext.requirement_table, filename)
	end
	
	if rgd.GameData.squad_ability_ext and rgd.GameData.squad_ability_ext.abilities
		squad.abilities = table()
		for k,val in rgd.GameData.squad_ability_ext.abilities
			if k != "$REF" and val != ""
				squad.abilities:insert(filepath_to_filename(val))
				local ability = mine_file_now(nil, val)
				if not ability.used_by
					ability.used_by = table()
				end
				ability.used_by:insert(filename)
			end
		end
	end
	
	if rgd.GameData.squad_veterancy_ext and rgd.GameData.squad_veterancy_ext.veterancy_rank_info
		squad.veterancy = table()
		for rank=1,9 do
			local vrank = rgd.GameData.squad_veterancy_ext.veterancy_rank_info["veterancy_rank_0" .. rank]
			if not vrank or vrank.single_player_only
				break
			end
			squad.veterancy:insert{required_xp = vrank.experience_value, actions = mine_actions(vrank.squad_actions, filename)}
		end
	end
	
	if rgd.GameData.squad_loadout_ext and rgd.GameData.squad_loadout_ext.unit_list
		squad.comprised_of = table()
		for k,val in rgd.GameData.squad_loadout_ext.unit_list
			if k != "$REF" and val.type.type != ""
				squad.comprised_of:insert{entity = filepath_to_filename(val.type.type), num = val.num, max = val.max}
				local entity = mine_file_now(nil, val.type.type)
				if not entity.member_of
					entity.member_of = table()
				end
				entity.member_of:insert(filename)
				if entity.cost
					for k,v in entity.cost
						if not squad.cost
							squad.cost = {}
						end
						if squad.cost[k]
							squad.cost[k] = squad.cost[k] + (val.num * v)
						else
							squad.cost[k] = (val.num * v)
						end
					end
				end
			end
		end
	end
	
	return squad
end
miners_index.sbps = mine_squad

function mine_slot_item(filename, rgd, filepath)
	local name = UCS(rgd.GameData.slot_item_bag.ui_info.screen_name)
	if name == "$0 no key"
		name = rgd.GameData.slot_item_bag.ui_info.hotkey_name:gsub("_"," ")
		if name == ""
			name = rgd.name:beforelast".":gsub("_"," ")
			name = name:sub(1,1):upper() .. name:sub(2)
		end
	end
	local slot_item = mine_core(name, filename, filepath)
	slot_item.template = "slot_item"
	slot_item.object_type = "Slot Item"
	
	return slot_item
end
miners_index.slot_item = mine_slot_item

function mine_upgrade(filename, rgd, filepath)
	local upgrade = mine_core(UCS(rgd.GameData.upgrade_bag.ui_info.screen_name), filename, filepath)
	upgrade.template = "upgrade"
	upgrade.from_title = "Can be researched from"
	upgrade.object_type = "Upgrade"
	
	upgrade.requirements = mine_requirements(rgd.GameData.upgrade_bag.requirements, filename)
	upgrade.actions = mine_actions(rgd.GameData.upgrade_bag.actions, filename)
	
	upgrade.time_takes = rgd.GameData.upgrade_bag.time_cost.time_seconds
	upgrade.cost = {}
	upgrade.cost.Manpower = rgd.GameData.upgrade_bag.time_cost.cost.manpower
	upgrade.cost.Fuel = rgd.GameData.upgrade_bag.time_cost.cost.fuel
	upgrade.cost.Munitions = rgd.GameData.upgrade_bag.time_cost.cost.munition
	upgrade.cost["Command Points"] = rgd.GameData.upgrade_bag.time_cost.cost.command
	
	return upgrade
end
miners_index.upgrade = mine_upgrade

function mine_commander_tree(filename, rgd, filepath)
	local tree = mine_core(UCS(rgd.GameData.commander_tree_bag.name), filename, filepath)
	tree.template = "commander_tree"
	tree.object_type = "Commander Tree"
	
	if rgd.GameData.commander_tree_bag.lock_upgrade and rgd.GameData.commander_tree_bag.lock_upgrade != ""
		local upgrade = mine_file_now(nil, rgd.GameData.commander_tree_bag.lock_upgrade)
		if not upgrade.from
			upgrade.from = table()
		end
		upgrade.from:insert(filename)
		if upgrade.name == "$0 no key"
			upgrade.name = tree.name
		end
	end
	
	tree.branches = {}
	for row = 0,2
		if rgd.GameData.commander_tree_bag["upgrade_row0" .. row .. "_col00"]
			tree.branches[row+1] = {}
			for col = 0,2
				local val = rgd.GameData.commander_tree_bag["upgrade_row0" .. row .. "_col0" .. col]
				if val != ""
					tree.branches[row+1][col+1] = val:beforelast".lua":gsub("[\\/]","_") .. ".html"
					local upgrade = mine_file_now(tree.branches[row+1][col+1], val:beforelast".lua" .. ".rgd")
					if not upgrade.from
						upgrade.from = table()
					end
					upgrade.from:insert(filename)
				end
			end
		end
	end
	
	return tree
end
miners_index.commander_tree = mine_commander_tree

function mine_race(filename, rgd, filepath)
	local race = mine_core(UCS(rgd.GameData.race_bag.race_path.name_id), filename, filepath)
	race.template = "race"
	race.object_type = "Army"
	
	race.starting_resources = {}
	race.starting_resources.Fuel = rgd.GameData.race_bag.starting_resources.fuel
	race.starting_resources.Manpower = rgd.GameData.race_bag.starting_resources.manpower
	race.starting_resources.Munitions = rgd.GameData.race_bag.starting_resources.munition
	race.commander_trees = table()
	for i=1,3
		local file = rgd.GameData.race_bag["company_commander_tree_0" .. i]
		race.commander_trees:insert( filepath_to_filename(file) )
		local commander_tree = mine_file_now(nil, file)
		if not commander_tree.armies
			commander_tree.armies = table()
		end
		commander_tree.armies:insert(filename)
	end
	race.medic_spawn_squad = rgd.GameData.race_bag.squad_to_spawn:beforelast".lua":gsub("[\\/]","_") .. ".html"
	todo_mine_file(race.starting_squad, rgd.GameData.race_bag.squad_to_spawn:beforelast".lua" .. ".rgd")
	race.command_abilities = table()
	for k,val in rgd.GameData.race_bag.abilities
		if k != "$REF" and val != ""
			if (not val:find "[/_\\]sp[/_%.\\]")
				local abil = mine_file_now(nil, val)
				if abil.name != "$0 no key"
					if not abil.is_command_ability
						abil.is_command_ability = table()
					end
					abil.is_command_ability:insert(filename)
					race.command_abilities:insert(filepath_to_filename(val))
				end
			end
		end
	end

	if type(filemap.ebps.races[rgd.GameData.race_bag.race_path.path_name].buildings["hq.rgd"]) == "userdata"
		race.hq = "ebps_races_" .. rgd.GameData.race_bag.race_path.path_name .. "_buildings_hq.html"
		local hq = mine_file_now(race.hq, "ebps\\races\\" .. rgd.GameData.race_bag.race_path.path_name .. "\\buildings\\hq.rgd")
		if not hq.from
			hq.from = table()
		end
		hq.from:insert(filename)
		race.starting_squads = hq.spawns
	elseif type(filemap.ebps.races[rgd.GameData.race_bag.race_path.path_name].vehicles["hq.rgd"]) == "userdata"
		race.hq = "ebps_races_" .. rgd.GameData.race_bag.race_path.path_name .. "_vehicles_hq.html"
		local hq = mine_file_now(race.hq, "ebps\\races\\" .. rgd.GameData.race_bag.race_path.path_name .. "\\vehicles\\hq.rgd")
		if not hq.from
			hq.from = table()
		end
		hq.from:insert(filename)
		race.starting_squads = hq.spawns
	end
	
	return race
end
miners_index.racebps = mine_race