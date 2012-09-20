-- tolua: function class
-- Written by Waldemar Celes
-- TeCGraf/PUC-Rio
-- Jul 1998
-- $Id: function.lua,v 1.1 2001/10/27 19:35:29 angband Exp $

-- This code is free software; you can redistribute it and/or modify it.
-- The software provided hereunder is on an "as is" basis, and
-- the author has no obligation to provide maintenance, support, updates,
-- enhancements, or modifications. 



-- Function class
-- Represents a function or a class method.
-- The following fields are stored:
--  mod  = type modifiers
--  type = type
--  ptr  = "*" or "&", if representing a pointer or a reference
--  name = name
--  args  = list of argument declarations
--  const = if it is a method receiving a const "this".
classFunction = {
 mod = '',
 type = '',
 ptr = '',
 name = '',
 args = {n=0},
 const = '',
 _base = classFeature,
}
settag(classFunction,tolua_tag)

-- declare tags
function classFunction:decltag ()
 self.itype,self.tag = tagvar(self.type,strfind(self.mod,'const'))
 local i=1
 while self.args[i] do
  self.args[i]:decltag()
  i = i+1
 end
end


-- Write binding function
-- Outputs C/C++ binding function.
function classFunction:supcode ()
 local nret = 0      -- number of returned values
 local class = self:inclass()
 local _,_,static = strfind(self.mod,'^%s*(static)')

 if class then
  output("/* method:",self.name," of class ",class," */")
 else
  output("/* function:",self.name," */")
 end
 output("static int",self.cname,"(lua_State* tolua_S)")
 output("{")

 -- check types
 output(' if (\n')
 -- check self
 local narg
 if class then narg=2 else narg=1 end
 if class and self.name~='new' and static==nil then
  if self.const == 'const' then
   output('     !tolua_istype(tolua_S,1,',self.parent.ctag,',0) ||\n') 
  else
   output('     !tolua_istype(tolua_S,1,',self.parent.tag,',0) ||\n') 
  end
 end
 -- check args
 if self.args[1].type ~= 'void' then
  local i=1
  while self.args[i] do
   if isbasic(self.args[i].type) ~= 'value' then
    output('     !'..self.args[i]:outchecktype(narg)..' ||\n')
   end
   narg = narg+1
   i = i+1
  end
 end
 -- check end of list 
 output('     !tolua_isnoobj(tolua_S,'..narg..')\n )\n  goto tolua_lerror;')

 output(' else\n {')
 
 -- declare self, if the case
 local narg
 if class then narg=2 else narg=1 end
 if class and self.name~='new' and static==nil then
  output(' ',self.const,class,'*','self = ')
  output('(',self.const,class,'*) ')
  output('tolua_getusertype(tolua_S,1,0);')
 elseif static then
  _,_,self.mod = strfind(self.mod,'^%s*static%s%s*(.*)')
 end
 -- declare parameters
 if self.args[1].type ~= 'void' then
  local i=1
  while self.args[i] do
   self.args[i]:declare(narg)
   narg = narg+1
   i = i+1
  end
 end

 -- check self
 if class and self.name~='new' and static==nil then 
  output('  if (!self) tolua_error(tolua_S,"invalid \'self\' in function \''..self.name..'\'");');
 end

 -- get array element values
 if class then narg=2 else narg=1 end
 if self.args[1].type ~= 'void' then
  local i=1
  while self.args[i] do
   self.args[i]:getarray(narg)
   narg = narg+1
   i = i+1
  end
 end

 -- call function
 if class and self.name=='delete' then
  output('  delete self;')
 elseif class and self.name == 'operator&[]' then
  output('  self->operator[](',self.args[1].name,') = ',self.args[2].name,';')
 else
  output('  {')
  if self.type ~= '' and self.type ~= 'void' then
   output('  ',self.mod,self.type,self.ptr,'toluaI_ret = ')
   output('(',self.mod,self.type,self.ptr,') ')
  else
   output('  ')
  end
  if class and self.name=='new' then
   output('new',class,'(')
  elseif class and static then
   output(class..'::'..self.name,'(')
  elseif class then
   output('self->'..self.name,'(')
  else
   output(self.name,'(')
  end

  -- write parameters
  local i=1
  while self.args[i] do
   self.args[i]:passpar()
   i = i+1
   if self.args[i] then
    output(',')
   end
  end
     
  output(');')

  -- return values
  if self.type ~= '' and self.type ~= 'void' then
   nret = nret + 1
   local t,ct = isbasic(self.type)
   if t then
    output('   tolua_push'..t..'(tolua_S,(',ct,')toluaI_ret);')
   else
    if self.ptr == '' then
     output('   {')
     output('#ifdef __cplusplus\n')
     output('    void* toluaI_clone = new',self.type,'(toluaI_ret);') 
     output('#else\n')
     output('    void* toluaI_clone = tolua_copy(tolua_S,(void*)&toluaI_ret,sizeof(',self.type,'));')
     output('#endif\n')
     output('    tolua_pushusertype(tolua_S,tolua_doclone(tolua_S,toluaI_clone,',self.tag,'),',self.tag,');')
     output('   }')
     --output('   tolua_pushclone((void*)&toluaI_ret,sizeof(',self.type,'),',self.tag,');')
    elseif self.ptr == '&' then
     output('   tolua_pushusertype(tolua_S,(void*)&toluaI_ret,',self.tag,');')
    else
     output('   tolua_pushusertype(tolua_S,(void*)toluaI_ret,',self.tag,');')
    end
   end
  end
  local i=1
  while self.args[i] do
   nret = nret + self.args[i]:retvalue()
   i = i+1
  end
  output('  }')

  -- set array element values
  if class then narg=2 else narg=1 end
  if self.args[1].type ~= 'void' then
   local i=1
   while self.args[i] do
    self.args[i]:setarray(narg)
    narg = narg+1
    i = i+1
   end
  end
 
  -- free dynamically allocated array
  if self.args[1].type ~= 'void' then
   local i=1
   while self.args[i] do
    self.args[i]:freearray()
    i = i+1
   end
  end
 end

 output(' }')
 output(' return '..nret..';')

 -- call overloaded function or generate error
 output('tolua_lerror:\n')
 local overload = strsub(self.cname,-2,-1) - 1
 if overload >= 0 then
  output(' return '..strsub(self.cname,1,-3)..format("%02d",overload)..'(tolua_S);')
 else
  output(' tolua_error(tolua_S,"#ferror in function \''..self.lname..'\'.");')
  output(' return 0;')
 end

 output('}')
 output('\n')
