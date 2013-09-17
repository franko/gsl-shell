local defs = { }
local util = require('nyanga.util')

defs.tonumber = function(s)
   local n = string.gsub(s, '_', '')
   return tonumber(n)
end
defs.tostring = tostring

function defs.octal(s)
   return tostring(tonumber(s, 8))
end
function defs.quote(s)
   return string.format("%q", s)
end
function defs.chunk(body)
   return { type = "Chunk", body = body }
end
function defs.stmt(pos, node)
   node.pos = pos
   return node
end
function defs.error(src, pos)
   local loc = string.sub(src, pos, pos)
   if loc == '' then
      error("Unexpected end of input")
   else
      local tok = string.match(src, '(%w+)', pos) or loc
      local line = 0
      local ofs  = 0
      while ofs < pos do
         local a, b = string.find(src, "\n", ofs)
         if a then
            ofs = a + 1
            line = line + 1
         else
            break
         end
      end
      error("Unexpected token '"..tok.."' on line "..tostring(line))
   end
end
function defs.fail(src, pos, msg)
   local loc = string.sub(src, pos, pos)
   if loc == '' then
      error("Unexpected end of input")
   else
      local tok = string.match(src, '(%w+)', pos) or loc
      error(msg.." near '"..tok.."'")
   end
end

local strEscape = {
   ["\\r"] = "\r",
   ["\\n"] = "\n",
   ["\\t"] = "\t",
   ["\\\\"] = "\\",
}
function defs.string(str)
   return string.gsub(str, "(\\[rnt\\])", strEscape)
end
function defs.literal(val)
   return { type = "Literal", value = val }
end
function defs.boolean(val)
   return val == 'true'
end
function defs.nilExpr()
   return { type = "Literal", value = nil }
end
function defs.identifier(name)
   return { type = "Identifier", name = name }
end

function defs.tableExpr(members)
   return { type = "TableExpression", members = members }
end
function defs.ifStmt(test, cons, altn)
   if cons.type ~= "BlockStatement" then
      cons = defs.blockStmt{ cons }
   end
   if altn and altn.type ~= "BlockStatement" then
      altn = defs.blockStmt{ altn }
   end
   return { type = "IfStatement", test = test, consequent = cons, alternate = altn }
end
function defs.whileStmt(test, body)
   return { type = "WhileStatement", test = test, body = body }
end
function defs.repeatStmt(body, test)
   return { type = "RepeatStatement", test = test, body = body }
end
function defs.forStmt(name, init, last, step, body)
   return {
      type = "ForStatement",
      name = name, init = init, last = last, step = step,
      body = body
   }
end
function defs.forInStmt(left, right, body)
   return { type = "ForInStatement", left = left, right = right, body = body }
end
function defs.spreadExpr(arg)
   return { type = "SpreadExpression", argument = arg }
end
function defs.funcDecl(localkw, name, head, body)
   if body.type ~= "BlockStatement" then
      body = defs.blockStmt{ defs.returnStmt{ body } }
   end
   local decl = { type = "FunctionDeclaration", id = name, body = body }
   local defaults, params, rest = { }, { }, nil
   for i=1, #head do
      local p = head[i]
      if p.rest then
         rest = p.name
      else
         params[#params + 1] = p.name
         if p.default then
            defaults[i] = p.default
         end
      end 
   end
   decl.params   = params
   decl.defaults = defaults
   decl.rest     = rest
   decl.is_local = (localkw ~= "")
   return decl
end
function defs.funcExpr(head, body)
   local decl = defs.funcDecl("", nil, head, body)
   decl.expression = true
   return decl
end
function defs.blockStmt(body)
   return {
      type = "BlockStatement",
      body = body
   }
end
function defs.returnStmt(args)
   return { type = "ReturnStatement", arguments = args }
end
function defs.breakStmt()
   return { type = "BreakStatement" }
end
function defs.exprStmt(pos, expr)
   return { type = "ExpressionStatement", expression = expr, pos = pos }
end
function defs.selfExpr()
   return { type = "SelfExpression" }
end
function defs.prefixExpr(o, a)
   return { type = "UnaryExpression", operator = o, argument = a }
end
function defs.postfixExpr(expr)
   local base = expr[1]
   for i=2, #expr do
      if expr[i][1] == "(" then
         base = defs.callExpr(base, expr[i][2])
      else
         base = defs.memberExpr(base, expr[i][2], expr[i][1] == "[")
         base.namespace = (expr[i][1] == ".")
      end
   end
   return base
end
function defs.memberExpr(b, e, c)
   return { type = "MemberExpression", object = b, property = e, computed = c }
end
function defs.callExpr(expr, args)
   return { type = "CallExpression", callee = expr, arguments = args } 
end
function defs.newExpr(expr, args)
   return { type = "NewExpression", callee = expr, arguments = args } 
end

function defs.binaryExpr(op, lhs, rhs)
   return { type = "BinaryExpression", operator = op, left = lhs, right = rhs }
end
function defs.logicalExpr(op, lhs, rhs)
   return { type = "LogicalExpression", operator = op, left = lhs, right = rhs }
end
function defs.assignExpr(lhs, rhs)
   return { type = "AssignmentExpression", left = lhs, right = rhs }
end
function defs.localDecl(lhs, rhs)
   return { type = "VariableDeclaration", names = lhs, inits = rhs }
end
function defs.doStmt(block)
   return { type = "DoStatement", body = block }
end

local prec = {
   ["or"]  = 1,
   ["and"] = 2,
   [".."]  = 3,

   ["=="]  = 4,
   ["~="]  = 4,

   ["in"]  = 5,

   [">="]  = 6,
   ["<="]  = 6,
   [">"]   = 6,
   ["<"]   = 6,

   ["-"]   = 7,
   ["+"]   = 7,

   ["*"]   = 8,
   ["/"]   = 8,
   ["%"]   = 8,

   ["^"]   = 9,
}

local shift = table.remove

local function fold_infix(exp, lhs, min)
   while prec[exp[1]] ~= nil and prec[exp[1]] >= min do
      local op  = shift(exp, 1)
      local rhs = shift(exp, 1)
      while prec[exp[1]] ~= nil and prec[exp[1]] > prec[op] do
         rhs = fold_infix(exp, rhs, prec[exp[1]])
      end
      if op == "or" or op == "and" then
         lhs = defs.logicalExpr(op, lhs, rhs)
      else
         lhs = defs.binaryExpr(op, lhs, rhs)
      end
   end
   return lhs
end

function defs.infixExpr(exp)
   return fold_infix(exp, shift(exp, 1), 0)
end

return defs
