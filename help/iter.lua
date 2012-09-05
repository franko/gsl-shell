local M = {

  [iter.sequence] = [[
iter.sequence(f, a, b)
iter.sequence(f, b)

   Return an iterator that gives the value (or the values) returned
   by the evaluation of "f(i)" where "i" is an integer that goes
   from "a" to "b". In the second form, the generated values start
   from one.
]],

  [iter.sample] = [[
iter.sample(f, xi, xs, n)

   Return an iterator that gives the couple "x, f(x)" for "x" going
   from "xi" to "xs" with "n" uniformly spaced intervals. If "f"
   returns multiple values, only the first one is retained.
]],

  [iter.isample] = [[
iter.isample(f, a, b)
iter.isample(f, b)

   Return an iterator that gives the couple "i, f(i)" where "i" is an
   integer going from "a" to "b". In the second form, the sequence
   will start from one. If "f" returns multiple values, only the first
   one is retained.
]],

  [iter.ilist] = [[
iter.ilist(f, a, b)
iter.ilist(f, b)

   Returns a list with the elements "f(i)" where "i" is an integer
   going from a to b. In the second form, the sequence will start from
   one.
]],

  [iter.isum] = [[
iter.isum(f, a, b)
iter.isum(f, b)

   Returns the sum of "f(i)" for all integers "i" from a to b. In the
   second form, the sequence will start from one.
]]
}

return M
