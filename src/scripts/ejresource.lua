local libimage = require "libimage"
local libos = require "libos"
local binpacking = require "binpacking"

local TEMPLATE_IMAGE = [[
	{ name="%s", pos={%d,%d}, size={%d,%d} },
]]

-- image
local img_mt = {}
img_mt.__index = img_mt

function img_mt:save(path, desc)  -- ensure no file ext in path string
	libimage:saveppm(path, self.w, self.h, self.pixfmt, self.buf)
	if desc then  --generate a description file as image sheet
		local lua_path = path..".p.lua"
		local body = "return {\n\n"
		body = body..string.format(TEMPLATE_IMAGE, self.name, 0, 0, self.w, self.h)
		body = body.."\n}"
		libos:writefile(lua_path, body)
	end
end

-- image sheet
local sheet_mt = {}
sheet_mt.__index = sheet_mt

function sheet_mt:pack_img(img)
	assert(self.bin)

	local rect = self.bin:insert(img.w + 2, img.h + 2)  -- 2 empty pixel between images
	if rect then
		local x = rect.x + 1
		local y = rect.y + 1
		libimage:blitimg(self.buf, self.size, self.pixfmt, img.buf, img.w, img.h, x, y)
		local info = {name=img.name, pos={x,y}, size={img.w,img.h}}
		table.insert(self.imgs, info)
		return true
	end

	return false
end

function sheet_mt:save(path, desc)  -- ensure no file ext in path string
	libimage:saveppm(path, self.size, self.size, self.pixfmt, self.buf)
	if desc then
		local lua_path = path..".p.lua"
		local body = "return {\n\n"
		for _,v in ipairs(self.imgs) do
			body = body..string.format(TEMPLATE_IMAGE, v.name, v.pos[1], v.pos[2], v.size[1], v.size[2])
		end
		body = body.."\n}"
		libos:writefile(lua_path, body)
	end
end

-- animations
local anim_mt = {}
anim_mt.__index = anim_mt

function anim_mt:add_action(frames)
	table.insert(self.actions, frames)
end

function anim_mt:save(path)
	local body = "return {\n\n"
	for _,act in ipairs(self.actions) do
		body = body.."{\n"
		for _,frm in ipairs(act) do
			body = body.."\t{ "
			for _,com in ipairs(frm) do
				body = body..string.format("{ name=\"%s\", ", com.name)
				if com.scale then
					body = body..string.format("scale={%d,%d}, ", com.scale[1], com.scale[2])
				end
				if com.rot then
					body = body..string.format("rot=%d, ", com.rot)
				end
				if com.trans then
					body = body..string.format("trans={%d,%d}, ", com.trans[1], com.trans[2])
				end
				if com.color then
					body = body..string.format("color=0x%08x, ", com.color)
				end
				if com.additive then
					body = body..string.format("additive=0x%08x, ", com.additive)
				end
				body = body.."}, "
			end
			body = body.."},\n"
		end
		body = body.."},\n"
	end
	body = body.."\n}"
	libos:writefile(path, body)
end

-- ejoy2d resource module
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
	sheet.buf = libimage:newimg(size, pixfmt)
	sheet.pixfmt = pixfmt
	sheet.size = size
	sheet.imgs = {}
	sheet.bin = binpacking:new_bin(size, size)
	return setmetatable(sheet, sheet_mt)
end

function M:load_sheet(path)
	local buf, pixfmt, w, h = libimage:loadppm(string.sub(path, 1, -7))
	if buf then
		local sheet = {}
		sheet.buf = buf
		sheet.pixfmt = pixfmt
		sheet.size = w
		sheet.imgs = dofile(path)
		sheet.bin = false  -- loaded sheet cannot pack in new image
		return setmetatable(sheet, sheet_mt)
	end
end

function M:new_anim(name)
	local anim = {}
	anim.name = name
	anim.actions = {}
	return setmetatable(anim, anim_mt)
end

function M:load_anim(path, name)
	local anim = {}
	anim.name = name
	anim.actions = dofile(path)
	return setmetatable(anim, anim_mt)
end

return M