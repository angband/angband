-- tolua: container abstract class
-- Written by Waldemar Celes
-- TeCGraf/PUC-Rio
-- Jul 1998
-- $Id: container.lua,v 1.2 2001/11/08 17:34:50 angband Exp $

-- This code is free software; you can redistribute it and/or modify it.
-- The software provided hereunder is on an "as is" basis, and
-- the author has no obligation to provide maintenance, support, updates,
-- enhancements, or modifications. 


-- Container class
-- Represents a container of features to be bound
-- to lua.
classContainer = 
{
 curr = nil,
 _base = classFeature,
}
settag(classContainer,tolua_tag)

-- output tags
function classContainer:decltag ()
 push(self)
 local i=1
 while self[i] do
  self[i]:decltag()
  i = i+1
 end
 pop()
end


-- write support code
function classContainer:supcode ()
 push(self)
 local i=1
 while self[i] do
  self[i]:supcode()
  i = i+1
 end
 pop()
end


-- Internal container constructor
function _Container (self)
 self._base = classContainer
 settag(self,tolua_tag)
 self.n = 0
 self.typedefs = {n=0}
 self.lnames = {}
 return self
end

-- push container
function push (t)
 classContainer.curr = t
end

-- pop container
function pop ()
 classContainer.curr = classContainer.curr.parent
end

-- append to current container
function append (t)
 return classContainer.curr:append(t)
end
 
-- append typedef to current container
function appendtypedef (t)
 return classContainer.curr:appendtypedef(t)
end
 
-- substitute typedef
function findtypedef (type)
 return classContainer.curr:findtypedef(type)
end

-- check if is typedef
function istypedef (type)
 return classContainer.curr:istypedef(type)
end

-- append feature to container
function classContainer:append (t)
 self.n = self.n + 1
 self[self.n] = t
 t.parent = self
end

-- append typedef 
function classContainer:appendtypedef (t)
 self.typedefs.n = self.typedefs.n + 1
 self.typedefs[self.typedefs.n] = t
end

-- determine lua function name overload
function classContainer:overload (lname)
 if not self.lnames[lname] then 
  self.lnames[lname] = 0
 else
  self.lnames[lname] = self.lnames[lname] + 1
 end
 return format("%02d",self.lnames[lname])
end

function classContainer:findtypedef (type)
 local env = self
 while env do
  if env.typedefs then
   local i=1
   while env.typedefs[i] do
    if env.typedefs[i].utype == type then
	 local mod1,type1 = env.typedefs[i].mod,env.typedefs[i].type
         local mod2,type2 = findtypedef(type1)
         return mod2..' '..mod1,type2
	end
	i = i+1
   end
  end
  env = env.parent
 end
 return '',type
end 

function classContainer:istypedef (type)
 local env = self
 while env do
  if env.typedefs then
   local i=1
   while env.typedefs[i] do
    if env.typedefs[i].utype == type then
         return 1
        end
        i = i+1
   end
  end
  env = env.parent
 end
 return nil 
end

