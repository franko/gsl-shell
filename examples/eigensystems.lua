
local function vandermonde(x)
   local n = #x
   return new(n, n, |i,j| x[i]^(n-j))
end


function demo1()
   local m = new(4, 4, |i,j| 1/(i+j-1))
   print(m)
   local e, v = eigsv(m)
   print('Eigenvalues:', ilist(|i| e[i], 4))
   print('Eigenvectors:')
   print(v)
   -- the following expression will give a diagonal matrix with the eigenvalues
   --- along the diagonal
   print(prod(v,mul(m,v)))
end

function demo2()
   local m = vandermonde {-1, -2, 3, 4}
   print(m)
   local e, v = eignsv(m)
   print('Eigenvalues:', ilist(|i| e[i], 4))
   print('Eigenvectors:')
   print(v)
   -- the following expression will give a diagonal matrix with the eigenvalues
   --- along the diagonal
   print(mul(inv(v),m,v))
end
