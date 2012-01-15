-- monte-vegas.lua
-- 
-- Copyright (C) 2012 Lesley De Cruz
-- 
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 3 of the License, or (at
-- your option) any later version.
-- 
-- This program is distributed in the hope that it will be useful, but
-- WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
-- General Public License for more details.
-- 
-- You should have received a copy of the GNU General Public License
-- along with this program; if not, write to the Free Software
-- Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
-- 
-- This is an implementation of the adaptive Monte-Carlo algorithm "VEGAS"
-- of G. P. Lepage, originally described in J. Comp. Phys. 27, 192(1978).
-- The current version of the algorithm was described in the Cornell
-- preprint CLNS-80/447 of March, 1980.
-- 
-- Adapted from GSL, version 1.15
-- 
-- Original author: Michael J. Booth, 1996
-- Modified by: Brian Gough, 12/2000
-- Adapted for LuaJIT2 by: Lesley De Cruz, 2012 

local floor, min, max, random = math.floor, math.min, math.max, math.random
local modf, abs, sqrt, log  = math.modf, math.abs, math.sqrt, math.log

local mt={}

--- initialise a fresh vegas state
function mt:init(a,b)
  for i=1,self.dim do
    assert(a[i]<b[i],"lower bound should be smaller than upper bound")
    self.dx[i] = b[i]-a[i]
    self.volume = self.volume * self.dx[i]
    self.d[i] = {}
    self.xi[i]= {0,1}
  end
  return self
end

--- reset the distribution of the grid and the current box coordinates
function mt:reset_val_and_box()
  for i=1,self.dim do 
    self.box[i]=0
    for j=1,self.bins do
      self.d[i][j]=0
    end
  end
end

-- step through the box coordinates like
-- {0, 1},..., {0, boxes-1}, {1, 0}, {1, 1},..., {1, boxes-1}, ...
-- returns true when reaching {0,0} again
function mt:boxes_traversed()
  for i=self.dim,1,-1 do
    self.box[i] = ( (self.box[i] + 1) % self.boxes) 
    if self.box[i] ~= 0 then return false end
  end
  return true
end

-- return a random point from the box, weighted with bin_vol
function mt:random_point(a,rget)
  local vol = 1
  for i=1,self.dim do
    -- box[j] + ran gives the position in the box units, 
    -- while z is the position in bin units.
    local z = (( self.box[i] + rget() ) / self.boxes ) * self.bins + 1
    local k, loc = modf(z) -- int: bin index and fract: location inside bin 
    self.bin[i] = k
    local bin_width = self.xi[i][k+1] - self.xi[i][k]
    local y = self.xi[i][k] + loc * bin_width
    self.x[i]=a[i] + y * self.dx[i]
    vol = vol * bin_width
  end    
  return vol
end

-- keep track of the squared function value in each bin 
-- to later refine the grid
function mt:accumulate_distribution(fsq)
  for i=1,self.dim do 
    local bin = self.bin[i]
    self.d[i][bin] = self.d[i][bin] + fsq 
  end
end

-- clear the results, but keep the grid
-- done in stage 0 and 1 in GSL
function mt:clear_stage1()
  self.wtd_int_sum = 0
  self.sum_wgts = 0
  self.chi_sum = 0
  self.it_num = 1
  self.samples = 0
  self.chisq = 0
end

