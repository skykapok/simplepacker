local libimage = require "libimage"
local binpacking = require "binpacking"

local PIXEL_FMT_RGB = 0
local PIXEL_FMT_RGBA = 1

-- image
local img_mt = {}
img_mt.__index = img_mt

function img_mt:save(path)
	libimage:saveppm(self.w, self.h, self.pixfmt, self.buf)
end

-- packed large image
local pimg_mt = {}
pimg_mt.__index = pimg_mt

function pimg_mt:save(path)
	local buf = libimage:mergeimg(self.imgs)
	libimage:saveppm(self.w, self.h, self.pixfmt, buf)
	self:_write_desc(path)
end

function pimg_mt:_write_desc(path)
end

-- libimage module
local M = {}

function M:load_img(path)
	local buf, pixfmt, w, h = libimage:loadpng(path)
	if buf then
		local img = {}
		img.buf = buf  -- raw memory for pixel data
		img.pixfmt = pixfmt
		img.w = w
		img.h = h
		return setmetatable(img, img_mt)
	end
end

function M:new_pimg(size, pixfmt)
	local pimg = {}
	pimg.w = size or 1024
	pimg.h = size or 1024
	pimg.pixfmt = pixfmt or PIXEL_FMT_RGBA
	pimg.imgs = {}
	pimg.bin = binpacking:new_bin(pimg.w, pimg.h)
	return setmetatable(pimg, pimg_mt)
end

return M