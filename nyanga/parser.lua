local util = require('nyanga.util')
local re   = require('lpeg.re')
local defs = require('nyanga.parser.defs')
local lpeg = require('lpeg')
lpeg.setmaxstack(1024)

local patt = [[
   chunk  <- {|
      s (<stmt> (<sep> s <stmt>)* <sep>?)? s (!. / '' => error)
   |} -> chunk

   close    <- ']' =eq ']' / . close

   lcomment <- (!%nl %s)* "--" (!%nl .)* %nl
   bcomment <- ('--[' {:eq: '='* :} '[' <close>)
   comment  <- <bcomment> / <lcomment>
   idsafe   <- !(%alnum / "_")
   s        <- (<comment> / %s)*
   S        <- (<comment> / %s)+
   word     <- (%alpha / "_") (%alnum / "_")*

   keyword  <- (
      "local" / "function"
      / "nil" / "true" / "false" / "return" / "end"
      / "break" / "not"
      / "while" / "do" / "for" / "in" / "and" / "or"
      / "if" / "elseif" / "else" / "then"
      / "repeat" / "until"
   ) <idsafe>

   sep <- <bcomment>? (%nl / ";" / &"}" / <lcomment>) / %s <sep>?

   escape <- {~ ('\' (%digit^3 / .)) -> escape ~}

   astring <- "'" {~ (<escape> / {!"'" .})* ~} "'"
   qstring <- '"' {~ (<escape> / {!'"' .})* ~} '"'
   lstring <- ('[' {:eq: '='* :} '[' <close>)

   string  <- <qstring> / <astring> / <lstring>

   hexnum <- "-"? "0x" %xdigit+

   decexp <- ("e"/"E") ("-"/"+")? %digit+

   decimal_base <- (
      %digit+ ("." %digit*)? / "." %digit+
   )

   decimal_number <- "-"? <decimal_base> <decexp>?

   octal   <- {~ { "-"? "0" [0-7]+ } -> octal ~}

   number  <- {~
      <hexnum> / <octal> / <decimal_number>
   ~} -> tonumber

   boolean <- (
      {"true"/"false"} <idsafe>
   ) -> boolean

   literal <- ( <number> / <string> / <boolean> ) -> literal

   in  <- "in"  <idsafe>
   end <- "end" <idsafe>
   do  <- "do"  <idsafe>

   stmt <- ({} (
      <if_stmt>
      / <while_stmt>
      / <repeat_stmt>
      / <for_stmt>
      / <for_in_stmt>
      / <do_stmt>
      / <expr_stmt>
      / <decl_stmt>
      / <return_stmt>
      / <break_stmt>
   )) -> stmt

   stmt_list <- {|
      (<stmt> (<sep> s <stmt>)* <sep>?)?
   |}

   break_stmt <- (
      "break" <idsafe>
   ) -> breakStmt

   return_stmt <- (
      "return" <idsafe> s {| <expr_list>? |}
   ) -> returnStmt

   decl_stmt <- (
      <local_decl> / <func_decl>
   )

   local_decl <- (
      "local" <idsafe> s {| <name_list> |} (s "=" s {| <expr_list> |})?
   ) -> localDecl

   name_list <- (
      <ident> (s "," s <ident>)*
   )

   vararg_expr <- "..." -> varargExpr

   expr_list <- (
        <vararg_expr>
      / <expr> (s "," s <expr>)* (s "," s <vararg_expr>)?
   )

   func_path <- {|
      <ident> (s {"."/":"} s <ident>)*
   |}

   func_decl <- (
      {"local" <idsafe> s / ""} "function" <idsafe> s <func_path> s <func_head> s <func_body>
   ) -> funcDecl

   func_head <- (
      "(" s {| <param_list>? |} s ")"
   )

   lambda_func_expr <- (
      "|" s {| <param_list>? |} s "|" s <expr>
   ) -> lambdaFuncExpr

   func_expr <- (
      "function" <idsafe> s <func_head> s <func_body>
   ) -> funcExpr

   func_body <- <block_stmt> s <end> / <expr>

   param <- {| {:name: <ident> :} |}

   param_list <- (
        <param> s "," s <param_list>
      / <param>
      / {| "..." {:vararg: '' -> 'true' :} |}
   )

   block_stmt <- (
      {| (<stmt> (<sep> s <stmt>)* <sep>?)? |}
   ) -> blockStmt

   if_stmt <- (
      "if" <idsafe> s <expr> s "then" <idsafe> s <block_stmt> s (
           "else" <if_stmt>
         / "else" <idsafe> s <block_stmt> s <end>
         / <end>
      )
   ) -> ifStmt

   for_stmt <- (
      "for" <idsafe> s <ident> s "=" s <expr> s "," s <expr>
      (s "," s <expr> / ('' -> '1') -> literalNumber) s
      <loop_body>
   ) -> forStmt

   for_in_stmt <- (
      "for" <idsafe> s {| <name_list> |} s <in> s <expr> s
      <loop_body>
   ) -> forInStmt

   loop_body <- <do> s <block_stmt> s <end>

   do_stmt <- <loop_body> -> doStmt

   while_stmt <- (
      "while" <idsafe> s <expr> s <loop_body>
   ) -> whileStmt

   repeat_stmt <- (
      "repeat" <idsafe> s <block_stmt> s "until" <idsafe> s <expr>
   ) -> repeatStmt

   ident <- (
      !<keyword> { <word> }
   ) -> identifier

   expr_row <- {|
      <expr> (s "," s <expr>)*
   |}

   matrix_literal <- {|
      '[' s <expr_row> (s ';' s <expr_row>)* s ']'
   |} -> matrixLiteral

   term <- (
        <func_expr>
      / <nil_expr>
      / <table_expr>
      / <ident>
      / <literal>
      / <matrix_literal>
      / "(" s <expr> s ")"
   )

   nil_expr <- (
      "nil" <idsafe>
   ) -> nilExpr

   expr_stmt <- (
      {} (<content_assign> / <assign_expr> / <expr>)
   ) -> exprStmt

   binop <- !"<-" {
      "+" / "-" / "/" / "^" / "*" / "%"
      / ">=" / ">" / "<=" / "<" / ".."
      / "~=" / "==" / ("or" / "and") <idsafe>
   }

   expr  <- (
      {| <prefix_expr> (s <binop> s <prefix_expr>)+ |}
   ) -> infixExpr / <prefix_expr> / <lambda_func_expr>

   prefix_expr <- (
      { "`" / "#" / "+" / "-" / "not" <idsafe> } s <prefix_expr>
   ) -> prefixExpr / <postfix_expr>

   postfix_expr <- {|
      <term> (s <postfix_tail>)+
   |} -> postfixExpr / <term>

   range <- {|
      {:start: <expr>? :} (s {:range: "~" :} s {:stop: <expr>? :} )?
   |}

   expr_r <- {| {:start: <expr> :} |}

   member_postfix <- (
        { "." } s <ident>
      / { ":" } s (<ident> / '' => error)
   )

   index_postfix <-
      { "[" } s <expr_r> (s "," s <expr_r>)? s ("]" / '' => error)

   range_postfix <-
      { "[" } s <range> (s "," s <range>)? s ("]" / '' => error)

   postfix_tail <- {|
        <member_postfix>
      / <range_postfix>
      / { "(" } s {| <expr_list>? |} s (")" / '' => error)
      / {~ '' -> "(" ~} s {| <table_expr> |}
      / {~ '' -> "(" ~} s {| <string> -> literal |}
   |}

   member_expr <- {|
      <term> (s <member_next>)?
   |} -> postfixExpr / <term>

   member_next <- (
      <postfix_tail> s <member_next> / <member_tail>
   )
   member_tail <- {|
        <member_postfix>
      / <index_postfix>
   |}

   assign_expr <- (
      {| <member_expr> (s "," s <member_expr>)* |} s "=" s {| <expr_list> |}
   ) -> assignExpr

   range_expr <- {|
      <term> (s <range_tail>)+
   |} -> postfixExpr / <term>

   range_tail <- {|
        <member_postfix>
      / <range_postfix>
   |}

   content_assign <- (
      {} <range_expr> s "<-" s <expr>
   ) -> contentAssign

   table_expr <- (
      "{" s {| <table_members>? |} s "}"
   ) -> tableExpr

   table_members <- (
      <table_member> (s (","/";") s <table_member>)* (s (","/";"))?
   )
   table_member <- {|
      {:key: ("[" s <expr> s "]" / <ident>) :} s "=" s {:value: <expr> :}
    / {:value: <expr> :} |}
]]

local grammar = re.compile(patt, defs)
local function parse(src)
   return grammar:match(src)
end

return {
   parse = parse
}


