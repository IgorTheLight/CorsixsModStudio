function Reference(file)
	local loaded,rgd = pcall(loadRgd,
	"attrib\\attrib\\" .. file .. ".rgd")
	if loaded and rgd then
		rgd.GameData["$REF"] = file .. ".lua"
		return rgd.GameData
	end
end

function each_file(rgd)
	if not rgd.GameData.squad_paradrop_ext then
		rgd.GameData.squad_paradrop_ext =
			Reference("sbpextensions\\squad_paradrop_ext")
		rgd:save()
	end
end
