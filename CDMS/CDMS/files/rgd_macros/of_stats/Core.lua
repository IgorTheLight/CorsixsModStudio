
-- ** Util: Access all files ** --
function each_file(rgd)
	local map_part = _G
	local key = "filemap"
	for _,part in ipairs(rgd.path:lower():after"\\attrib\\":split"\\")
		map_part = map_part[key]
		key = part
	end
	map_part[key] = rgd
end

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
