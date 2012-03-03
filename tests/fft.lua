use'num'
local t = {}

local y = function(n) return matrix.new(n, 1, |i| i < n/3 and 0 or (i < 2*n/3 and 1 or 0)) end

t.fft = function(test) test{fft(y(64))} end
t.fft_id = function(test) test{tostring(fftinv(fft(y(1024))))==tostring(y(1024)) } end
return t
