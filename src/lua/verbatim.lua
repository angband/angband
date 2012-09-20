-- tolua: verbatim class
-- Written by Waldemar Celes
-- TeCGraf/PUC-Rio
-- Jul 1998
-- $Id: verbatim.lua,v 1.1 2001/10/27 19:35:29 angband Exp $

-- This code is free software; you can redistribute it and/or modify it.
-- The software provided hereunder is on an "as is" basis, and
-- the author has no obligation to provide maintenance, support, updates,
-- enhancements, or modifications. 



-- Verbatim class
-- Represents a line translated directed to the binding file.
-- The following filds are stored:
--   line = line text
classVerbatim = {
 line = '',
 _base = classFeature,
}
settag(classVerbatim,tolua_tag)

-- preamble verbatim
function classVerbatim:preamble ()
 if not self.cond then
  write(self.line)
 end
end

-- support code
function classVerbatim:supcode ()
 if self.cond then
  write(self.line)
  write('\n')
 end
end

-- register code
function classVerbatim:register ()
 if self.cond then
  write(self.line)
 end
end
 

-- Print method
function classVerbatim:print (ident,close)
 print(ident.."Verbatim{")
 print(ident.." line = '"..self.line.."',")
 print(ident.."}"..close)
end


-- Internal constructor
function _Verbatim (t)
 t._base = classVerbatim
 settag(t,tolua_tag)
 append(t)
 return t
end

-- Constructor
-- Expects a string representing the text line
function Verbatim (l)
 local c
 if strsub(l,1,1) == '$' then
  c = 1
  l = strsub(l,2)
 end
 return _Verbatim {
  line = l,
  cond = c
 }
end


