local ffi = require 'ffi'

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
function defs.complex(s)
   return ffi.new('double complex', 0, tonumber(s))
end
function defs.uint64(s)
   return ffi.new('uint64_t', tonumber(s))
end
function defs.int64(s)
   return ffi.new('int64_t', tonumber(s))
end
function defs.quote(s)
   return string.format("%q", s)
end
local escape_lookup = {
   ["a"] = "\a",
   ["b"] = "\b",
   ["f"] = "\f",
   ["n"] = "\n",
   ["r"] = "\r",
   ["t"] = "\t",
   ["v"] = "\v",
   ["0"] = "\0",
   ['"'] = '"',
   ["'"] = "'",
   ["\\"]= "\\"
}
function defs.escape(s)
   local t = string.sub(s, 2)
   local n = tonumber(t)
   if n then return string.char(n) end
   if escape_lookup[t] then return escape_lookup[t] end
   if t == "\n" then return "\n" end
   error("invalid escape sequence : \\" .. t)
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

function defs.literal(val)
   return { type = "Literal", value = val }
end
function defs.literalNumber(s)
   return { type = "Literal", value = tonumber(s) }
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
function defs.varargExpr(name)
   return { type = "Ellipses" }
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
function defs.funcDecl(localkw, name, head, body)
   assert(body.type == "BlockStatement")
   local decl = { type = "FunctionDeclaration", id = name, body = body }
   local params, vararg = { }, false
   for i=1, #head do
      local p = head[i]
      if p.vararg then
         vararg = true
      else
         params[#params + 1] = p.name
      end 
   end
   decl.params   = params
   decl.vararg   = vararg
   decl.is_local = (localkw ~= "")
   return decl
end
function defs.funcExpr(head, body)
   local decl = defs.funcDecl("", nil, head, body)
   decl.expression = true
   return decl
end
function defs.matrixLiteral(body)
   local n = #body
   local m = #body[1]
   for i = 2, n do
      if #body[i] ~= m then error('columns number mismatch') end
   end
   return { type = "MatrixLiteral", body = body}
end
function defs.lambdaFuncExpr(head, expr)
   local body = defs.blockStmt{ defs.returnStmt{ expr } }
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
function defs.matchStmt(disc, cases, default)
   if default then
      cases[#cases + 1] = defs.matchWhen(nil, default)
   end
   return { type = "MatchStatement", discriminant = disc, cases = cases }
end
function defs.matchWhen(test, cons)
   return { type = "MatchWhen", test = test, consequent = cons }
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
local function fix_range_expr(r)
   if r.start == "" then r.start = nil end
   if r.stop  == "" then r.stop  = nil end
   if not r.start and not r.stop then r.range = '~' end
end
function defs.postfixExpr(expr)
   local base = expr[1]
   for i=2, #expr do
      local term = expr[i]
      if term[1] == "(" then
         base = defs.callExpr(base, term[2])
      elseif term[1] == "." or term[1] == ":" then
         base = defs.memberExpr(base, term[2], false)
         base.namespace = (term[1] == ".")
      else
         local index_nb = #term - 1
         fix_range_expr(term[2])
         if index_nb == 1 then
            if term[2].range == "~" then error('range not allowed with single index') end
            base = defs.memberExpr(base, term[2].start, true)
            base.namespace = false
         else
            fix_range_expr(term[3])
            local row, col = term[2], term[3]
            if row.range ~= "~" and col.range ~= "~" then
               base = defs.matrixMemberExpr(base, row.start, col.start)
            else
               base = defs.matrixRangeExpr(base, row, col)
            end
         end
      end
   end
   return base
end
function defs.memberExpr(b, e, c)
   return { type = "MemberExpression", object = b, property = e, computed = c }
end
function defs.matrixMemberExpr(b, row, col)
   return { type = "MatrixMemberExpression", object = b, row = row, column = col }
end
function defs.matrixRangeExpr(b, row, col)
   return { type = "MatrixRangeExpression", object = b, row = row, column = col }
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
function defs.contentAssign(pos, lhs, rhs)
   return { type = "ContentAssignment", left = lhs, right = rhs, pos = pos }
end
function defs.localDecl(lhs, rhs)
   return { type = "VariableDeclaration", names = lhs, inits = rhs }
end
function defs.doStmt(block)
   return { type = "DoStatement", body = block }
end

local op_info = {
   ["or"]  = { 1, 'L' },
   ["and"] = { 2, 'L' },
   [".."]  = { 3, 'L' },

   ["=="]  = { 4, 'L' },
   ["~="]  = { 4, 'L' },

   ["in"]  = { 5, 'L' },

   [">="]  = { 6, 'L' },
   ["<="]  = { 6, 'L' },
   [">"]   = { 6, 'L' },
   ["<"]   = { 6, 'L' },

   ["-"]   = { 7, 'L' },
   ["+"]   = { 7, 'L' },

   ["*"]   = { 8, 'L' },
   ["/"]   = { 8, 'L' },
   ["%"]   = { 8, 'L' },

   ["^"]   = { 9, 'R' },
}

local shift = table.remove

local function fold_infix(exp, lhs, min)
   while op_info[exp[1]] ~= nil and op_info[exp[1]][1] >= min do
      local op   = shift(exp, 1)
      local rhs  = shift(exp, 1)
      while op_info[exp[1]] ~= nil do
         local info = op_info[exp[1]]
         local prec, assoc = info[1], info[2]
         if prec > op_info[op][1] or (assoc == 'R' and prec == op_info[op][1]) then
            rhs = fold_infix(exp, rhs, prec)
         else
            break
         end
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
