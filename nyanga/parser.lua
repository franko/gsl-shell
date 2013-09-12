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
   hs       <- (!%nl %s)*
   HS       <- (!%nl %s)+
   digits   <- %digit (%digit / (&('_' %digit) '_') %digit)*
   word     <- (%alpha / "_") (%alnum / "_")*

   keyword  <- (
      "local" / "function"
      / "nil" / "true" / "false" / "return" / "end"
      / "break" / "not"
      / "while" / "do" / "for" / "in" / "of" / "and" / "or"
      / "if" / "elseif" / "else" / "then" / "is" / "typeof"
      / "repeat" / "until"
   ) <idsafe>

   sep <- <bcomment>? (%nl / ";" / &"}" / <lcomment>) / %s <sep>?

   astring <- "'" { (!"'" .)* } "'"
   qstring <- '"' { (!'"' .)* } '"'
   lstring <- ('[' {:eq: '='* :} '[' <close>)

   string  <- (
      <qstring> / <astring> / <lstring>
   ) -> string

   hexnum <- "-"? "0x" %xdigit+

   decexp <- ("e"/"E") "-"? <digits>

   decimal <- "-"? <digits> ("." <digits> <decexp>? / <decexp>)

   integer <- "-"? <digits>

   octal   <- {~ { "-"? "0" [0-7]+ } -> octal ~}

   number  <- {~
      <hexnum> / <octal> / <decimal> / <integer>
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

   expr_list <- (
      <expr> (s "," s <expr>)*
   )

   func_path <- {|
      <ident> (s {"."/":"} s <ident>)*
   |}

   func_decl <- (
      "function" <idsafe> s <func_path> s <func_head> s <func_body>
   ) -> funcDecl

   func_head <- (
      "(" s {| <param_list>? |} s ")"
   )

   func_expr <- (
      "function" <idsafe> s <func_head> s <func_body>
      / (<func_head> / {| |}) s "=>" s <func_body>
   ) -> funcExpr

   func_body <- <block_stmt> s <end> / <expr>

   param <- {|
      {:name: <ident> :} (s "=" s {:default: <expr> :})?
   |}
   param_list <- (
        <param> s "," s <param_list>
      / <param> s "," s <param_rest>
      / <param>
      / <param_rest>
   )

   param_rest <- {| "..." {:name: <ident> :} {:rest: '' -> 'true' :} |}

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
      (s "," s <expr> / ('' -> '1') -> tonumber) s
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

   term <- (
        <func_expr>
      / <nil_expr>
      / <table_expr>
      / <ident>
      / <literal>
      / "(" s <expr> s ")"
   )

   expr <- <infix_expr> / <spread_expr>

   spread_expr <- (
      "..." <postfix_expr>
   ) -> spreadExpr

   nil_expr <- (
      "nil" <idsafe>
   ) -> nilExpr

   expr_stmt <- (
      {} (<assign_expr> / <update_expr> / <expr>)
   ) -> exprStmt

   binop <- {
      "+" / "-" / "~" / "/" / "**" / "*" / "%" / "^" / "|" / "&"
      / ">>>" / ">>" / ">=" / ">" / "<<" / "<=" / "<" / ".."
      / "!=" / "==" / ("or" / "and" / "is") <idsafe>
   }

   infix_expr  <- (
      {| <prefix_expr> (s <binop> s <prefix_expr>)+ |}
   ) -> infixExpr / <prefix_expr>

   prefix_expr <- (
      { "#" / "~" / "+" / "-" / "!" / ("not" / "typeof") <idsafe> } s <prefix_expr>
   ) -> prefixExpr / <postfix_expr>

   postfix_expr <- {|
      <term> <postfix_tail>+
   |} -> postfixExpr / <term>

   postfix_tail <- {|
      s { "." } s <ident>
      / { ":" } s (<ident> / '' => error)
      / { "[" } s <expr> s ("]" / '' => error)
      / { "(" } s {| <expr_list>? |} s (")" / '' => error)
      / {~ HS -> "(" ~} {| !<binop> <expr_list> |}
   |}

   member_expr <- {|
      <term> <member_next>?
   |} -> postfixExpr / <term>

   member_next <- (
      <postfix_tail> <member_next> / <member_tail>
   )
   member_tail <- {|
      s { "." } s <ident>
      / { ":" } s <ident>
      / { "[" } s <expr> s ("]" / '' => error)
   |}

   assop <- {
      "+=" / "-=" / "~=" / "**=" / "*=" / "/=" / "%="
      / "|=" / "&=" / "^=" / "<<=" / ">>>=" / ">>="
   }

   left_expr <- (
      <member_expr> / <ident>
   )

   assign_expr <- (
      {| <left_expr> (s "," s <left_expr>)* |} s "=" s {| <expr_list> |}
   ) -> assignExpr

   update_expr <- (
      <left_expr> s <assop> s <expr>
   ) -> updateExpr

   table_expr <- (
      "{" s {| <table_members>? |} s "}"
   ) -> tableExpr

   table_members <- (
      <table_member> (hs (","/";"/%nl) s <table_member>)* (hs (","/";"/%nl))?
   )
   table_member <- ({|
      {:key: ("[" s <expr> s "]" / <ident>) :} s "=" s {:value: <expr> :}
   |} / <ident>)
]]

local grammar = re.compile(patt, defs)
local function parse(src)
   return grammar:match(src)
end

return {
   parse = parse
}


