local B = require('nyanga.builder')
local util = require('nyanga.util')

local Scope = { }
Scope.__index = Scope
function Scope.new(outer)
   local self = {
      outer = outer;
      entries = { };
   }
   return setmetatable(self, Scope)
end
function Scope:define(name, info)
   self.entries[name] = info
end
function Scope:lookup(name)
   if self.entries[name] then
      return self.entries[name]
   elseif self.outer then
      return self.outer:lookup(name)
   else
      return nil
   end
end

local Context = { }
Context.__index = Context
function Context.new()
   local self = {
      scope = Scope.new(),
      gen_index = {}
   }
   return setmetatable(self, Context)
end
function Context:enter()
   self.scope = Scope.new(self.scope)
end
function Context:leave()
   self.scope = self.scope.outer
end
function Context:define(name, info)
   info = info or { }
   self.scope:define(name, info)
   return info
end
function Context:lookup(name)
   local info = self.scope:lookup(name)
   return info
end
local function gen_index_incr(index)
   local ascii_a = string.byte('a')
   local ascii_z = string.byte('z')
   for k = #index, 1, -1 do
      if index[k] < ascii_z - ascii_a - 1 then
         index[k] = index[k] + 1
         return
      else
         index[k] = 0
      end
   end
   index[#index + 1] = 0
end
function Context:generateVar(main_ctx)
   local ascii_a = string.byte('a')
   while true do
      gen_index_incr(self.gen_index)
      local var_name = "_"
      for k = 1, #self.gen_index do
         var_name = var_name .. string.char(ascii_a + self.gen_index[k])
      end
      if not main_ctx:lookup(var_name) then
         return B.identifier(var_name)
      end
   end
end

local builtin = {
   transpose = {
      name = B.identifier("__transpose__"),
      value = B.memberExpression(B.identifier("matrix"), B.identifier("transpose"), false),
   },
   matrix = {
      name = B.identifier("__matrix__"),
      value = B.memberExpression(B.identifier("matrix"), B.identifier("def"), false),
   },
   slice = {
      name = B.identifier("__slice__"),
      value = B.memberExpression(B.identifier("matrix"), B.identifier("slice"), false),
   },
}

local match = { }

local function pop_context_vars(ctx)
   local decls = { }
   local entries = ctx.scope.entries
   for id, info in pairs(entries) do
      local decl = B.localDeclaration({ id }, { info })
      decls[#decls+1] = decl
   end
   return decls
end
local function stmt_decl_block(stmt, decls)
   if #decls > 0 then
      decls[#decls + 1] = stmt
      return B.blockStatement(decls)
   else
      return stmt
   end
end
local function stmt_decl_append(stmts, stmt, decls)
   for i = 1, #decls do
      stmts[#stmts+1] = decls[i]
   end
   stmts[#stmts+1] = stmt
end
local function expr_context_stmt_list(self, body)
   local stmts = { }
   for i = 1, #body do
      self.genctx:enter()
      local stmt = self:get(body[i])
      local decls = pop_context_vars(self.genctx)
      self.genctx:leave()
      if i < #body then
         stmts[i] = stmt_decl_block(stmt, decls)
      else
         stmt_decl_append(stmts, stmt, decls)
      end
   end
   return stmts
end

function match:Chunk(node)
   self.scope = expr_context_stmt_list(self, node.body)
   for prop in pairs(self.use) do
      local item = builtin[prop]
      local decl = B.localDeclaration({ item.name }, { item.value })
     table.insert(self.scope, 1, decl)
   end
   return B.chunk(self.scope)
end
function match:Literal(node)
   return B.literal(node.value)
end
function match:Identifier(node)
   return B.identifier(node.name)
end
function match:Ellipses(node)
   return B.vararg()
end
function match:VariableDeclaration(node)
   local inits = node.inits and self:list(node.inits) or { }
   for i=1, #node.names do
      local n = node.names[i]
      if n.type == 'Identifier' and not self.ctx:lookup(n.name) then
         self.ctx:define(n.name)
      end
   end
   return B.localDeclaration(self:list(node.names), inits)
end
function match:AssignmentExpression(node)
   return B.assignmentExpression(
      self:list(node.left), self:list(node.right)
   )
end

local function matrixElementRef(self, obj, row, col)
   local comp_obj = B.memberExpression(obj, B.identifier("data"), false)
   local tda = B.memberExpression(obj, B.identifier("tda"), false)
   local iminus = B.binaryExpression("-", self:get(row), B.literal(1))
   local tda_i = B.binaryExpression("*", tda, iminus)
   local jminus = B.binaryExpression("-", self:get(col), B.literal(1))
   local comp_prop = B.binaryExpression("+", tda_i, jminus)
   return B.memberExpression(comp_obj, comp_prop, true)
end

function match:MatrixMemberExpression(node)
   if node.object.type ~= "Identifier" then
      local temp = self.genctx:generateVar(self.ctx)
      self.genctx:define(temp, self:get(node.object))
      return matrixElementRef(self, temp, node.row, node.column)
   else
      return matrixElementRef(self, self:get(node.object), node.row, node.column)
   end
end
local function get_range(self, t, stop_expr)
   local start = t.start and self:get(t.start) or B.literal(1)
   local stop  = t.stop  and self:get(t.stop)  or stop_expr
   return start, stop
end
function match:MatrixRangeExpression(node)
   local need_hoist = (node.object.type ~= "Identifier") and (not node.row.stop or not node.column.stop)
   local object
   if need_hoist then
      object = self.genctx:generateVar(self.ctx)
      self.genctx:define(object, self:get(node.object))
   else
      object = self:get(node.object)
   end
   self.use.slice = true
   local slice = builtin.slice.name
   local rs, re = get_range(self, node.row, B.memberExpression(object, B.identifier('size1'), false))
   local cs, ce = get_range(self, node.column, B.memberExpression(object, B.identifier('size2'), false))
   return B.callExpression(slice, { object, rs, re, cs, ce })
end
function match:MemberExpression(node)
   return B.memberExpression(
      self:get(node.object), self:get(node.property), node.computed
   )
end
function match:SelfExpression(node)
   return B.identifier('self')
end

function match:ReturnStatement(node)
   return B.returnStatement(self:list(node.arguments))
end

function match:IfStatement(node)
   local test, cons, altn = self:get(node.test)
   if node.consequent then
      cons = self:get(node.consequent)
   end
   if node.alternate then
      altn = self:get(node.alternate)
   end
   local stmt = B.ifStatement(test, cons, altn)
   return stmt
end

function match:BreakStatement(node)
   return B.breakStatement()
end

function match:LogicalExpression(node)
   return B.logicalExpression(
      node.operator, self:get(node.left), self:get(node.right)
   )
end

function match:BinaryExpression(node)
   local o = node.operator
   return B.binaryExpression(o, self:get(node.left), self:get(node.right))
end
function match:UnaryExpression(node)
   if node.operator == "`" then
      self.use.transpose = true
      local transpose = builtin.transpose.name
      return B.callExpression(transpose, { self:get(node.argument) })
   end
   return B.unaryExpression(node.operator, self:get(node.argument))
end
function match:MatrixLiteral(node)
   local n = #node.body
   local body = {}
   for i = 1, n do
      body[i] = B.table(self:list(node.body[i]))
   end
   self.use.matrix = true
   local matrix = builtin.matrix.name
   return B.callExpression(matrix, { B.table(body) })
end
function match:FunctionDeclaration(node)
   local name
   if not node.expression then
      name = self:get(node.id[1])
   end

   local params  = { }
   local vararg = node.vararg

   self.ctx:enter()

   for i=1, #node.params do
      local name = self:get(node.params[i])
      self.ctx:define(name.name)
      params[#params + 1] = name
   end

   if vararg then
      params[#params + 1] = B.vararg()
   end

   local body = self:get(node.body)
   local func = B.functionExpression(params, body, vararg)

   self.ctx:leave()

   if node.expression then
      return func
   end
   if node.is_local then
      return B.localDeclaration({ name }, { func })
   else
      return B.assignmentExpression({ name }, { func })
   end
end

function match:NilExpression(node)
   return B.literal(nil)
end
function match:PropertyDefinition(node)
   node.value.generator = node.generator
   return self:get(node.value)
end
function match:BlockStatement(node)
   return B.blockStatement(expr_context_stmt_list(self, node.body))
end
function match:ExpressionStatement(node)
   return B.expressionStatement(self:get(node.expression))
end
function match:CallExpression(node)
   local callee = node.callee
   if callee.type == 'MemberExpression' and not callee.computed then
      if callee.namespace then
         return B.callExpression(self:get(callee), self:list(node.arguments))
      else
         local recv = self:get(callee.object)
         local prop = self:get(callee.property)
         return B.sendExpression(recv, prop, self:list(node.arguments))
      end
   else
      local args = self:list(node.arguments)
      return B.callExpression(self:get(callee), args)
   end
end
function match:NewExpression(node)
   return B.callExpression(B.identifier('new'), {
      self:get(node.callee), unpack(self:list(node.arguments))
   })
end
function match:WhileStatement(node)
   local loop = B.identifier(util.genid())
   local save = self.loop
   self.loop = loop
   local body = self:get(node.body)
   self.loop = save
   return B.whileStatement(self:get(node.test), body)
end
function match:ForStatement(node)
   local loop = B.identifier(util.genid())
   local save = self.loop
   self.loop = loop

   local name = self:get(node.name)
   local init = self:get(node.init)
   local last = self:get(node.last)
   local step = B.literal(node.step)
   local body = self:get(node.body)
   self.loop = save

   return B.forStatement(B.forInit(name, init), last, step, body)
end
function match:ForInStatement(node)
   local loop = B.identifier(util.genid())
   local save = self.loop
   self.loop = loop

   local none = B.tempnam()
   local temp = B.tempnam()
   local iter = self:get(node.right)

   local left = { }
   for i=1, #node.left do
      left[i] = self:get(node.left[i])
   end

   local body = self:get(node.body);
   self.loop = save

   return B.forInStatement(B.forNames(left), iter, body)
end
function match:TableExpression(node)
   local properties = { }
   local index = 0
   for i=1, #node.members do
      local prop = node.members[i]

      local key, val
      if prop.key then
         if prop.key.type == 'Identifier' then
            key = prop.key.name
         elseif prop.key.type == "Literal" then
            key = prop.key.value
         end
      else
         index = index + 1
         key = index
      end

      properties[key] = self:get(prop.value)
   end

   return B.table(properties)
end

local function countln(src, pos, idx)
   local line = 0
   local index, limit = idx or 1, pos
   while index <= limit do
      local s, e = string.find(src, "\n", index, true)
      if s == nil or e > limit then
         break
      end
      index = e + 1
      line  = line + 1
   end
   return line 
end

local function transform(tree, src)
   local self = { }
   self.line = 1
   self.pos  = 0

   self.use = {}
   self.ctx = Context.new()
   self.genctx = Context.new()

   function self:sync(node)
      local pos = node.pos
      if pos ~= nil and pos > self.pos then
         local prev = self.pos
         local line = countln(src, pos, prev + 1) + self.line
         self.line = line
         self.pos = pos
      end
   end

   function self:get(node, ...)
      if not match[node.type] then
         error("no handler for "..tostring(node.type))
      end
      self:sync(node)
      local out = match[node.type](self, node, ...)
      out.line = self.line
      return out
   end

   function self:list(nodes, ...)
      local list = { }
      for i=1, #nodes do
         list[#list + 1] = self:get(nodes[i], ...)
      end
      return list
   end

   return self:get(tree)
end

return {
   transform = transform
}