-- intelligently resize the old grid given the new number of bins
function mt:resize(bins)
  -- create new tables
  self.weight = {}		-- ratio of bin sizes (# bins)
  local pts_per_bin = self.bins / bins
  for i=1,self.dim do 
    local xin = {0}
    local xold,xnew,dw,j=0,0,0,2
    for k=2,self.bins+1 do
      dw = dw + 1
      xold, xnew =  xnew, self.xi[i][k]
      while dw > pts_per_bin do
        dw = dw - pts_per_bin
        xin[j] = xnew - (xnew - xold) * dw
        j = j + 1
      end
    end
    xin[bins+1] = 1
    self.xi[i] = xin
    self.d[i] = {} -- distribution (depends on function^2) (# dim*bins)
  end
  self.bins = bins
end

-- refine the grid based on accumulated stats in self.d.
function mt:refine()
  local d,bins = self.d,self.bins
  for i=1,self.dim do
    -- implements gs[i][j] = (gs[i][j-1]+gs[i][j]+gs[i][j+1])/3
    local oldg,newg = d[i][1],d[i][2]
    -- total grid value for dimension i
    local grid_tot_i = (oldg + newg) / 2 
    d[i][1] = grid_tot_i
    for j=2,bins-1 do
      oldg, newg, d[i][j] = newg, d[i][j+1],(oldg + newg + d[i][j+1]) / 3
      grid_tot_i = grid_tot_i + d[i][j]
    end
    d[i][bins] = (oldg + newg) / 2
    grid_tot_i = grid_tot_i + d[i][bins]
    
    local tot_weight = 0
    for j=1,bins do
      self.weight[j]=0.
      if d[i][j] > 0 then
        local invwt = grid_tot_i / d[i][j] -- kind of "inverse weight"
        -- damped change
        self.weight[j] = ((invwt - 1) / (invwt* log(invwt)))^self.alpha 
      end
      tot_weight = tot_weight + self.weight[j]
    end
    
    -- now determine the new bin boundaries
    local pts_per_bin = tot_weight / bins
    if pts_per_bin~=0 then -- don't update grid if tot_weight==0
      local xin = {0}
      local xold,xnew,dw,j = 0,0,0,2
      for k=1,bins do
        dw = dw + self.weight[k]
        xold, xnew = xnew, self.xi[i][k+1]
        while dw > pts_per_bin do
          dw = dw - pts_per_bin
          xin[j] = xnew - (xnew - xold) * dw / self.weight[k]
          j = j + 1
        end
      end
      xin[bins+1] = 1
      self.xi[i] = xin
    end
  end
end

-- determine the number of calls, bins, boxes etc. 
-- based on the requested number of calls
-- intelligently rebin the old grid
-- done in stage 0,1 and 2 in GSL
function mt:rebin_stage2(calls)
  local bins,boxes = self.bins_max, 1
  if self.mode ~= "importance_only" then
    -- shooting for 2 calls/box
    boxes = floor((calls/2)^(1/self.dim))
    self.mode="importance"
    if 2*boxes >= self.bins_max then
      -- if there are too many boxes, we switch to stratified sampling
      local box_per_bin = max(floor(boxes/self.bins_max),1)
      bins = min(floor(boxes/box_per_bin), self.bins_max)
      boxes = box_per_bin * bins
      self.mode = "stratified"
    end
  end
  
  local tot_boxes= boxes^self.dim
  self.calls_per_box = max(floor(calls/tot_boxes),2)
  calls = self.calls_per_box*tot_boxes
  -- x-space volume / avg number of calls per bin
  self.jac = self.volume * bins^self.dim / calls 
  self.boxes = boxes
  -- If the number of bins changes from the previous invocation, bins
  -- are expanded or contracted accordingly, while preserving bin 
  -- density
  if bins ~= self.bins then
    self:resize(bins)
  end  
end

--- run (self.iterations) integrations
function mt:integrate(f,a,rget)
  self.it_start = self.it_num
  local cum_int, cum_sig = 0,0
  for it=1,self.iterations do
    local intgrl = 0 -- integral for this iteration
    local tss = 0 -- total squared sum
    local calls_per_box = self.calls_per_box
    local jacbin = self.jac
    
    self.it_num = self.it_start + it
    self:reset_val_and_box()
    
    repeat
      local m,q = 0,0 -- first and second moment
      local f_sq_sum = 0
      for k=1,calls_per_box do 
        local bin_vol = self:random_point(a,rget)
        local fval = jacbin * bin_vol * f(self.x)
        
        -- incrementally calculate first (mean) and second moments 
        local d = fval - m
        m = m + d / (k)
        q = q + d*d * ((k-1)/k)
        if self.mode~="stratified" then
          self:accumulate_distribution(fval*fval)
        end
        
      end
      intgrl = intgrl + m * calls_per_box;
      f_sq_sum = q * calls_per_box;
      tss = tss + f_sq_sum;
      if self.mode=="stratified" then
        self:accumulate_distribution(f_sq_sum)
      end
    until self:boxes_traversed()
    
    -- Compute final results for this iteration
    -- Determine variance and weight
    local var, wgt = tss / (calls_per_box - 1), 0
    
    if var > 0 then 
      wgt = 1 / var
    elseif self.sum_wgts > 0 then 
      wgt = self.sum_wgts / self.samples
    end
    self.result = intgrl;
    self.sigma = sqrt(var);

    if wgt > 0 then
      local sum_wgts = self.sum_wgts
      local wtd_int_sum = self.wtd_int_sum
      local m = (sum_wgts > 0) and (wtd_int_sum / sum_wgts) or 0
      local q = intgrl - m
      
      -- update stats
      self.samples = self.samples + 1
      self.sum_wgts = sum_wgts + wgt
      self.wtd_int_sum = wtd_int_sum + intgrl * wgt
      self.chi_sum = self.chi_sum + intgrl * intgrl * wgt
      cum_int = self.wtd_int_sum / self.sum_wgts
      cum_sig = sqrt (1 / self.sum_wgts)

      if self.samples == 1 then 
        self.chisq = 0
      else
        self.chisq = self.chisq * (self.samples - 2)
        self.chisq = self.chisq + (wgt / (1 + (wgt / sum_wgts))) * q * q 
        self.chisq = self.chisq / (self.samples - 1)
      end
    else
      cum_int = cum_int + (intgrl - cum_int) / it
      cum_sig = 0
    end
    self:refine()
  end
  return cum_int, cum_sig;
end

-- corresponds to stage 0 in GSL
local function getnewstate(a,b) 
  local d=#a
  return setmetatable(
  
    {
      dim = d,		-- dimension of the integral
      bins_max = 50,	-- even integer, will be divided by two
      bins = 1,		-- number of bins
      boxes = 0,	-- number of boxes
      volume = 1,	-- volume of the integration domain
      
      dx = {}, 		-- ranges, delta x (# dim)
      weight = {},	-- ratio of bin sizes (# bins)
      x = {}, 		-- evaluate the function at x (# dim)
      bin = {},		-- current bin coordinates (integer) (# dim)
      box = {}, 	-- current box coordinates (integer) (# dim)
      d = {}, 		-- distribution (depends on function^2) (# dim*bins)
      xi = {}, 		-- bin boundaries, i.e. grid (# dim*(bins+1))

      -- control variables
      alpha = 1.5,	-- grid stiffness (for rebinning), typically between
                    -- 1 and 2 (higher is more adaptive, 0 is rigid)
      mode = "importance",
      iterations=5,

      -- intermediate results for an iteration
      result = 0,
      sigma = 0,
      
      -- intermediate results for an integrate(...)
      wtd_int_sum = 0, 	-- weighted sum of integrals of each iteration (numerator)
      sum_wgts = 0,	-- sum of weights (denominator)
      chi_sum = 0,	-- sum of squares of the integrals computed this run
      it_num = 1, 	-- current iteration
      it_start = 1, -- start iteration for this run
      samples = 0,	-- number of integrals computed this run
      chisq = 0,	  -- chi^2 for the integrals computed this run
      calls_per_box=2,
      
      },{__index = mt}):init(a,b)
end

--- perform VEGAS monte carlo integration of f
-- @param f function of an n-dimensional vector
-- @param a lower bound vector
-- @param b upper bound vector
-- @param calls number of function calls (will be rounded down to fit grid) (optional)
-- @param r random number generator (optional)
-- @param chidev deviation tolerance for the integrals' chi^2 value (optional)
-- 	  integration will be repeated until chi^2 < chidev
-- @return result the result of the integration
-- @return sigma the estimated error or standard deviation 
-- @return num_int the number of runs required to calculate the integral
-- @return run function to compute the integral again via run(calls)
local function monte_vegas(f, a, b, calls, r, chidev)
  calls = calls or 5e5
  local rget = r and function() return r:get() end or random
  chidev = chidev or 0.5
  local dim = #a
  assert(dim==#b,"number of dimensions of lower and upper bounds differ")
  local state = getnewstate(a,b)
  -- INTEGRATION 
  -- warmup
  state:clear_stage1() -- clear results
  state:rebin_stage2(1e4) -- intialise grid for 1e4 calls
  local result,sigma = state:integrate(f,a,rget)
  local n
  -- full (stage 1)
  local run = function (c)
    calls = c or calls
    n=0
    repeat
      state:clear_stage1() -- forget previous results, but not the grid
      state:rebin_stage2(calls/state.iterations) -- initialise grid for calls/iterations calls
      result,sigma = state:integrate(f,a,rget)
      n=n+1
    until abs(state.chisq - 1) < chidev
    return result,sigma,n
  end
  result, sigma, n = run(calls)
  return result, sigma, n, run
end

return monte_vegas
