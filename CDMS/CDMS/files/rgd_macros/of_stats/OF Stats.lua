-- Defines where to output to
output_folder = [[E:\HTML\Apache\users\coh\stats\]]

-- The Core maps all RGDs into the filemap variable
include "Core"

-- Data extraction from RGDs is done with a "miner" function
include "Mine"

-- Data can then be tweaked
include "Tweakers"

-- Data output is done with templates
include "Templates"

LOG_NOTICES = true

-- Think of this as the script entry point:
function at_end()
	-- There are two terms we use:
	--  filename := the name of the HTML page made from an RGD, eg. "racebps_allied_rifle_company.html"
	--  filepath := the name of the RGD itself, eg. "racebps\allied_rifle_company.(rgd|lua)"
	--  object := the LUA data structure containing the daa mined from an RGD
	
	-- To find an object by its filename, use filename_index[filename]
	filename_index = {}
	
	-- A list of files that we need to mine sometime
	files_todo = table()
	
	-- Seed the script with the RaceBPs files, and let it crawl from there
	for filename,rgd in pairs_recursive(filemap.racebps) do
		todo_mine_file(nil, "racebps\\" .. filename)
	end
	
	-- Mine each of the RGDs still to do
	while (#files_todo) > 0
		mine_file_now(nil, files_todo[#files_todo])
	end
	
	-- Allow each object to tweak itself now that all files have been loaded
	for _,o in filename_index
		if o.tweaker
			o:tweaker()
		end
	end
	
	-- Print each object using its specified template
	for _,o in filename_index
		if LOG_NOTICES
			print("Printing " .. o.filename)
		end
		if template[o.template]
			template[o.template](o)
		else
			print("Cannot find template " .. (o.template or "(no template)"))
		end
	end
	
	-- Generate all breadcrumbs pages that were linked to during the object output phase
	for filename,path in breadcrumbs_todo
		if not filename_index[filename]
			make_breadcrumbs(filename,path)
		end
	end
end

-- Will mine the specified file and return the object
-- todo := required, the filepath of the file to mine, with a ".lua" or ".rgd" extension
-- filename := the filename of the file to mine, not required any more
function mine_file_now(filename, todo)
	-- Check parameters
	if todo:find".lua" then todo = todo:beforelast".lua" .. ".rgd" end
	filename = filename or filepath_to_filename(todo)
	
	-- Remove this file from the todo list
	for i,v in ipairs(files_todo)
		if v:lower() == todo:lower()
			files_todo:remove(i)
			break
		end
	end
	
	-- If the file is already loaded, return that object
	if filename and filename_index[filename]
		return filename_index[filename]
	end
	
	-- Find the file in the filemap
	local map_part = _G
	local key = "filemap"
	local parts = todo:split"\\"
	for _,part in ipairs(parts)
		map_part = map_part[key]
		key = part
	end
	local rgd = map_part[key]
	
	if LOG_NOTICES
		print("Mining " .. todo .. "(" .. parts[1] .. ")")
	end
	
	-- Find a suitable miner function
	local mine_fn = miners_index[parts[1]]
	if mine_fn
		return mine_fn(filename, rgd, todo)
	else
		print("No miner found for " .. todo .. " (" .. filename .. ")")
	end
end

-- Mark a file for mining later
function todo_mine_file(filename, filepath)
	if filepath:find".lua" then filepath = filepath:beforelast".lua" .. ".rgd" end
	filename = filename or filepath_to_filename(filepath)
	if not filename_index[filename]
		files_todo:insert(filepath)
	end
end

-- Convert a filepath to a filename
function filepath_to_filename(filepath)
	return filepath:beforelast".":gsub("[\\/]","_") .. ".html"
end
