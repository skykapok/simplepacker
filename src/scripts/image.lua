local libimage = require "libimage"
local libos = require "libos"
local binpacking = require "binpacking"

-- image
local img_mt = {}
img_mt.__index = img_mt

function img_mt:save(path, desc)  -- ensure no file ext in path string
	libimage:saveppm(path, self.w, self.h, self.pixfmt, self.buf)
	if desc then
		-- TODO
	end
end

-- packed image sheet
local sheet_mt = {}
sheet_mt.__index = sheet_mt

function sheet_mt:pack_img(img)
	assert(self.imgs[img] == nil)

	local rect = self.bin:insert(img.w + 2, img.h + 2)  -- 2 empty pixel between images
	if rect then
		self.imgs[img] = {rect.x + 1, rect.y + 1}
		return true
	end

	return false
end

function sheet_mt:save(path, desc)  -- ensure no file ext in path string
	-- merge images onto a large texture
	local buf = libimage:newimg(self.size, self.pixfmt)
	for k,v in pairs(self.imgs) do
		libimage:mergeimg(buf, self.size, self.pixfmt, k.buf, k.w, k.h, v[1], v[2])
	end

	-- save the large merged texture
	libimage:saveppm(path, self.size, self.size, self.pixfmt, buf)

	if desc then
		-- TODO
	end
end

-- libimage module
local M = {}

function M:load_img(path, name)
	local buf, pixfmt, w, h = libimage:loadpng(path)
	if buf then
		local img = {}
		img.buf = buf  -- raw memory for pixel data
		img.pixfmt = pixfmt  -- pixel format string
		img.w = w
		img.h = h
		img.name = name
		return setmetatable(img, img_mt)
	end
end

function M:new_sheet(size, pixfmt)
	local sheet = {}
	sheet.size = size or 1024
	sheet.pixfmt = pixfmt or "RGBA"
	sheet.imgs = {}
	sheet.bin = binpacking:new_bin(sheet.size, sheet.size)
	return setmetatable(sheet, sheet_mt)
end

return M