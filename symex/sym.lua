
oper_table = {
   add = {type='infix', associative= true},
   sub = {type='infix', associative= false},
   mul = {type='infix', associative= true},	
   div = {type='infix', associative= false},
   umn = {type='unary', associative= falve}
}

function node(spec)
   setmetatable(spec, SymOperand)
   return spec
end

function sym(a)
   if type(a) == 'number' then
      return node {type='value', value= a}
   end
   return a
end

local function append(a, b)
   for k, v in ipairs(b) do a[#a+1] = v end
end

local function infix_merge(operands, b, oper)
   if b.type == 'infix' and b.oper == oper then
      append(operands, b.operands)
   else
      operands[#operands + 1] = b
   end
end

function infix_node (a, b, oper)
   local r = node {type='infix', oper= oper}
   a, b = sym(a), sym(b)
   if oper_table[oper].associative then
      r.operands = {}
      infix_merge(r.operands, a, oper)
      infix_merge(r.operands, b, oper)
   else
      r.operands = {a, b}
   end
   return r
end

SymOperand = {
   __add= function(a,b) return infix_node (a, b, 'add') end,
   __sub= function(a,b) return infix_node (a, b, 'sub') end,
   __mul= function(a,b) return infix_node (a, b, 'mul') end,
   __div= function(a,b) return infix_node (a, b, 'div') end,
   __unm= function(a) return node {type='unary', oper= 'minus', value= a} end
}

function value(a)
  local n = {type='value', value= a}
  setmetatable (n, SymOperand)
  return n
end

function infix_dump(n)
   local s = '(' .. n.oper
   for i, v in ipairs(n.operands) do
     s = s .. ' ' .. dump(v)
   end
   return s .. ')'
end

function value_dump(n)
   return n.value
end

dump_table = {
  infix= infix_dump,
  unary= infix_dump,
  value= value_dump
}

function dump(e)
  local f = dump_table[e.type]
  return f(e)
end
