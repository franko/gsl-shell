-- based on https://gist.github.com/bikz05/6fd21c812ef6ebac66e1
-- c2linear provided by chatGPT
-- validated using http://colormine.org/convert/rgb-to-lab
-- or better with:
-- http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
-- Choosing Calc and "CIE Color Calculator"
-- Scale RGB: check, Ref White: D65, RGB Model: sRGB, gamma: sRGB

local function c2linear(c)
  return c < 0.04045 and c / 12.92 or ((c+0.055) / 1.055)^2.4
end

local function c2linear_inv(lc)
  return lc < 0.0031308 and lc * 12.92 or lc^(1/2.4) * 1.055 - 0.055
end

local function rgb_to_linrgb(r, g, b)
  return c2linear(r / 255), c2linear(g / 255), c2linear(b / 255)
end

-- Takes a single channel value from a linear RGB color and returns
-- its RGB channel value. Its basically the function c2linear_inv and
-- multiply by 255.
local function linear2c8(lc)
  local c = 255 * c2linear_inv(lc)
  local cr = math.floor(c + 0.5)
  return math.max(0, math.min(cr, 255))
end

local function linrgb_to_rgb(lr, lg, lb)
  return linear2c8(lr), linear2c8(lg), linear2c8(lb)
end

-- based on https://gist.github.com/bikz05/6fd21c812ef6ebac66e1
-- Corresponds to https://en.wikipedia.org/wiki/Illuminant_D65 but
-- normalized to 1 and with greater precision
-- D65_xyz_gist = { 0.950456, 1, 1.088754 }
-- Value from view-source:http://www.brucelindbloom.com/javascript/ColorConv.js
local D65_xyz = { 0.95047, 1, 1.08883 }

-- http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
-- RGB to XYZ [M] sRGB D65
local bb_sRGB_D65_lindbloom = {
  { 0.4124564,  0.3575761,  0.1804375 },
  { 0.2126729,  0.7151522,  0.0721750 },
  { 0.0193339,  0.1191920,  0.9503041 },
}

local bb = bb_sRGB_D65_lindbloom
local bbinv = matrix.inv(matrix.inv(matrix.diag(D65_xyz)) * matrix.def(bb))

local function linrgb_to_xyz(lr, lg, lb)
  local x = (bb[1][1] * lr + bb[1][2] * lg + bb[1][3] * lb) / D65_xyz[1]
  local y = (bb[2][1] * lr + bb[2][2] * lg + bb[2][3] * lb) / D65_xyz[2]
  local z = (bb[3][1] * lr + bb[3][2] * lg + bb[3][3] * lb) / D65_xyz[3]
  return x, y, z
end

local function xyz_to_linrgb(x, y, z)
  local lr = bbinv[1][1] * x + bbinv[1][2] * y + bbinv[1][3] * z
  local lg = bbinv[2][1] * x + bbinv[2][2] * y + bbinv[2][3] * z
  local lb = bbinv[3][1] * x + bbinv[3][2] * y + bbinv[3][3] * z
  return lr, lg, lb
end

local function f(t)
  return t > 0.008856 and t^(1/3) or 7.787*t + 16/116
end

local function finv(y)
  return y > 0.20689303442296 and y^3 or (y - 16/116) / 7.787
end

local function xyz_to_Lab(x, y, z)
  local L = 116 * f(y) - 16
  local a = 500 * (f(x) - f(y))
  local b = 200 * (f(y) - f(z))
  return L, a, b
end

local function Lab_to_xyz(L, a, b)
  local fy = (L + 16) / 116
  return finv(fy + a / 500), finv(fy), finv(fy - b / 200)
end

local function rgb_to_Lab(r, g, b)
  return xyz_to_Lab(linrgb_to_xyz(rgb_to_linrgb(r, g, b)))
end

local function Lab_to_rgb(L, a, b)
  return linrgb_to_rgb(xyz_to_linrgb(Lab_to_xyz(L, a, b)))
end

return { rgb2lab = rgb_to_Lab, lab2rgb = Lab_to_rgb }
