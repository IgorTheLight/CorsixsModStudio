-- Just run the macro
function at_end()
	print "String functions:"
	text = "fred\\bob\\jim.bo.txt"
	for _,k in {"\\", ".", "bo"} do
		for f in {before =_, after =_, afterlast =_, beforelast =_}
			print("string." .. f .. "(\"" .. text .. "\",\"" .. k .. "\") => " .. string[f](text,k))
		end
	end
	split = "string.split(\"" .. text .. "\",\"[.\\]\") => {"
	for k,v in ipairs(text:split("[.\\]"))
		split = split .. "\"" .. v .. "\", "
	end
	print(split .. "}\n")
	
	print "Math functions:"
	for n = 0,10
		print("math.clamp(" .. n .. ",2.4,8) => " .. n:clamp(2.4,8))
	end
end
