
local function vandermonde(x)
   local n = #x
   return new(n, n, |i,j| x[i]^(n-j))
end


function demo1()
   local m = new(4, 4, |i,j| 1/(i+j-1))
   echo 'Matrix:'
   print(m)
   local e, v = eigsv(m)
   echo('Eigenvalues:')
   print(ilist(|i| e[i], 4))

   -- the following expression will give a diagonal matrix with the eigenvalues
   --- along the diagonal
   echo('Matrix diagonal form:')
   print(prod(v, m*v))
end

function demo2()
   local m = vandermonde {-1, -2, 3, 4}
   echo 'Matrix:'
   print(m)

   local e, v = eignsv(m)
   echo 'Eigenvalues:'
   print(ilist(|i| e[i], 4))

   -- the following expression will give a diagonal matrix with the eigenvalues
   --- along the diagonal
   echo('Matrix diagonal form:')
   print(inv(v) * m * v)
end

echo([[
demo1() - example of eigensystem solving for a real symmetrix matrix
demo2() - example of eigensystem solving for a real non-symmetrix matrix]])
