function error_handler(e)
	local trace = debug.traceback()
	if not trace | e end
	trace_parts = trace:split "\n"
	trace = ""
	for n = #trace_parts, 2, -1
		trace = trace .. trace_parts[n]:after"\t" .. "\n"
	end
	trace_parts = e:split"\r\n"
	for _,part in ipairs(trace_parts)
	  if part ~= ""
		trace = trace .. "[" .. part:before" line " ..
		"]:" .. part:after" line ":before": " ..
		": " .. part:after": " .. "\n"
	  end
	end
	return trace
end
 
function at_end()
	loaded,rgd = xpcall([| loadRgd(
		[[attrib\attrib\bags\ability_bag.rgd]]
		)] , error_handler)
	if loaded and rgd
		-- rgd was loaded
		for k in rgd.GameData do print(k) end
	else
		if rgd
			-- rgd is the error message
			print("Error: Cannot load RGD. Stack trace:\n"
				.. rgd)
		else
			-- permission denied to load rgds
			print "Error: Not permitted to load RGD"
		end
	end
end
