local exports = { }

local util   = require('nyanga.util')
local syntax = require('nyanga.syntax')

function exports.tempnam()
   return exports.identifier(util.genid())
end
function exports.chunk(body, line)
   return syntax.build("Chunk", { body = body, line = line })
end
function exports.identifier(name, line)
   return syntax.build("Identifier", { name = name, line = line })
end
function exports.vararg(line)
   return syntax.build("Vararg", { line = line })
end
function exports.concatenateExpression(terms, line)
   return syntax.build("ConcatenateExpression", {
      terms = terms, line = line
   })
end
function exports.binaryExpression(op, left, right, line)
   return syntax.build("BinaryExpression", {
      operator = op, left = left, right = right, line = line
   })
end
function exports.unaryExpression(op, arg, line)
   return syntax.build("UnaryExpression", {
      operator = op, argument = arg, line = line
   })
end

function exports.parenExpression(exprs, line)
   return syntax.build("ParenExpression", {
      expressions = exprs, line = line
   })
end
function exports.assignmentExpression(left, right, line)
   return syntax.build("AssignmentExpression", {
      left = left, right = right, line = line
   })
end
function exports.logicalExpression(op, left, right, line)
   return syntax.build("LogicalExpression", {
      operator = op, left = left, right = right, line = line
   })
end
function exports.memberExpression(obj, prop, comp, line)
   return syntax.build("MemberExpression", {
      object = obj, property = prop, computed = comp or false, line = line
   })
end
function exports.callExpression(callee, args, line)
   return syntax.build("CallExpression", {
      callee = callee, arguments = args, line = line
   })
end
function exports.sendExpression(recv, meth, args, line)
   return syntax.build("SendExpression", {
      receiver = recv, method = meth, arguments = args, line = line
   })
end
function exports.literal(val, line)
   return syntax.build("Literal", { value = val, line = line })
end
--[[
  The arguments are, in the given order:
    * the "array" terms (without explicit index)
    * the "hash" keys
    * the "hash" values ]]
function exports.table(as, hks, hvs, line)
   return syntax.build("Table", { array_entries = as, hash_keys = hks, hash_values = hvs, line = line })
end
function exports.expressionStatement(expr, line)
   return syntax.build("ExpressionStatement", { expression = expr, line = line })
end
function exports.emptyStatement(line)
   return syntax.build("EmptyStatement", { line = line })
end
function exports.blockStatement(body, line)
   return syntax.build("BlockStatement", { body = body, line = line })
end
function exports.doStatement(body, line)
   return syntax.build("DoStatement", { body = body, line = line })
end
function exports.ifStatement(test, cons, alt, line)
   return syntax.build("IfStatement", {
      test = test, consequent = cons, alternate = alt, line = line
   })
end
function exports.labelStatement(label, line)
   return syntax.build("LabelStatement", { label = label, line = line })
end
function exports.gotoStatement(label, line)
   return syntax.build("GotoStatement", { label = label, line = line })
end
function exports.breakStatement(line)
   return syntax.build("BreakStatement", { line = line })
end
function exports.returnStatement(arg, line)
   return syntax.build("ReturnStatement", { arguments = arg, line = line })
end
function exports.whileStatement(test, body, line)
   return syntax.build("WhileStatement", {
      test = test, body = body, line = line
   })
end
function exports.repeatStatement(test, body, line)
   return syntax.build("RepeatStatement", {
      test = test, body = body, line = line
   })
end
function exports.forInit(name, value, line)
   return syntax.build("ForInit", { id = name, value = value, line = line })
end
function exports.forStatement(init, last, step, body, line)
   return syntax.build("ForStatement", {
      init = init, last = last, step = step, body = body, line = line
   })
end
function exports.forNames(names, line)
   return syntax.build("ForNames", { names = names, line = line })
end
function exports.forInStatement(init, iter, body, line)
   return syntax.build("ForInStatement", {
      init = init, iter = iter, body = body, line = line
   })
end
function exports.localDeclaration(names, exprs, line)
   return syntax.build("LocalDeclaration", {
      names = names, expressions = exprs, line = line
   })
end
function exports.functionDeclaration(name, params, body, vararg, local_decl, line)
   return syntax.build("FunctionDeclaration", {
      id         = name,
      body       = body,
      params     = params or { },
      vararg     = vararg,
      locald     = local_decl,
      line       = line
   })
end
function exports.functionExpression(params, body, vararg, line)
   return syntax.build("FunctionExpression", {
      body       = body,
      params     = params or { },
      vararg     = vararg,
      line       = line
   })
end


return exports
