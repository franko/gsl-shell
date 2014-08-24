local syntax = require('syntax')
local build, tget, field = syntax.build, syntax.tget, syntax.field

local function is_ident(node)
    return node.kind == "Identifier"
end

local function is_binop(node, op)
    return node.kind == "BinaryExpression" and node.operator == op
end

local function is_unop(node, op)
    return node.kind == "UnaryExpression" and node.operator == op
end

local function literal(value)
    return build("Literal", { value = value })
end

local function op_eval(op, a, b)
    if     op == "+" then return a + b
    elseif op == "-" then return a - b
    elseif op == "*" then return a * b
    elseif op == "/" and b ~= 0 then return a / b
    elseif op == "^" then return a^b end
end

local function is_const(expr)
    if expr.kind == "Literal" and type(expr.value) == "number" then
        return expr.value
    elseif expr.kind == "BinaryExpression" then
        local a = is_const(expr.left)
        local b = is_const(expr.right)
        if a and b then return op_eval(expr.operator, a, b) end
    elseif expr.kind == "UnaryExpression" then
        local a = is_const(expr.argument)
        if a and expr.operator == "-" then return -a end
    end
end

local function binop(op, left, right)
    local lconst = is_const(left)
    local rconst = is_const(right)
    if lconst and rconst then
        local r = op_eval(op, lconst, rconst)
        if r then return literal(r) end
    elseif op == "*" then
        if lconst == 1 then
            return right
        elseif rconst == 1 then
            return left
        elseif lconst == 0 or rconst == 0 then
            return literal(0)
        end
    elseif op == "/" then
        if rconst == 1 then return left end
        if lconst == 0 and rconst and rconst ~= 0 then return literal(0) end
    elseif op == "+" then
        if lconst == 0 then
            return right
        elseif rconst == 0 then
            return left
        end
    elseif op == "-" then
        if rconst == 0 then
            return left
        elseif lconst == 0 then
            return build("UnaryExpression", { operator = op, argument = right })
        end
    end
    return build("BinaryExpression", { operator = op, left = left, right = right })
end

local function unop(op, arg)
    if op == "-" then
        local aconst = is_const(arg)
        if aconst then return literal(-aconst) end
    end
    return build("UnaryExpression", { operator = op, argument = arg })
end

local function linear_ctxfree(expr, var, ast)
    if is_ident(expr) then
        local expr_var_name = expr.name
        if expr_var_name == var.name then
            return expr, true, 1
        else
            local vinfo, var_scope = ast:lookup_local(expr_var_name)
            local num_const = vinfo and vinfo.num_const
            if num_const and var_scope ~= ast.current then
                -- The variable is a local in an outer scope.
                return expr, true, 0
            end
        end
    elseif is_const(expr) then
        return expr, true, 0
    elseif is_binop(expr, "+") then
        local aexp, alin, acoeff = linear_ctxfree(expr.left, var, ast)
        local bexp, blin, bcoeff = linear_ctxfree(expr.right, var, ast)
        if alin and blin then
           return binop("+", aexp, bexp), true, acoeff + bcoeff
        end
    elseif is_binop(expr, "-") then
        local aexp, alin, acoeff = linear_ctxfree(expr.left, var, ast)
        local bexp, blin, bcoeff = linear_ctxfree(expr.right, var, ast)
        if alin and blin then
           return binop("-", aexp, bexp), true, acoeff - bcoeff
        end
    elseif is_binop(expr, "*") then
        local aconst = is_const(expr.left)
        if aconst then
            local bexp, blin, bcoeff = linear_ctxfree(expr.right, var, ast)
            if blin then return binop("*", literal(aconst), bexp), true, aconst * bcoeff end
        else
            local aexp, alin, acoeff = linear_ctxfree(expr.left, var, ast)
            if alin then
                local bconst = is_const(expr.right)
                if bconst then return binop("*", aexp, literal(bconst)), true, acoeff * bconst end
            end
        end
    elseif is_binop(expr, "/") then
        local aexp, alin, acoeff = linear_ctxfree(expr.left, var, ast)
        local bconst = is_const(expr.right)
        if bconst and alin then
            return binop("/", aexp, literal(bconst)), true, acoeff / bconst
        end
    elseif is_unop(expr, "-") then
        local aexp, alin, acoeff = linear_ctxfree(expr.argument, var, ast)
        if alin then return unop("-", aexp), true, -acoeff end
    end
    return false
