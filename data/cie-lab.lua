-- based on https://gist.github.com/bikz05/6fd21c812ef6ebac66e1
-- color_channel_linear provided by chatGPT
-- validated using http://colormine.org/convert/rgb-to-lab

local function color_channel_linear(c)
  return c < 0.04045 and c / 12.92 or ((c+0.055) / 1.055)^2.4
end

local function color_channel_linear_inv(d)
  return d < 0.0031308 and d * 12.92 or d^(1/2.4) * 1.055 - 0.055
end

local function rgb_to_linrgb(rgb)
  local linrgb = { }
  for i = 1, 3 do
    linrgb[i] = color_channel_linear(rgb[i] / 255)
  end
  return linrgb
end

local function linrgb_to_rgb(linrgb)
  local rgb = { }
  for i = 1, 3 do
    local c = 255 * color_channel_linear_inv(linrgb[i])
    local cr = math.floor(c + 0.5)
    rgb[i] = math.max(1, math.min(cr, 255))
  end
  return rgb
end

local bb = {
  { 0.412453, 0.357580, 0.180423 },
  { 0.212671, 0.715160, 0.072169 },
  { 0.019334, 0.119193, 0.950227 },
}

local bbinv = {
  {   3.0799349,  -1.5371515, -0.54278342 },
  { -0.92123418,     1.87599, 0.045244181 },
  { 0.052889682, -0.20404134,   1.1511517 },
}

local function linrgb_to_xyz(lrgb)
  local xyz = { }
  for i = 1, 3 do
    xyz[i] = bb[i][1] * lrgb[1] + bb[i][2] * lrgb[2] + bb[i][3] * lrgb[3]
  end
  xyz[1] = xyz[1] / 0.950456
  xyz[3] = xyz[3] / 1.088754
  return xyz
end

local function xyz_to_linrgb(xyz)
  local lrgb = { }
  for i = 1, 3 do
    lrgb[i] = bbinv[i][1] * xyz[1] + bbinv[i][2] * xyz[2] + bbinv[i][3] * xyz[3]
  end
  return lrgb
end

local function f(t)
  return t > 0.008856 and t^(1/3) or 7.787*t + 16/116
end

local function finv(y)
  return y > 0.20689303442296 and y^3 or (y - 16/116) / 7.787
end

local function xyz_to_Lab(xyz)
  return {
    --[[ L  ]] 116 * f(xyz[2]) - 16,
    --[[ a* ]] 500 * (f(xyz[1]) - f(xyz[2])),
    --[[ b* ]] 200 * (f(xyz[2]) - f(xyz[3])),
  }
end

local function Lab_to_xyz(Lab)
  local fY = (Lab[1] + 16) / 116
  return {
    finv(Lab[2] / 500 + fY),
    finv(fY),
    finv(fY - Lab[3] / 200),
  }
end

local function rgb_to_Lab(rgb)
  return xyz_to_Lab(linrgb_to_xyz(rgb_to_linrgb(rgb)))
end

local function Lab_to_rgb(Lab)
  return linrgb_to_rgb(xyz_to_linrgb(Lab_to_xyz(Lab)))
end

--[[
local rgb_test
rgb_test = {180, 12, 36}
print(rgb_test, rgb_to_Lab(rgb_test), 'EXPECT', {38.00993, 60.94216, 35.51627})
print(rgb_test, Lab_to_rgb(rgb_to_Lab(rgb_test)), 'EXPECT', rgb_test)

rgb_test = {36, 180, 12}
print(rgb_test, rgb_to_Lab(rgb_test), 'EXPECT', {64.19662484363941, -63.327377053061355, 62.914515592352146})
print(rgb_test, Lab_to_rgb(rgb_to_Lab(rgb_test)), 'EXPECT', rgb_test)
]]

return { rgb2lab = rgb_to_Lab, lab2rgb = Lab_to_rgb }
