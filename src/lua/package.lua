-- tolua: package class
-- Written by Waldemar Celes
-- TeCGraf/PUC-Rio
-- Jul 1998
-- $Id: package.lua,v 1.2 2001/10/28 16:18:38 angband Exp $

-- This code is free software; you can redistribute it and/or modify it.
-- The software provided hereunder is on an "as is" basis, and
-- the author has no obligation to provide maintenance, support, updates,
-- enhancements, or modifications. 



-- Package class
-- Represents the whole package being bound.
-- The following fields are stored:
--    {i} = list of objects in the package.
classPackage = {
 _base = classContainer,
 type = 'package'
}
settag(classPackage,tolua_tag)

-- Print method
function classPackage:print ()
 print("Package: "..self.name)
 local i=1
 while self[i] do
  self[i]:print("","")
  i = i+1
 end
end

function classPackage:preprocess ()
 self.code = "\n"..self.code    -- add a blank sentinel line
 -- avoid preprocessing verbatim lines
 local V = {}
 self.code = gsub(self.code,"\n(%s*%$[^%[%]][^\n]*)",function (v)
                                               tinsert(%V,v)
                                               return "\n$"..getn(%V).."$" 
                                              end)
 -- avoid preprocessing embedded lua code
 local C = {}
 self.code = gsub(self.code,"\n%s*%$%[","\1") -- deal with embedded Lua code
 self.code = gsub(self.code,"\n%s*%$%]","\2")
 self.code = gsub(self.code,"(%b\1\2)",       function (c)
                                               tinsert(%C,c)
                                               return "\n$["..getn(%C).."]$" 
                                              end)
 -- perform global substitution

 self.code = gsub(self.code,"(//[^\n]*)","")     -- eliminate C++ comments
 self.code = gsub(self.code,"/%*","\1")
 self.code = gsub(self.code,"%*/","\2")
 self.code = gsub(self.code,"%b\1\2","")
 self.code = gsub(self.code,"\1","/%*")
 self.code = gsub(self.code,"\2","%*/")
 self.code = gsub(self.code,"%s*@%s*","@") -- eliminate spaces beside @
 self.code = gsub(self.code,"%s?inline(%s)","%1") -- eliminate 'inline' keyword
 self.code = gsub(self.code,"%s?extern(%s)","%1") -- eliminate 'extern' keyword
 self.code = gsub(self.code,"%s?virtual(%s)","%1") -- eliminate 'virtual' keyword
 self.code = gsub(self.code,"public:","") -- eliminate 'public:' keyword
 self.code = gsub(self.code,"([^%w_])void%s*%*","%1_userdata ") -- substitute 'void*'
 self.code = gsub(self.code,"([^%w_])void%s*%*","%1_userdata ") -- substitute 'void*'
 self.code = gsub(self.code,"([^%w_])char%s*%*","%1_cstring ")  -- substitute 'char*'

 -- restore embedded code
 self.code = gsub(self.code,"%$%[(%d+)%]%$",function (n)
                                             return %C[tonumber(n)]
                                            end)
 -- restore verbatim lines
 self.code = gsub(self.code,"%$(%d+)%$",function (n)
                                         return %V[tonumber(n)]
                                        end)
end

-- translate verbatim
function classPackage:preamble ()
 output('/*\n')
 output('** Lua binding: '..self.name..'\n')
 output('** Generated automatically by '..TOLUA_VERSION..' on '..date()..'.\n')
 output('*/\n\n')

 output('#include "lua/tolua.h"\n\n')

 if not flags.h then
  output('/* Exported function */')
  output('int  tolua_'..self.name..'_open (lua_State* tolua_S);')
  output('void tolua_'..self.name..'_close (lua_State* tolua_S);')
  output('\n')
 end

 local i=1
 while self[i] do
  self[i]:preamble()
  i = i+1
 end
 output('\n')
 output('/* function to register type */')
 output('static void toluaI_reg_types (lua_State* tolua_S)')
 output('{')
 foreach(_usertype,function(n,v) output(' tolua_usertype(tolua_S,"',v,'");') end)
 output('}')
 output('\n')

 output('/* error messages */')
 output('#define TOLUA_ERR_SELF tolua_error(tolua_S,\"invalid \'self\'\")')
 output('#define TOLUA_ERR_ASSIGN tolua_error(tolua_S,\"#vinvalid type in variable assignment.\")')
 output('\n')
end

-- register package
-- write package open function
function classPackage:register ()
 output("/* Open function */")
 output("int tolua_"..self.name.."_open (lua_State* tolua_S)")
 output("{")
 output(" tolua_open(tolua_S);")
 output(" toluaI_reg_types(tolua_S);")
 local i=1
 while self[i] do
  self[i]:register()
  i = i+1
 end
 output(" return 1;")
 output("}")
end

-- unregister package
-- write package close function
function classPackage:unregister ()
 output("/* Close function */")
 output("void tolua_"..self.name.."_close (lua_State* tolua_S)")
 output("{")
 local i=1
 while self[i] do
  self[i]:unregister()
  i = i+1
 end
 output("}")
end

-- write header file
function classPackage:header ()
 output('/*\n') output('** Lua binding: '..self.name..'\n')
 output('** Generated automatically by '..TOLUA_VERSION..' on '..date()..'.\n')
 output('*/\n\n')

 if not flags.h then
  output('/* Exported function */')
  output('int  tolua_'..self.name..'_open (lua_State* tolua_S);')
  output('void tolua_'..self.name..'_close (lua_State* tolua_S);')
  output('\n')
 end
end

-- Internal constructor
function _Package (t)
 t._base = classPackage
 settag(t,tolua_tag)
 return t
end

-- Constructor
-- Expects the base file name.
-- It assumes the file has extension ".pkg".
function Package (name)
 -- read file
 local code = read("*a")
 code = "\n" .. code         -- add sentinel
 -- deal with include directive
 local nsubst
 repeat
  code,nsubst = gsub(code,"\n%s*%$<(.-)>%s*\n",function (fn)
                                                local fp,msg = openfile(fn,'r')
                                                if not fp then
                                                 error('#'..msg..': '..fn)
                                                end
                                                local s = read(fp,'*a')
                                                closefile(fp)
                                                return "\n" .. s
                                               end)
 until nsubst==0

 -- deal with include directive for C/C++ header files
 local nsubst
 repeat
  code,nsubst = 
   gsub(code,"\n%s*%${(.-)}%s*\n",
        function (fn)
         local fp,msg = openfile(fn,'r')
         if not fp then
          error('#'..msg..': '..fn)
         end
         local s = read(fp,'*a')
         closefile(fp)
         -- extract marked code
         local T = {code="\n"}
         s= "\n" .. s .. "\n" -- add blank lines as sentinels
         -- extract one-line statments
         gsub(s,"\n(.-)[Tt][Oo][Ll][Uu][Aa]_[Ee][Xx][Pp][Oo][Rr][Tt][^\n]*\n",
              function (c) %T.code = %T.code .. c .. "\n" end
             )
         -- extract multiline statments
         gsub(s,"\n[^\n]*[Tt][Oo][Ll][Uu][Aa]_[Bb][Ee][Gg][Ii][Nn][^\n]*"..
                "(.-)" ..
                "\n[^\n]*[Tt][Oo][Ll][Uu][Aa]_[Ee][Nn][Dd][^\n]*\n",
              function (c) %T.code = %T.code .. c .. "\n" end
             )
         return T.code
        end)
 until nsubst==0

 local t = _Package(_Container{name=name, code=code})
 push(t)
 t:preprocess()
 t:parse(t.code)
 pop()
 return t
end


