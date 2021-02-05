function at_end()
	print "tables:"
	t = table{
		a = "moo",
		b = "boo",
		c = "cow",
		d = "ben",
		f = "goo",
		g = "bag",
		h = "meh",
		"123",
		"321",
		"567"
	}
	t:filter([(k,v)|v:find(k)])
	for k,v in t print(k,"=",v) end
	
	print "strings:"
	print( t.g:formatself'%q' )
	
	print "math:"
	print( math.pi:formatself'%.3e' )
end
