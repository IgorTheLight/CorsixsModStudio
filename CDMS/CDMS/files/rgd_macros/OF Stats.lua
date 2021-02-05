-- Loader script for OF Stats Generation

-- Utility function to load parts of the script, and never load a part twice
function include(file)
	local file_lower = file:lower()
	if loaded[file_lower]
		return
	end
	loaded[file_lower] = true
	dofile("Mod_studio_files/rgd_macros/of_stats/" .. file .. ".lua")
end
loaded = {}

-- Include the script core
include "OF Stats"

-- Catch any errors within at_end and display a lovely stack traceback
do
	local at_end_orig = at_end
	local function error_handler(e)
		local trace = debug.traceback()
		print(e .. "\r\n" .. trace)
	end
	at_end = function() xpcall(at_end_orig, error_handler) end
end
