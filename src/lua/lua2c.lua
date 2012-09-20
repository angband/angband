-- lua2c.lua
-- embed lua code into C source
-- celetecgraf.puc-rio.br
-- dez 2000

function embed (code)

 -- clean Lua code
 local s = clean(code)
 if not s then
  error("parser error in embedded code")  
 end
 
 -- convert to C
 output('\n { /* begin embedded lua code */\n')
 output('  static unsigned char B[] = {\n   ')
 local t={n=0}
 local b = gsub(s,'(.)',function (c) 
                         local e = '' 
                         %t.n=%t.n+1 if %t.n==15 then %t.n=0 e='\n   ' end 
                         return format('%3u,%s',strbyte(c),e) 
                        end
               )
 output(b..strbyte(" "))
 output('\n  };\n')
 output('  lua_dobuffer(tolua_S,(char*)B,sizeof(B),"'..fn..': embedded Lua code");')
 output(' } /* end of embedded lua code */\n\n')
end