end

local function var_const(vinfo)
    return vinfo and (not vinfo.mutable)
end

local function var_num_const(vinfo)
    return vinfo and vinfo.num_const
end

local function expr_is_context_free(expr, ast, var_predicate)
    if is_ident(expr) then
        local vinfo, var_scope = ast:lookup_local(expr.name)
        if var_predicate(vinfo) then
            return expr
        end
    elseif expr.kind == "Literal" and type(expr.value) == "number" then
        return expr
    elseif expr.kind == "BinaryExpression" then
        local a = expr_is_context_free(expr.left, ast, var_predicate)
        local b = expr_is_context_free(expr.right, ast, var_predicate)
        if a and b then return binop(expr.operator, a, b) end
    elseif expr.kind == "UnaryExpression" then
        local a = expr_is_context_free(expr.argument, ast, var_predicate)
        if a then return unop(expr.operator, a) end
    elseif expr.kind == "MemberExpression" then
        local obj_resolve = expr_is_context_free(expr.object, ast, var_const)
        if obj_resolve then
            if expr.computed then
                local prop_resolve = expr_is_context_free(expr.property, ast, var_const)
                if prop_resolve then
                    return tget(obj_resolve, prop_resolve)
                end
            else
                return tget(obj_resolve, expr.property)
            end
        end
    end
    return false
end

local function expr_eval_subst(expr, var, value)
    if is_ident(expr) and expr.name == var.name then
        return value
    elseif expr.kind == "BinaryExpression" then
        local left = expr_eval_subst(expr.left, var, value)
        local right = expr_eval_subst(expr.right, var, value)
        return binop(expr.operator, left, right)
    elseif expr.kind == "UnaryExpression" then
        local arg = expr_eval_subst(expr.argument, var, value)
        return unop(expr.operator, arg)
    elseif expr.kind == "MemberExpression" then
        local obj_resolve = expr_eval_subst(expr.object, var, value)
        if expr.computed then
            local prop_resolve = expr_eval_subst(expr.property, var, value)
            return tget(obj_resolve, prop_resolve)
        else
            return field(obj_resolve, expr.property.name)
        end
    else
        return expr
    end
end

local function expr_eval(expr, ast)
    if is_ident(expr) then
        local expr_var_name = expr.name
        local vinfo, var_scope = ast:lookup_local(expr_var_name)
        local num_const = vinfo and vinfo.num_const
        if num_const and var_scope == ast.current and vinfo.value then
            return expr_eval(vinfo.value, ast)
        end
        return expr
    elseif expr.kind == "BinaryExpression" then
        local left = expr_eval(expr.left, ast)
        local right = expr_eval(expr.right, ast)
        return binop(expr.operator, left, right)
    elseif expr.kind == "UnaryExpression" then
        local arg = expr_eval(expr.argument, ast)
        return unop(expr.operator, arg)
    elseif expr.kind == "MemberExpression" then
        local obj_resolve = expr_eval(expr.object, ast)
        if expr.computed then
            local prop_resolve = expr_eval(expr.property, ast)
            return tget(obj_resolve, prop_resolve)
        else
            return field(obj_resolve, expr.property.name)
        end
    else
        return expr
    end
end

return { is_const = is_const, linear_ctxfree = linear_ctxfree, context_free  = expr_is_context_free, eval_subst = expr_eval_subst, eval = expr_eval, const_predicate = var_const, num_const_predicate = var_num_const }
