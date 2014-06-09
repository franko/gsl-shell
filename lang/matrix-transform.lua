local syntax = require('syntax')

local build, ident, literal, logical, binop, field, tget = syntax.build, syntax.ident, syntax.literal, syntax.logical, syntax.binop, syntax.field, syntax.tget

local ExpressionRule = { }
local LHSExpressionRule = { }

function ExpressionRule.MatrixSliceExpression(node)
    local obj = node.object
    local slice_fun = field(ident("matrix"), "__slice")
    local arguments = { obj, node.row_start, node.row_end, node.col_start, node.col_end }
    return build("CallExpression", { callee = slice_fun, arguments = arguments })
end

function ExpressionRule.Matrix(node)
    local build_fun = field(ident("matrix"), "build")
    local t = build("Table", { array_entries = node.terms, hash_keys = {}, hash_values = {} })
    local arguments = { t, literal(node.ncols) }
    return build("CallExpression", { callee = build_fun, arguments = arguments })
end

function ExpressionRule.MatrixIndex(node)
    local obj = node.object
    if node.safe then
        local one = literal(1)
        local index = binop("*", field(obj, "tda"), binop("-", node.row, one))
        index = binop("+", index, binop("-", node.col, one))
        return tget(field(obj, "data"), index)
    else
        local args = { node.row, node.col }
        return build("SendExpression", { receiver = obj, method = ident("get"), arguments = args })
    end
end

LHSExpressionRule.MatrixIndex = ExpressionRule.MatrixIndex

local function transform(rule_lookup, node)
    local rule = rule_lookup[node.kind]
    if rule then
        return rule(node)
    end
end

local function lookup(rule_lookup)
    return function(node) return transform(rule_lookup, node) end
end

return { expression = lookup(ExpressionRule), lhs_expression = lookup(LHSExpressionRule) }
