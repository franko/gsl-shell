--
-- luacode-generator.lua
--
-- This file is part of the LuaJIT Language Toolkit.
--
-- Module to generate the Lua code that corresponds to a given Lua AST Tree.
-- Can be used as an alternative to the bytecode generator.

local operator = require("operator")
local matrix_trans = require('matrix-transform')
local syntax = require('syntax')

local build, ident, literal, logical, binop, field, tget = syntax.build, syntax.ident, syntax.literal, syntax.logical, syntax.binop, syntax.field, syntax.tget

local genid = require("util").genid

local StatementRule = { }
local ExpressionRule = { }
local LHSExpressionRule = { }

local concat = table.concat
local format = string.format

local function is_string(node)
    return node.kind == "Literal" and type(node.value) == "string"
end

local function is_const(node, val)
    return node.kind == "Literal" and node.value == val
end

local function comma_sep_list(ls, f)
    local strls
    if f then
        strls = { }
        for k = 1, #ls do strls[k] = f(ls[k]) end
    else
        strls = ls
    end
    return concat(strls, ", ")
end

local function as_parameter(node)
    return node.kind == "Vararg" and "..." or node.name
end

function ExpressionRule:Identifier(node)
    return node.name, operator.ident_priority
end

function ExpressionRule:Literal(node)
    local val = node.value
    local str = type(val) == "string" and format("%q", val) or tostring(val)
    return str, operator.ident_priority
end

function ExpressionRule:MemberExpression(node)
    local object, prio = self:expr_emit(node.object)
    if prio < operator.ident_priority then object = "(" .. object .. ")" end
    local exp
    if node.computed then
        local prop = self:expr_emit(node.property)
        exp = format("%s[%s]", object, prop)
    else
        exp = format("%s.%s", object, node.property.name)
    end
    return exp, operator.ident_priority
end

function ExpressionRule:Vararg()
    return "...", operator.ident_priority
end

function ExpressionRule:BinaryExpression(node)
    local oper = node.operator
    local lprio = operator.left_priority(oper)
    local rprio = operator.right_priority(oper)
    local a, alprio, arprio = self:expr_emit(node.left)
    local b, blprio, brprio = self:expr_emit(node.right)
    if not arprio then arprio = alprio end
    if not brprio then brprio = blprio end
    local ap = arprio <  lprio and format("(%s)", a) or a
    local bp = blprio <= rprio and format("(%s)", b) or b
    return format("%s %s %s", ap, oper, bp), lprio, rprio
end

function ExpressionRule:UnaryExpression(node)
    local arg, arg_prio = self:expr_emit(node.argument)
    local op_prio = operator.unary_priority
    if arg_prio < op_prio then arg = format("(%s)", arg) end
    return format("%s%s", node.operator, arg), operator.unary_priority
end

ExpressionRule.LogicalExpression = ExpressionRule.BinaryExpression

function ExpressionRule:ConcatenateExpression(node)
    local ls = {}
    local cat_prio = operator.left_priority("..")
    for k = 1, #node.terms do
        ls[k], kprio = self:expr_emit(node.terms[k])
        if kprio < cat_prio then ls[k] = format("(%s)", ls[k]) end
    end
    return concat(ls, " .. "), cat_prio
end

function ExpressionRule:Table(node)
    local array = self:expr_list(node.array_entries)
    local hash = { }
    for k = 1, #node.hash_keys do
        local key = node.hash_keys[k]
        local value = self:expr_emit(node.hash_values[k])
        if is_string(key) then
            hash[k] = format("%s = %s", key.value, value)
        else
            hash[k] = format("[%s] = %s", self:expr_emit(key), value)
        end
    end
    local content = array
    if #hash > 0 then
        local hash_str = comma_sep_list(hash)
        content = content ~= "" and (content .. ", " .. hash_str) or hash_str
    end
    return "{" .. content .. "}", operator.ident_priority
end

function ExpressionRule:CallExpression(node)
    local callee, prio = self:expr_emit(node.callee)
    if prio < operator.ident_priority then
        callee = "(" .. callee .. ")"
    end
    local exp = format("%s(%s)", callee, self:expr_list(node.arguments))
    return exp, operator.ident_priority
end

