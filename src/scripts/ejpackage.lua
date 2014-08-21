local libos = require "libos"

-- package description file template
local TEMPLATE_BODY = [[
return {
%s}
]]

local TEMPLATE_PICTURE = [[
{
	type = "picture",
	id = %d,
	export = "%s",
	{
		tex = %d,
		src = { %d, %d, %d, %d, %d, %d, %d, %d },
		screen = { %d, %d, %d, %d, %d, %d, %d, %d },
	},
},
]]

-- ejoy2d package
local pkg_mt = {}
pkg_mt.__index = pkg_mt

function pkg_mt:add_img(img)
end

function pkg_mt:add_sheet(sheet)
	table.insert(self.pictures, sheet)
	for k,v in pairs(sheet.imgs) do
		self:_add_picture(#self.pictures, v[1], v[2], k.w, k.h, k.name)
	end
end

function pkg_mt:add_anim(anim)
end

function pkg_mt:save(path)
	for i,v in ipairs(self.pictures) do
		local picture_path = string.format("%s/%s.%d", path, self.name, i)
		v:save(picture_path)
	end

	local body = ""
	for i,v in ipairs(self.items) do
		if v.type == "picture" then
			local name = v.data[6]
			local tex = v.data[1]

			local sl = v.data[2]
			local sr = v.data[2] + v.data[4]
			local st = v.data[3]
			local sb = v.data[3] + v.data[5]

			local dl = -v.data[4]*8  -- left = -w/2 * 16
			local dr = v.data[4]*8
			local dt = -v.data[5]*8
			local db = v.data[5]*8

			body = body..string.format(TEMPLATE_PICTURE, v.id, name, tex,
				sl, st, sl, sb, sr, sb, sr, st,
				dl, dt, dl, db, dr, db, dr, dt)
		end
	end

	local all = string.format(TEMPLATE_BODY, body)
	local lua_path = string.format("%s/%s.lua", path, self.name)
	libos:writefile(lua_path, all)
end

function pkg_mt:_add_picture(idx, x, y, w, h, name)
	local item = {}
	item.type = "picture"
	item.id = self:_next_id()
	item.data = {idx, x, y, w, h, name}
	table.insert(self.items, item)
end

function pkg_mt:_next_id()
	local ret = self.id
	self.id = self.id + 1
	return ret
end

-- package module
local M = {}

function M:new_pkg(name)
	local pkg = {}
	pkg.name = name
	pkg.id = 0
	pkg.pictures = {}
	pkg.items = {}
	return setmetatable(pkg, pkg_mt)
end

return M