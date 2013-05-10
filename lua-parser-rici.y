%fallback  OPEN '(' .

chunk      ::= block .

semi       ::= ';' .
semi       ::= .

block      ::= scope statlist .
block      ::= scope statlist laststat semi .
ublock     ::= block 'until' exp .

scope      ::= .
scope      ::= scope statlist binding semi.
           
statlist   ::= .
statlist   ::= statlist stat semi .

stat       ::= 'do' block 'end' .
stat       ::= 'while' exp 'do' block 'end' .
stat       ::= repetition 'do' block 'end' .
stat       ::= 'repeat' ublock .
stat       ::= 'if' conds 'end' .
stat       ::= 'function' funcname funcbody .
stat       ::= setlist '=' explist1 .
stat       ::= functioncall .

repetition ::= 'for' NAME '=' explist23 .
repetition ::= 'for' namelist 'in' explist1 .
           
conds      ::= condlist .
conds      ::= condlist 'else' block .
condlist   ::= cond .
condlist   ::= condlist 'elseif' cond .
cond       ::= exp 'then' block .
           
laststat   ::= 'break' .
laststat   ::= 'return' .
laststat   ::= 'return' explist1 .

binding    ::= 'local' namelist .
binding    ::= 'local' namelist '=' explist1 .
binding    ::= 'local' 'function' NAME funcbody .

funcname   ::= dottedname .
funcname   ::= dottedname ':' NAME .

dottedname ::= NAME .
dottedname ::= dottedname '.' NAME .

namelist   ::= NAME .
namelist   ::= namelist ',' NAME .

explist1   ::= exp .
explist1   ::= explist1 ',' exp .
explist23  ::= exp ',' exp .
explist23  ::= exp ',' exp ',' exp .

%left      'or' .
%left      'and' .
%left      '<' '<=' '>' '>=' '==' '~=' .
%right     '..' .
%left      '+' '-' .
%left      '*' '/' '%' .
%right     'not' '#' .
%right     '^' .

exp        ::= 'nil'|'true'|'false'|NUMBER|STRING|'...' .
exp        ::= function .
exp        ::= prefixexp .
exp        ::= tableconstructor .
exp        ::= 'not'|'#'|'-' exp .         ['not']
exp        ::= exp 'or' exp .
exp        ::= exp 'and' exp .
exp        ::= exp '<'|'<='|'>'|'>='|'=='|'~=' exp .
exp        ::= exp '..' exp .
exp        ::= exp '+'|'-' exp .
exp        ::= exp '*'|'/'|'%' exp .
exp        ::= exp '^' exp .
           
setlist    ::= var .
setlist    ::= setlist ',' var .

var        ::= NAME .
var        ::= prefixexp '[' exp ']' .
var        ::= prefixexp '.' NAME .

prefixexp  ::= var .
prefixexp  ::= functioncall .
prefixexp  ::= OPEN exp ')' .

functioncall ::= prefixexp args .
functioncall ::= prefixexp ':' NAME args .

args        ::= '(' ')' .
args        ::= '(' explist1 ')' .
args        ::= tableconstructor .
args        ::= STRING .

function    ::= 'function' funcbody .

funcbody    ::= params block 'end' .

params      ::= '(' parlist ')' .

parlist     ::= .
parlist     ::= namelist .
parlist     ::= '...' .
parlist     ::= namelist ',' '...' .

tableconstructor ::= '{' '}' .
tableconstructor ::= '{' fieldlist '}' .
tableconstructor ::= '{' fieldlist ','|';' '}' .

fieldlist   ::= field .
fieldlist   ::= fieldlist ','|';' field .
            
field       ::= exp .
field       ::= NAME '=' exp .
field       ::= '[' exp ']' '=' exp .

