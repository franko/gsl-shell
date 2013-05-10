%include {

#include <stdlib.h>

struct expr;

struct infix {
    int oper;
    struct expr *a, *b;
};

struct unary {
    int oper;
    struct expr *a;
};

struct expr {
    int tag;
    union {
        struct infix infix;
        struct unary unary;
        const char *id;
        double number;
        int boolean;
    } value;
};

struct expr *unary(int op, struct expr *a) {
    struct expr *bin = malloc(sizeof(struct expr));
    bin->tag = EXPR_UNARY;
    struct unary *c = &bin->value.unary;
    c->oper = op;
    c->a = a;
    return bin;
}

struct expr* infix(int op, struct expr *a, struct expr *b) {
    struct expr *bin = malloc(sizeof(struct expr));
    bin->tag = EXPR_INFIX;
    struct infix *c = &bin->value.infix;
    c->oper = op;
    c->a = a;
    c->b = b;
    return bin;
}
}

%left SEMICOLON.
%left NIL TRUE FALSE NUMBER STRING ELLIPSES.
%left NOT SHARP.
%left OR.
%left AND.
%nonassoc LT LEQT GT GEQT EQUAL NOT_EQUAL.
%left CONCAT.
%left PLUS MINUS.
%left MULT DIV MOD.
%right RAISE.

%type expr { struct expr * }

line ::= exp(A) SEMICOLON. { print_expr(A); }


exp(A)        ::= NIL|TRUE|FALSE|NUMBER|STRING|ELLIPSES(X) . { A = X; }
// exp(A)        ::= function(X) . { A = X; }
// exp(A)        ::= prefixexp(X) . { A = X; }
// exp(A)        ::= tableconstructor(X) . { A = X; }
exp(A)        ::= NOT|SHARP|MINUS(S) exp(X) . [NOT] { A = unary(S, X); }
exp(A)        ::= exp(B) OR(P) exp(C) . { A = logical(P, B, C); }
exp(A)        ::= exp(B) AND(P) exp(C) .  { A = logical(P, B, C); }
exp(A)        ::= exp(B) LT|LEQT|GT|GEQT|EQUAL|NOT_EQUAL(S) exp(C) . { A = compare(S, B, C); }
exp(A)        ::= exp(B) CONCAT(P) exp(C) . { A = infix(P, B, C); }
exp(A)        ::= exp(B) PLUS|MINUS(P) exp(C) . { A = infix(P, B, C); }
exp(A)        ::= exp(B) MULT|DIV|MOD(P) exp(C) . { A = infix(P, B, C); }
exp(A)        ::= exp(B) RAISE(P) exp(C) . { A = infix(P, B, C); }
