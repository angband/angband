-- tolua: basic utility functions
-- Written by Waldemar Celes
-- TeCGraf/PUC-Rio
-- Jul 1998
-- $Id: basic.lua,v 1.1 2001/10/27 19:35:28 angband Exp $

-- This code is free software; you can redistribute it and/or modify it.
-- The software provided hereunder is on an "as is" basis, and
-- the author has no obligation to provide maintenance, support, updates,
-- enhancements, or modifications.


-- Basic C types and their corresponding Lua types
-- All occurrences of "char*" will be replaced by "_cstring",
-- and all occurrences of "void*" will be replaced by "_userdata" 
_basic = {
 ['void'] = '',
 ['char'] = 'number',
 ['int'] = 'number',
 ['short'] = 'number',
 ['long'] = 'number',
 ['_cstring'] = 'string',
 ['_userdata'] = 'userdata',
 ['char*'] = 'string',
 ['void*'] = 'userdata',
 ['bool'] = 'bool',
 ['LUA_VALUE'] = 'value',
 ['byte'] = 'number',
 ['s16b'] = 'number',
 ['u16b'] = 'number',
 ['s32b'] = 'number',
 ['u32b'] = 'number',
}

_basic_tag = {
 ['void'] = '',
 ['char'] = 'LUA_TNUMBER',
 ['int'] = 'LUA_TNUMBER',
 ['short'] = 'LUA_TNUMBER',
 ['long'] = 'LUA_TNUMBER',
 ['_cstring'] = 'LUA_TSTRING',
 ['_userdata'] = 'LUA_TUSERDATA',
 ['char*'] = 'LUA_TSTRING',
 ['void*'] = 'LUA_TUSERDATA',
 ['bool'] = 'tolua_tag(tolua_S,"bool")',
 ['byte'] = 'LUA_TNUMBER',
 ['s16b'] = 'LUA_TNUMBER',
 ['u16b'] = 'LUA_TNUMBER',
 ['s32b'] = 'LUA_TNUMBER',
 ['u32b'] = 'LUA_TNUMBER',
}

_basic_ctype = {
 number = "long",
 string = "const char*",
 userdata = "void*",
 bool = "int",
}

-- List of user defined types
-- Each type corresponds to a variable name that stores its tag value.
_usertype = {}

-- Tag method to provide inheritance
function tolua_index (t,f)
 if f == '_base' then  -- to avoid loop
   return tolua_old_index(t,f)
 else
  return t._base[f]
 end
end

tolua_tag = newtag()
tolua_old_index = settagmethod(tolua_tag,"index",tolua_index)

-- Error handler
function tolua_error (s)
 local out = _OUTPUT
 _OUTPUT = _STDERR
 if strsub(s,1,1) == '#' then
  write("\n** tolua: "..strsub(s,2)..".\n\n")
 else
  write("\n** tolua internal error: "..s..".\n\n")
  return
 end

 if _curr_code then
  local _,_,s = strfind(_curr_code,"^%s*(.-\n)") -- extract first line
  if s==nil then s = _curr_code end
  s = gsub(s,"_userdata","void*") -- return with 'void*'
  s = gsub(s,"_cstring","char*")  -- return with 'char*'
  write("Code being processed:\n"..s.."\n")
 end
 _OUTPUT = out
end


_ERRORMESSAGE = tolua_error

-- register an user defined type
function regtype (t)
 if not istype(t) then
  _usertype[t] = t
 end
 return t
end

-- return tag name
function tagvar(type,const)
 if type == '' or type == 'void' then
  return type,0
 else
  local m,t = findtypedef(type)
  if isbasic(t) then
   return t, _basic_tag[t]
  end
  if strfind(m,'const') then const = 'const' end
  regtype(t)
  if const and const ~= '' then
   t = 'const '..t
  end
  return t,'tolua_tag(tolua_S,"'..t..'")'
 end
end

-- check if basic type
function isbasic (type)
 local m,t = findtypedef(type)
 local b = _basic[t]
 if b then
  return b,_basic_ctype[b]
 end
 return nil
end

-- check if type
function istype (t)
 return _basic[t] or _usertype[t] or istypedef(t)
end


-- split string using a token
function split (s,t)
 local l = {n=0}
 local f = function (s)
  %l.n = %l.n + 1
  %l[%l.n] = s
 end
 local p = "%s*(.-)%s*"..t.."%s*"
 s = gsub(s,"^%s+","")
 s = gsub(s,"%s+$","")
 s = gsub(s,p,f)
 l.n = l.n + 1
 l[l.n] = gsub(s,"(%s%s*)$","")
 return l
end


-- concatenate strings of a table
function concat (t,f,l)
 local s = ''
 local i=f
 while i<=l do
  s = s..t[i]
  i = i+1
  if i <= l then s = s..' ' end
 end
 return s
end

-- output line
function output (...)
 local i=1
 while i<=arg.n do
  if _cont and not strfind(_cont,'[%(,"]') and 
     strfind(arg[i],"^[%a_~]") then 
	    write(' ') 
  end
  write(arg[i])
  if arg[i] ~= '' then
   _cont = strsub(arg[i],-1,-1)
  end
  i = i+1
 end
 if strfind(arg[arg.n],"[%/%)%;%{%}]$") then 
  _cont=nil write('\n')
 end
end


