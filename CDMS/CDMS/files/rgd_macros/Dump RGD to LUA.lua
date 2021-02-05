function each_file(rgd)
	for key, val in rgd
		dump_table(val, key, "Inherit")
	end
end

function dump_table(t, prefix, func)
	print(prefix," = ",func,"([[",t["$REF"] or "","]])")
	for key, val in t
		local newprefix = prefix .. "[" .. key:formatself'%q' .. "]"
		if type(val) == "userdata"
			dump_table(val, newprefix, "Reference")
		elseif type(val) != "string"
			print(newprefix," = ", val)
		elseif key != "$REF"
			print(newprefix," = ", val:formatself'%q')
		end
	end
end
