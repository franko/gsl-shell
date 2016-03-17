LuaJIT Language Toolkit
===

The LuaJIT Language Toolkit is an implementation of the Lua programming language written in Lua itself.
It works by generating LuaJIT bytecode, including debug information, and uses LuaJIT's virtual machine to run the generated bytecode.

On its own, the language toolkit does not do anything useful, since LuaJIT itself does the same things natively.
The purpose of the language toolkit is to provide a starting point to implement a programming language that targets the LuaJIT virtual machine.

With the LuaJIT Language Toolkit, it is easy to create a new language or modify the Lua language because the parser is cleanly separated from the bytecode generator and the virtual machine.

The toolkit implements a complete pipeline to parse a Lua program, generate an AST, and generate the corresponding bytecode.

Lexer
---

Its role is to recognize lexical elements from the program text.
It takes the text of the program as input and produces a stream of "tokens" as its output.

Using the language toolkit you can run the lexer only, to examinate the stream of tokens:

```
luajit run-lexer.lua tests/test-1.lua
```

The command above will lex the following code fragment:

```lua
local x = {}
for k = 1, 10 do
    x[k] = k*k + 1
end
```

...to generate the list of tokens:

    TK_local
    TK_name	x
    =
    {
    }
    TK_for
    TK_name	k
    =
    TK_number	1
    ,
    TK_number	10
    TK_do
    TK_name	x
    [
    TK_name	k
    ]
    =
    TK_name	k
    *
    TK_name	k
    +
    TK_number	1
    TK_end

Each line represents a token where the first element is the kind of token and the second element is its value, if any.

The Lexer's code is an almost literal translation of the LuaJIT's lexer.

Parser
---

The parser takes the token stream from the lexer and builds statements and expressions according to the language's grammar.
The parser is based on a list of parsing rules that are invoked each time the input matches a given rule.
When the input matches a rule, a corresponding function in the AST (abstract syntax tree) module is called to build an AST node.
The generated nodes in turns are passed as arguments to the other parsing rules until the whole program is parsed and a complete AST is built for the program text.

The AST is very useful as an abstraction of the structure of the program, and is easier to manipulate.

What distinguishes the language toolkit from LuaJIT is that the parser phase generates an AST, and the bytecode generation is done in a separate phase only when the AST is complete.

LuaJIT itself operates differently.
During the parsing phase it does not generate any AST but instead the bytecode is directly generated and loaded into the memory to be executed by the VM.
This means that LuaJIT's C implementation performs the three operations:

- parse the program text
- generate the bytecode
- load the bytecode into memory

in one single pass.
This approach is remarkable and very efficient, but makes it difficult to modify or extend the programming language.

### Parsing Rule example ###

To illustrate how parsing works in the language toolkit, let us make an example.
The grammar rule for the "return" statement is:

```
explist ::= {exp ','} exp

return_stmt ::= return [explist]
```

In this case the toolkit parser's rule will parse the optional expression list by calling the function `expr_list`.
Then, once the expressions are parsed the AST's rule `ast:return_stmt(exps, line)` will be invoked by passing the expressions list obtained before.

```lua
local function parse_return(ast, ls, line)
    ls:next() -- Skip 'return'.
    ls.fs.has_return = true
    local exps
    if EndOfBlock[ls.token] or ls.token == ';' then -- Base return.
        exps = { }
    else -- Return with one or more values.
        exps = expr_list(ast, ls)
    end
    return ast:return_stmt(exps, line)
end
```

As you can see, the AST functions are invoked using the `ast` object.

In addition, the parser provides information about:

* the function prototype
* the syntactic scope

The first is used to keep track of some information about the current function being parsed.

The syntactic scope rules tell the user's rule when a new syntactic block begins or end.
Currently this is not really used by the AST builder but it can be useful for other implementations.

The Abstract Syntax Tree (AST)
---

The abstract syntax tree represent the whole Lua program, with all the information the parser has gathered about it.

One possible approach to implement a new programming language is to generate an AST that more closely corresponds to the target programming language, and then transform the tree into a Lua AST in a separate phase.

Another possible approach is to directly generate the appropriate Lua AST nodes from the parser itself.

Currently the language toolkit does not perform any additional transformations, and just passes the AST to the bytecode generator module.

