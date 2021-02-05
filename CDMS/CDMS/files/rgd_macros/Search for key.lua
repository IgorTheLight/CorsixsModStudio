-- Each rgd is passed into this function:
function each_file(rgd)
	-- Iterate over all root tables in the RGD (eg. GameData)
	for key,val in rgd do
		modify_table(val)
	end
end

function modify_table(t)
	for key,val in t do -- Look through all entries in the table:
		if type(val) == "userdata" then -- Recursively search child tables:
			modify_table(val)
		elseif type(val) == "string" then
			
		end
	end
end

-- Whenever an example is found, this function is called
function found_one()
	print("-- Found in file ",current_file_name," :")
	if (max_examples_to_find or 0) > 0 then
		max_examples_to_find = max_examples_to_find - 1
		if max_examples_to_find < 1 then
			each_file = nil
		end
	end
end
