-- Run this over the entire attrib folder to
-- get a list of files with "type" in the name
-- Edit line 8 if desired

print "List of types:"

function each_file(rgd)
	if rgd.name:find "type" then
		print(rgd.name," (",rgd.path,")")
	end
end

function at_end()
	print "end of list"
end