Bytecode Generator
---

Once the AST is generated, it can be fed to the bytecode generator module, which will generate the corresponding LuaJIT bytecode.

The bytecode generator is based on the original work of Richard Hundt for the Nyanga programming language.
It was largely modified by myself to produce optimized code similar to what LuaJIT would generate, itself.
A lot of work was also done to ensure the correctness of the bytecode and of the debug information.

Alternative Lua Code generator
------------------------------

Instead of passing the AST to the bytecode generator, an alternative module can be used to generate Lua code.
The module is called "luacode-generator" and can be used exactly like the bytecode generator.

The Lua code generator has the advantage of being more simple and more safe as the code is parsed directly by LuaJIT, ensuring from the beginning complete compatibility of the bytecode.

Currently the Lua Code Generator backend does not preserve the line numbers of the original source code. This is meant to be fixed in the future.

Use this backend instead of the bytecode generator if you prefer to have a more safe backend to convert the Lua AST to code.
The module can also be used for pretty-printing a Lua AST, since the code itself is probably the most human readable representation of the AST.

C API
---

The language toolkit provides a very simple set of C APIs to implement a custom language.
The functions provided by the C API are:

```c
/* The functions above are the equivalent of the luaL_* corresponding
   functions. */
extern int language_init(lua_State *L);
extern int language_report(lua_State *L, int status);
extern int language_loadbuffer(lua_State *L, const char *buff, size_t sz, const char *name);
extern int language_loadfile(lua_State *L, const char *filename);


/* This function push on the stack a Lua table with the functions:
   loadstring, loadfile, dofile and loader.
   The first three function can replace the Lua functions while the
   last one, loader, can be used as a customized "loader" function for
   the "require" function. */
extern int luaopen_langloaders(lua_State *L);

/* OPTIONAL:
   Load into package.preload lang.* modules using embedded bytecode. */
extern void language_bc_preload(lua_State *L)
```

The functions above can be used to create a custom LuaJIT executable that use the language toolkit implementation.

When the function `language_*` is used, an independent `lua_State` is created behind the scenes and used to compile the bytecode.
Once the bytecode is generated it is loaded into the user's `lua_State` ready to be executed.
The approach of using a separate Lua's state ensure that the process of compiling does not interfere with the user's application.

The function `language_bc_preload` is useful to create a standalone executable that does not depend on the presence of the Lua files at runtime.
The `lang.*` are compiled into bytecode and stored as static C data into the executable.
By calling the function `language_bc_preload` all the modules are *preloaded* using the embedded bytecode.
This feature can be disabled by changing the `BC_PRELOAD` variable in `src/Makefile`.

Running the Application
---

The application can be run with the following command:

```
luajit run.lua [lua-options] <filename>
```

The "run.lua" script will just invoke the complete pipeline of the lexer, parser and bytecode generator and it will pass the bytecode to luajit with "loadstring".

The language toolkit also provides a customized executable named `luajit-x` that uses the language toolkit's pipeline instead of the native one.
Otherwise, the program `luajit-x` works exactly the same as `luajit` itself, and accepts the same options.

In the standard build `luajit-x` will contain the `lang.*` modules as embedded bytecode data so that it does not rely on the Lua files at runtime.

This means that you can experiment with the language by modifying the Lua implementation of the language and test the changes immediately.
If the option `BC_PRELOAD` in `src/Makefile` is activated you just need to recompile `luajit-x`.

If you works with the Lua files of the language toolkit you may choose to disable the `BC_PRELOAD` variable to avoid recompiling the executable for each change in the Lua code.

### Generated Bytecode ###

You can inspect the bytecode generated by the language toolkit by using the "-b" options.
They can be invoked either with standard luajit by using "run.lua" or directly using the customized program `luajit-x`.

For example you can inspect the bytecode using the following command:

```
luajit run.lua -bl tests/test-1.lua
```

or alternatively:

```
./src/luajit-x -bl tests/test-1.lua
```

where we suppose that you are running `luajit-x` from the language toolkit's root directory.

Either way, when you use one of the two commands above to generate the bytecode you will the see following on the screen:

