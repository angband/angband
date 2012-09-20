-- Generate binding code
-- Written by Waldemar Celes
-- TeCGraf/PUC-Rio
-- Jul 1998
-- $Id: doit.lua,v 1.1 2001/10/27 19:35:29 angband Exp $


-- This code is free software; you can redistribute it and/or modify it.
-- The software provided hereunder is on an "as is" basis, and
-- the author has no obligation to provide maintenance, support, updates,
-- enhancements, or modifications. 


-- open input file, if any
if flags.f then
 local st, msg = readfrom(flags.f)
 if not st then
  error('#'..msg)
 end
end

-- define package name, if not provided
if not flags.n then
 if flags.f then
  flags.n = gsub(flags.f,"%..*","")
 else
  error("#no package name nor input file provided")
 end
end

local p  = Package(flags.n)

if flags.f then
 readfrom()
end

if flags.p then
 return        -- only parse
end

if flags.o then
 local st,msg = writeto(flags.o)
 if not st then
  error('#'..msg)
 end
end

if flags.P then
 p:print()
else
 p:decltag()
 p:preamble()
 p:supcode()
 p:register()
 p:unregister()
end

if flags.o then
 writeto()
end

-- write header file
if not flags.P then
 if flags.H then
  local st,msg = writeto(flags.H)
  if not st then
   error('#'..msg)
  end
  p:header()
  writeto()
 end
end

