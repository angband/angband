-- tolua: declaration class
-- Written by Waldemar Celes
-- TeCGraf/PUC-Rio
-- Jul 1998
-- $Id: declaration.lua,v 1.1 2001/10/27 19:35:28 angband Exp $

-- This code is free software; you can redistribute it and/or modify it.
-- The software provided hereunder is on an "as is" basis, and
-- the author has no obligation to provide maintenance, support, updates,
-- enhancements, or modifications. 


-- Declaration class
-- Represents variable, function, or argument declaration.
-- Stores the following fields:
--  mod  = type modifiers
--  type = type
--  ptr  = "*" or "&", if representing a pointer or a reference
--  name = name
--  dim  = dimension, if a vector
--  def  = default value, if any (only for arguments)
--  ret  = "*" or "&", if value is to be returned (only for arguments)
classDeclaration = {
 _base = classFeature,
 mod = '',
 type = '',
 ptr = '',
 name = '',
 dim = '',
 ret = '',
 def = ''
}
settag(classDeclaration,tolua_tag)

-- Create an unique variable name
function create_varname ()
 if not _varnumber then _varnumber = 0 end
 _varnumber = _varnumber + 1
 return "tolua_var_".._varnumber
end

-- Check declaration name
-- It also identifies default values
function classDeclaration:checkname ()

 if strsub(self.name,1,1) == '[' and not istype(self.type) then
  self.name = self.type..self.name
  local m = split(self.mod,'%s%s*')
  self.type = m[m.n]
  self.mod = concat(m,1,m.n-1)
 end

 local t = split(self.name,'=')
 if t.n==2 then
  self.name = t[1]
  self.def = t[t.n]
 end

 local b,e,d = strfind(self.name,"%[(.-)%]")
 if b then
  self.name = strsub(self.name,1,b-1)
  self.dim = d
 end


 if self.type ~= '' and self.type ~= 'void' and self.name == '' then
  self.name = create_varname()
 elseif self.kind=='var' then
  if self.type=='' and self.name~='' then
   self.type = self.type..self.name
   self.name = create_varname()
  elseif istype(self.name) then
   if self.type=='' then self.type = self.name
   else self.type = self.type..' '..self.name end
   self.name = create_varname()
  end
 end

end

-- Check declaration type
-- Substitutes typedef's.
function classDeclaration:checktype ()

 -- check if there is a pointer to basic type
 if isbasic(self.type) and self.ptr~='' then
  self.ret = self.ptr
  self.ptr = nil
 end

 -- check if there is array to be returned
 if self.dim~='' and self.ret~='' then
   error('#invalid parameter: cannot return an array of values')
 end

 -- register type
 if self.type~='' then
  regtype(self.type)
 end

 -- restore 'void*' and 'string*'
 if self.type == '_userdata' then self.type = 'void*'
 elseif self.type == '_cstring' then self.type = 'char*'
 end

--
-- -- if returning value, automatically set default value
-- if self.ret ~= '' and self.def == '' then
--  self.def = '0'
-- end
--

end

-- Print method
function classDeclaration:print (ident,close)
 print(ident.."Declaration{")
 print(ident.." mod  = '"..self.mod.."',")
 print(ident.." type = '"..self.type.."',")
 print(ident.." ptr  = '"..self.ptr.."',")
 print(ident.." name = '"..self.name.."',")
 print(ident.." dim  = '"..self.dim.."',")
 print(ident.." def  = '"..self.def.."',")
 print(ident.." ret  = '"..self.ret.."',")
 print(ident.."}"..close)
end

-- declare tag
function classDeclaration:decltag ()
 self.itype, self.tag = tagvar(self.type,strfind(self.mod,'const'))
end


-- output type checking
function classDeclaration:outchecktype (narg)
 local tag, def
 if self.dim ~= '' then 
  tag = 'LUA_TTABLE'
  def = 0
 else
  tag = self.tag
  def = self.def~='' or 0
 end
 return 'tolua_istype(tolua_S,'..narg..','..tag..','..def..')'
end

-- Declare variable
function classDeclaration:declare (narg)
 local ptr = ''
 if self.ptr~='' then ptr = '*' end
 output(" ",self.mod,self.type,ptr)
 if self.dim ~= '' and tonumber(self.dim)==nil then
  output('*')
 end 
 output(self.name)
 if self.dim ~= '' then
  if tonumber(self.dim)~=nil then
   output('[',self.dim,'];')
  else
   output(' = (',self.mod,self.type,ptr,'*)',
          'malloc(',self.dim,'*sizeof(',self.type,ptr,'));')
  end
 else
  local t = isbasic(self.type)
  output(' = ')
  if not t and ptr=='' then output('*') end
  output('((',self.mod,self.type)
  if not t then
   output('*')
  end
  output(') ')
  local def = 0
  if self.def ~= '' then def = self.def end
  if t then
   output('tolua_get'..t,'(tolua_S,',narg,',',def,'));')
  else
   output('tolua_getusertype(tolua_S,',narg,',',def,'));')
  end
 end
end

-- Get parameter value
function classDeclaration:getarray (narg)
 if self.dim ~= '' then
  output('  {')
  local def = self.def~='' or 0
  output('   if (!tolua_arrayistype(tolua_S,',narg,',',self.tag,',',self.dim,',',def,'))')
  output('    goto tolua_lerror;')
  output('   else\n')
  output('   {')
  output('    int i;')
  output('    for(i=0; i<'..self.dim..';i++)')
  local t = isbasic(self.type)
  local ptr = ''
  if self.ptr~='' then ptr = '*' end
  output('   ',self.name..'[i] = ')
  if not t and ptr=='' then output('*') end
  output('((',self.mod,self.type)
  if not t then
   output('*')
  end
  output(') ')
  local def = 0
  if self.def ~= '' then def = self.def end
  if t then
   output('tolua_getfield'..t..'(tolua_S,',narg,',i+1,',def,'));')
  else 
   output('tolua_getfieldusertype(tolua_S,',narg,',i+1,',def,'));')
  end
  output('   }')
  output('  }')
 end