```
-- BYTECODE -- "test-1.lua":0-7
00001    TNEW     0   0
0002    KSHORT   1   1
0003    KSHORT   2  10
0004    KSHORT   3   1
0005    FORI     1 => 0010
0006 => MULVV    5   4   4
0007    ADDVN    5   5   0  ; 1
0008    TSETV    5   0   4
0009    FORL     1 => 0006
0010 => KSHORT   1   1
0011    KSHORT   2  10
0012    KSHORT   3   1
0013    FORI     1 => 0018
0014 => GGET     5   0      ; "print"
0015    TGETV    6   0   4
0016    CALL     5   1   2
0017    FORL     1 => 0014
0018 => RET0     0   1
```

You can compare it with the bytecode generated natively by LuaJIT using the command:

```
luajit -bl tests/test-1.lua
```

In the example above the generated bytecode will be *identical* to that generated by LuaJIT.
This is not an accident, since the Language Toolkit's bytecode generator is designed to produce the same bytecode that LuaJIT itself would generate.
In some cases, the generated code will differ. But, this is not considered a big problem as long as the generated code is still semantically correct.

### Bytecode Annotated Dump ###

In addition to the standard LuaJIT bytecode functions, the language toolkit also supports a special debug mode where the bytecode is printed byte-by-byte in hex format with some annotations on the right side of the screen.
The annotations will explain the meaning of each chunk of bytes and decode them as appropriate.

For example:

```
luajit run.lua -bx tests/test-1.lua
```

will display something like:

```
1b 4c 4a 01             | Header LuaJIT 2.0 BC
00                      | Flags: None
11 40 74 65 73 74 73 2f | Chunkname: @tests/test-1.lua
74 65 73 74 2d 31 2e 6c |
75 61                   |
                        | .. prototype ..
8a 01                   | prototype length 138
02                      | prototype flags PROTO_VARARG
00                      | parameters number 0
07                      | framesize 7
00 01 01 12             | size uv: 0 kgc: 1 kn: 1 bc: 19
31                      | debug size 49
00 07                   | firstline: 0 numline: 7
                        | .. bytecode ..
32 00 00 00             | 0001    TNEW     0   0
27 01 01 00             | 0002    KSHORT   1   1
27 02 0a 00             | 0003    KSHORT   2  10
27 03 01 00             | 0004    KSHORT   3   1
49 01 04 80             | 0005    FORI     1 => 0010
20 05 04 04             | 0006 => MULVV    5   4   4
14 05 00 05             | 0007    ADDVN    5   5   0  ; 1
39 05 04 00             | 0008    TSETV    5   0   4
4b 01 fc 7f             | 0009    FORL     1 => 0006
27 01 01 00             | 0010 => KSHORT   1   1
27 02 0a 00             | 0011    KSHORT   2  10
27 03 01 00             | 0012    KSHORT   3   1
49 01 04 80             | 0013    FORI     1 => 0018
34 05 00 00             | 0014 => GGET     5   0      ; "print"
36 06 04 00             | 0015    TGETV    6   0   4
3e 05 02 01             | 0016    CALL     5   1   2
4b 01 fc 7f             | 0017    FORL     1 => 0014
47 00 01 00             | 0018 => RET0     0   1
                        | .. uv ..
                        | .. kgc ..
0a 70 72 69 6e 74       | kgc: "print"
                        | .. knum ..
02                      | knum int: 1
                        | .. debug ..
01                      | pc001: line 1
02                      | pc002: line 2
02                      | pc003: line 2
02                      | pc004: line 2
02                      | pc005: line 2
...
```

This kind of output is especially useful for debugging the language toolkit itself because it does account for every byte of the bytecode and include all the sections of the bytecode.
For example, you will be able to inspect the `kgc` or `knum` sections where the prototype's constants are stored.
The output will also include the debug section in decoded form so that it can be easily inspected.

Current Status
---

Currently LuaJIT Language Toolkit should be considered as beta software.

The implementation is now complete in term of features and well tested, even for the most complex cases, and a complete test suite is used to verify the correctness of the generated bytecode.

The language toolkit is currently capable of executing itself.
This means that the language toolkit is able to correctly compile and load all of its module and execute them correctly.

Yet some bugs are probably present and you should be cautious when you use LuaJIT language toolkit.