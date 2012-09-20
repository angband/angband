-- tolua: abstract feature class
-- Written by Waldemar Celes
-- TeCGraf/PUC-Rio
-- Jul 1998
-- $Id: feature.lua,v 1.1 2001/10/27 19:35:29 angband Exp $

-- This code is free software; you can redistribute it and/or modify it.
-- The software provided hereunder is on an "as is" basis, and
-- the author has no obligation to provide maintenance, support, updates,
-- enhancements, or modifications. 


-- Feature class
-- Represents the base class of all mapped feature.
classFeature = {
}

-- write support code
function classFeature:supcode ()
end

-- output tag
function classFeature:decltag ()
end

-- register feature
function classFeature:register ()
end

-- unregister feature
function classFeature:unregister ()
end

-- translate verbatim
function classFeature:preamble ()
end

-- check if feature is inside a class definition
-- it returns the feature class name or nil.
function classFeature:inclass ()
 if self.parent and self.parent.type == 'class' then
  return self.parent.name
 else
  return nil
 end
end

-- check if feature is inside a module
-- it returns the feature module name or nil.
function classFeature:inmodule ()
 if self.parent and self.parent.type == 'module' then
  return self.parent.name
 else
  return nil
 end
end

-- return C binding function name based on name
-- the client specifies a prefix
-- return C binding function name
-- the client specifies a prefix
function classFeature:cfuncname (n)
 if self.parent then
  n = self.parent:cfuncname(n)
 end
 if self.lname then
  return n..'_'..self.lname
 else
  return n..'_'..self.name
 end
end

