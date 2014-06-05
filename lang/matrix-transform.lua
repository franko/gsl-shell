local syntax = require('syntax')

local build, ident, literal, logical, binop, field, tget = syntax.build, syntax.ident, syntax.literal, syntax.logical, syntax.binop, syntax.field, syntax.tget

local ExpressionRule = { }

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

local function transform_expression(node)
	local rule = ExpressionRule[node.kind]
	if rule then
		return rule(node)
	end
end

return { expression = transform_expression }