end

-- Get parameter value
function classDeclaration:setarray (narg)
 if self.dim ~= '' then
  output('  {')
  output('   int i;')
  output('   for(i=0; i<'..self.dim..';i++)')
  local t,ct = isbasic(self.type)
  if t then
   output('   tolua_pushfield'..t..'(tolua_S,',narg,',i+1,(',ct,')',self.name,'[i]);')
  else
   if self.ptr == '' then
     output('   {')
     output('#ifdef __cplusplus\n')
     output('    void* toluaI_clone = new',self.type,'(',self.name,'[i]);')
     output('#else\n')
     output('    void* toluaI_clone = tolua_copy(tolua_S,(void*)&',self.name,'[i],sizeof(',self.type,'));')
     output('#endif\n')
     output('    tolua_pushfieldusertype(tolua_S,',narg,',i+1,tolua_doclone(tolua_S,toluaI_clone,',self.tag,'),',self.tag,');')
     output('   }')

    --output('   tolua_pushfieldclone(tolua_S,',narg,',i+1,(void*)&',self.name,'[i],sizeof(',self.type,'),',self.tag,');')
   else
    output('   tolua_pushfieldusertype(tolua_S,',narg,',i+1,(void*)',self.name,'[i],',self.tag,');')
   end
  end
  output('  }')
 end
end

-- Free dynamically allocated array
function classDeclaration:freearray ()
 if self.dim ~= '' and tonumber(self.dim)==nil then
  output('  free(',self.name,');')
 end
end

-- Pass parameter
function classDeclaration:passpar ()
 if self.ptr=='&' then
  output('*'..self.name)
 elseif self.ret=='*' then
  output('&'..self.name)
 else
  output(self.name)
 end
end

-- Return parameter value
function classDeclaration:retvalue ()
 if self.ret ~= '' then
  local t,ct = isbasic(self.type)
  if t then
   output('   tolua_push'..t..'(tolua_S,(',ct,')'..self.name..');')
  else
   output('   tolua_pushusertype(tolua_S,(void*)'..self.name..',',self.tag,');')
  end
  return 1
 end
 return 0
end

-- Internal constructor
function _Declaration (t)
 if t.name and t.name~='' then
  local n = split(t.name,'@')
  t.name = n[1]
  t.lname = gsub(n[2] or n[1],"%[.-%]","")
 end
 t._base = classDeclaration
 settag(t,tolua_tag)
 t:checkname()
 t:checktype()
 return t
end

-- Constructor
-- Expects the string declaration.
-- The kind of declaration can be "var" or "func".
function Declaration (s,kind)
 -- eliminate spaces if default value is provided
 s = gsub(s,"%s*=%s*","=")

 if kind == "var" then
  -- check the form: void
  if s == '' or s == 'void' then
   return _Declaration{type = 'void', kind = kind}
  end
 end

 -- check the form: mod type*& name
 local t = split(s,'%*%s*&')
 if t.n == 2 then
  if kind == 'func' then
   error("#invalid function return type: "..s)
  end
  local m = split(t[1],'%s%s*')
  return _Declaration{
   name = t[2],
   ptr = '*',
   ret = '&',
   type = m[m.n],
   mod = concat(m,1,m.n-1),
   kind = kind
  }
 end

 -- check the form: mod type** name
 t = split(s,'%*%s*%*')
 if t.n == 2 then
  if kind == 'func' then
   error("#invalid function return type: "..s)
  end
  local m = split(t[1],'%s%s*')
  return _Declaration{
   name = t[2],
   ptr = '*',
   ret = '*',
   type = m[m.n],
   mod = concat(m,1,m.n-1),
   kind = kind
  }
 end
 
 -- check the form: mod type& name
 t = split(s,'&')
 if t.n == 2 then
  local m = split(t[1],'%s%s*')
  return _Declaration{
   name = t[2],
   ptr = '&',
   type = m[m.n],
   mod = concat(m,1,m.n-1)   ,
   kind = kind
  }
 end
  
 -- check the form: mod type* name
 local s1 = gsub(s,"(%b\[\])",function (n) return gsub(n,'%*','\1') end)
 t = split(s1,'%*')
 if t.n == 2 then
  t[2] = gsub(t[2],'\1','%*') -- restore * in dimension expression
  local m = split(t[1],'%s%s*')
  return _Declaration{
   name = t[2],
   ptr = '*',
   type = m[m.n],
   mod = concat(m,1,m.n-1)   ,
   kind = kind
  }
 end

 if kind == 'var' then
  -- check the form: mod type name
  t = split(s,'%s%s*')
  local v
  if istype(t[t.n]) then v = '' else v = t[t.n]; t.n = t.n-1 end
  return _Declaration{
   name = v,
   type = t[t.n],
   mod = concat(t,1,t.n-1),
   kind = kind
  }

 else -- kind == "func"
 
  -- check the form: mod type name
  t = split(s,'%s%s*')
  local v = t[t.n]  -- last word is the function name
  local tp,md
  if t.n>1 then
   tp = t[t.n-1]
   md = concat(t,1,t.n-2)
  end
  return _Declaration{
   name = v,
   type = tp,
   mod = md,
   kind = kind
  }
 end

end