-- parse chunk
function classContainer:doparse (s)

 -- try module
 do
  local b,e,name,body = strfind(s,"^%s*module%s%s*([_%w][_%w]*)%s*(%b{})%s*")
  if b then
   _curr_code = strsub(s,b,e)
   Module(name,body)
   return strsub(s,e+1)
  end
 end

 -- try define
 do
  local b,e,name = strfind(s,"^%s*#define%s%s*([^%s]*)[^\n]*\n%s*")
  if b then
   _curr_code = strsub(s,b,e)
   Define(name)
   return strsub(s,e+1)
  end
 end

 -- try enumerates
 do
  local b,e,body = strfind(s,"^%s*enum[^{]*(%b{})%s*;?%s*")
  if b then
   _curr_code = strsub(s,b,e)
   Enumerate(body)
   return strsub(s,e+1)
  end
 end

 do
  local b,e,body,name = strfind(s,"^%s*typedef%s%s*enum[^{]*(%b{})%s*([%w_][^%s]*)%s*;%s*")
  if b then
   _curr_code = strsub(s,b,e)
   Enumerate(body)
   Typedef("int "..name)
   return strsub(s,e+1)
  end
 end

 -- try operator 
 do
  local b,e,decl,kind,arg,const = strfind(s,"^%s*([_%w][_%w%s%*&]*operator)%s*([^%s][^%s]*)%s*(%b())%s*(c?o?n?s?t?)%s*;%s*")
  if b then
   _curr_code = strsub(s,b,e)
   Operator(decl,kind,arg,const)
   return strsub(s,e+1)
  end
 end

 -- try function
 do
  local b,e,decl,arg,const = strfind(s,"^%s*([~_%w][_@%w%s%*&]*[_%w])%s*(%b())%s*(c?o?n?s?t?)%s*=?%s*0?%s*;%s*")
  if not b then
   -- try a single letter function name
   b,e,decl,arg,const = strfind(s,"^%s*([_%w])%s*(%b())%s*(c?o?n?s?t?)%s*;%s*")
  end
  if b then
   _curr_code = strsub(s,b,e)
   Function(decl,arg,const)
   return strsub(s,e+1)
  end
 end

 -- try inline function
 do
  local b,e,decl,arg,const = strfind(s,"^%s*([~_%w][_@%w%s%*&]*[_%w])%s*(%b())%s*(c?o?n?s?t?)%s*%b{}%s*")
  if not b then
   -- try a single letter function name
   b,e,decl,arg,const = strfind(s,"^%s*([_%w])%s*(%b())%s*(c?o?n?s?t?)%s*%b{}%s*")
  end
  if b then
   _curr_code = strsub(s,b,e)
   Function(decl,arg,const)
   return strsub(s,e+1)
  end
 end

 -- try class
 do
  local b,e,name,base,body = strfind(s,"^%s*class%s*([_%w][_%w]*)%s*(.-)%s*(%b{})%s*;%s*")
  if not b then
   b,e,name,base,body = strfind(s,"^%s*struct%s*([_%w][_%w]*)%s*(.-)%s*(%b{})%s*;%s*")
   if not b then
    base = ''
    b,e,body,name = strfind(s,"^%s*typedef%s%s*struct%s%s*[_%w]*%s*(%b{})%s*([_%w][_%w]*)%s*;%s*")
   end
  end
  if b then
   if base ~= '' then 
    local b,e
	b,e,base = strfind(base,".-([_%w][_%w]*)$") 
   end
   _curr_code = strsub(s,b,e)
   Class(name,base,body)
   return strsub(s,e+1)
  end
 end

 -- try typedef
 do
  local b,e,types = strfind(s,"^%s*typedef%s%s*(.-)%s*;%s*")
  if b then
   _curr_code = strsub(s,b,e)
   Typedef(types)
   return strsub(s,e+1)
  end
 end

 -- try variable
 do
  local b,e,decl = strfind(s,"^%s*([_%w][_@%s%w%d%*&]*[_%w%d])%s*;%s*")
  if b then
   _curr_code = strsub(s,b,e)
   Variable(decl)
   return strsub(s,e+1)
  end
 end

 -- try array
 do
  local b,e,decl = strfind(s,"^%s*([_%w][][_@%s%w%d%*&%-%>]*[]_%w%d])%s*;%s*")
  if b then
   _curr_code = strsub(s,b,e)
   Array(decl)
   return strsub(s,e+1)
  end
 end

 -- try code
 do
  local b,e,code = strfind(s,"^%s*(%b\1\2)")
  if b then
   Code(strsub(code,2,-2))
   return strsub(s,e+1)
  end 
 end 

 -- try verbatim
 do
  local b,e,line = strfind(s,"^%s*%$(.-\n)")
  if b then
   Verbatim(line)
   return strsub(s,e+1)
  end 
 end 

 -- no matching
 if gsub(s,"%s%s*","") ~= "" then
  _curr_code = s
  error("#parse error")
 else
  return ""
 end
end

function classContainer:parse (s)
 while s ~= '' do
  s = self:doparse(s)
 end
end