function ExpressionRule:SendExpression(node)
    local rec, prio = self:expr_emit(node.receiver)
    if prio < operator.ident_priority then
        rec = "(" .. rec .. ")"
    end
    local method = node.method.name
    local exp = format("%s:%s(%s)", rec, method, self:expr_list(node.arguments))
    return exp, operator.ident_priority
end

function ExpressionRule:FunctionExpression(node)
    self:proto_enter()
    local header = format("function(%s)", comma_sep_list(node.params, as_parameter))
    self:add_section(header, node.body)
    local child_proto = self:proto_leave()
    return child_proto:inline(self.proto.indent), 0
end

LHSExpressionRule.Identifier = ExpressionRule.Identifier
LHSExpressionRule.MemberExpression = ExpressionRule.MemberExpression

function LHSExpressionRule:MatrixSliceExpression(node, assign_ctx)
    local var_name = genid()
    local obj = node.object
    local slice_fun = field(ident("matrix"), "__slice_assign")
    local arguments = { obj, node.row_start, node.row_end, node.col_start, node.col_end, ident(var_name) }
    local fcall = build("CallExpression", { callee = slice_fun, arguments = arguments })
    assign_ctx[#assign_ctx+1] = build("ExpressionStatement", { expression = fcall })
    return var_name
end

function StatementRule:FunctionDeclaration(node)
    self:proto_enter()
    local name = self:expr_emit(node.id)
    local header = format("function %s(%s)", name, comma_sep_list(node.params, as_parameter))
    if node.locald then
        header = "local " .. header
    end
    self:add_section(header, node.body)
    local child_proto = self:proto_leave()
    self.proto:merge(child_proto)
end

function StatementRule:CallExpression(node)
    local line = self:expr_emit(node)
    self:add_line(line)
end

function StatementRule:CheckIndex(node)
    local index = self:expr_emit(node.index)
    if node.inf then
        local inf = self:expr_emit(node.inf)
        local line = format("assert(%s >= %s, 'invalid index')", index, inf)
        self:add_line(line)
    end
    if node.sup then
        local sup = self:expr_emit(node.sup)
        local line = format("assert(%s <= %s, 'invalid index')", index, sup)
        self:add_line(line)
    end
end

function StatementRule:ForStatement(node)
    local init = node.init
    local istart = self:expr_emit(init.value)
    local iend = self:expr_emit(node.last)
    local header
    if node.step and not is_const(node.step, 1) then
        local step = self:expr_emit(node.step)
        header = format("for %s = %s, %s, %s do", init.id.name, istart, iend, step)
    else
        header = format("for %s = %s, %s do", init.id.name, istart, iend)
    end
    self:add_section(header, node.body)
end

function StatementRule:ForInStatement(node)
    local vars = comma_sep_list(node.namelist.names, as_parameter)
    local explist = self:expr_list(node.explist)
    local header = format("for %s in %s do", vars, explist)
    self:add_section(header, node.body)
end

function StatementRule:DoStatement(node)
    self:add_section("do", node.body)
end

function StatementRule:WhileStatement(node)
    local test = self:expr_emit(node.test)
    local header = format("while %s do", test)
    self:add_section(header, node.body)
end

function StatementRule:RepeatStatement(node)
    self:add_section("repeat", node.body, true)
    local test = self:expr_emit(node.test)
    local until_line = format("until %s", test)
    self:add_line(until_line)
end

function StatementRule:BreakStatement()
    self:add_line("break")
end

function StatementRule:IfStatement(node)
    local ncons = #node.tests
    for i = 1, ncons do
        local header_tag = i == 1 and "if" or "elseif"
        local test = self:expr_emit(node.tests[i])
        local header = format("%s %s then", header_tag, test)
        self:add_section(header, node.cons[i], true)
    end
    if node.alternate then
        self:add_section("else", node.alternate, true)
    end
    self:add_line("end")
end

function StatementRule:LocalDeclaration(node)
    local line
    local names = comma_sep_list(node.names, as_parameter)
    if #node.expressions > 0 then
        line = format("local %s = %s", names, self:expr_list(node.expressions))
    else
        line = format("local %s", names)
    end
    self:add_line(line)
end

function StatementRule:AssignmentExpression(node)
    local assign_ctx = { }
    local lhs = {}
    for i = 1, #node.left do
        lhs[i] = self:lhs_expr_emit(node.left[i], assign_ctx)
    end
    local line = format("%s = %s", comma_sep_list(lhs), self:expr_list(node.right))
    self:add_line(line)
    self:post_assignment_do(assign_ctx)
end

function StatementRule:Chunk(node)
    self:list_emit(node.body)
end

function StatementRule:ExpressionStatement(node)
    local line = self:expr_emit(node.expression)
    self:add_line(line)
end

function StatementRule:ReturnStatement(node)
    local line = format("return %s", self:expr_list(node.arguments))
    self:add_line(line)
end

function StatementRule:LabelStatement(node)
   self:add_line("::" .. node.label .. "::")
end

function StatementRule:GotoStatement(node)
   self:add_line("goto " .. node.label)
end

local function proto_inline(proto, indent)
    local ls = { }
    for k = 1, #proto.code do
        local indent_str = k == 1 and "" or string.rep("    ", indent)
        ls[k] = indent_str .. proto.code[k]
    end
    return concat(ls, "\n")
end

local function proto_merge(proto, child)
    for k = 1, #child.code do
        local line = child.code[k]
        local indent_str = string.rep("    ", proto.indent)
        proto.code[#proto.code + 1] = indent_str .. line
    end
end

local function proto_new(parent)
    local proto = { code = { }, indent = 0, parent = parent }
    proto.inline = proto_inline
    proto.merge = proto_merge
    return proto
end

local Generator = {}

function Generator:proto_enter()
    self.proto = proto_new(self.proto)
end

function Generator:proto_leave()
    local proto = self.proto
    self.proto = proto.parent
    return proto
end

function Generator:compile_code()
    return concat(self.code, "\n")
end

function Generator:indent_more()
    local proto = self.proto
    proto.indent = proto.indent + 1
end

function Generator:indent_less()
    local proto = self.proto
    proto.indent = proto.indent - 1
end

function Generator:line(line)
    -- FIXME: ignored for the moment
end

function Generator:add_line(line)
    local proto = self.proto
    local indent = string.rep("    ", proto.indent)
    proto.code[#proto.code + 1] = indent .. line
end

function Generator:add_section(header, body, omit_end)
    self:add_line(header)
    self:indent_more()
    self:list_emit(body)
    self:indent_less()
    if not omit_end then
        self:add_line("end")
    end
end

function Generator:lhs_expr_emit(node, assign_ctx)
    local rule = LHSExpressionRule[node.kind]
    if not rule then
        local node_trans = matrix_trans.lhs_expression(node)
        if node_trans then
            return self:lhs_expr_emit(node_trans, assign_ctx)
        end
    end
    if not rule then error("Missing LHS Expression Rule: ", node.kind) end
    return rule(self, node, assign_ctx)
end

function Generator:expr_emit(node)
    local rule = ExpressionRule[node.kind]
    if not rule then
        local node_trans = matrix_trans.expression(node)
        if node_trans then
            return self:expr_emit(node_trans)
        end
        error("Missing ExpressionRule: ", node.kind)
    end
    return rule(self, node)
end

function Generator:expr_list(exps)
    return comma_sep_list(exps, self.to_expr)
end

function Generator:emit(node)
    local rule = StatementRule[node.kind]
    if not rule then error("cannot find a statement rule for " .. node.kind) end
    rule(self, node)
    if node.line then self:line(node.line) end
end

function Generator:list_emit(node_list)
    for i = 1, #node_list do
        self:emit(node_list[i])
    end
end

function Generator:post_assignment_do(assign_ctx)
    for i = 1, #assign_ctx do
        self:emit(assign_ctx[i])
    end
end

local function generator_new(tree, name)
    local self = { }
    self.proto = proto_new()
    self.chunkname = tree.chunkname
    self.to_expr = function(node) return self:expr_emit(node) end
    return setmetatable(self, { __index = Generator })
end

local function generate(tree, name)
    local gen = generator_new(tree, name)
    gen:emit(tree)
    return gen:proto_leave():inline(0)
end

local function generate_expr(node)
    local gen = generator_new(node)
    return gen:expr_emit(node)
end

return { chunk = generate, expr = generate_expr }