end

-- register function
function classFunction:register ()
 local parent = self:inclass() or self:inmodule()
 if parent then
  output(' tolua_function(tolua_S,"'..parent..'","'..self.lname..'",'..self.cname..');')
 else
  output(' tolua_function(tolua_S,NULL,"'..self.lname..'",'..self.cname..');')
 end
end

-- unregister function
function classFunction:unregister ()
 if self:inclass()==nil and self:inmodule()==nil then
  output(' lua_pushnil(tolua_S); lua_setglobal(tolua_S,"'..self.lname..'");')
 end
end


-- Print method
function classFunction:print (ident,close)
 print(ident.."Function{")
 print(ident.." mod  = '"..self.mod.."',")
 print(ident.." type = '"..self.type.."',")
 print(ident.." ptr  = '"..self.ptr.."',")
 print(ident.." name = '"..self.name.."',")
 print(ident.." const = '"..self.const.."',")
 print(ident.." cname = '"..self.cname.."',")
 print(ident.." lname = '"..self.lname.."',")
 print(ident.." args = {")
 local i=1
 while self.args[i] do
  self.args[i]:print(ident.."  ",",")
  i = i+1
 end
 print(ident.." }")
 print(ident.."}"..close)
end

-- determine lua function name overload
function classFunction:overload ()
 return self.parent:overload(self.lname)
end



-- Internal constructor
function _Function (t)
 t._base = classFunction
 settag(t,tolua_tag)

 if t.const ~= 'const' and t.const ~= '' then
  error("#invalid 'const' specification")
 end

 append(t)
 if t:inclass() then
  if t.name == t.parent.name then
   t.name = 'new'
   t.lname = 'new'
   t.type = t.parent.name
   t.ptr = '*'
  elseif t.name == '~'..t.parent.name then
   t.name = 'delete'
   t.lname = 'delete'
  end
 end
 t.cname = t:cfuncname("toluaI")..t:overload(t)
 return t
end

-- Constructor
-- Expects three strings: one representing the function declaration,
-- another representing the argument list, and the third representing
-- the "const" or empty string.
function Function (d,a,c)
 local t = split(strsub(a,2,-2),',') -- eliminate braces
 local i=1
 local l = {n=0}
 while t[i] do
  l.n = l.n+1
  l[l.n] = Declaration(t[i],'var')
  i = i+1
 end
 local f = Declaration(d,'func')
 f.args = l
 f.const = c
 return _Function(f)
end


