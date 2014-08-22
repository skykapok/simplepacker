local libos = require "libos"

-- file templates
local TEMPLATE_BODY = [[
return {

%s
}
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

local TEMPLATE_ANIMATION = [[
{
	type = "animation",
	id = %d,
	export = "%s",
	component = {
%s	},
%s
},
]]

-- ejoy2d package
local pkg_mt = {}
pkg_mt.__index = pkg_mt

function pkg_mt:add_img(img)
end

function pkg_mt:add_sheet(sheet)
	table.insert(self.sheets, sheet)
	for k,v in pairs(sheet.imgs) do
		local item = {}
		item.type = "picture"
		item.id = self:_next_id()
		item.data = {#self.sheets, v[1], v[2], k.w, k.h}  -- texid, x, y, w, h
		self.items[k.name] = item
	end
end

function pkg_mt:add_anim(anim)  -- image name sequence
	local item = {}
	item.type = "animation"
	item.id = self:_next_id()
	item.data = anim.actions  -- frames
	self.items[anim.name] = item
end

function pkg_mt:save(path)
	-- save image sheet
	for i,v in ipairs(self.sheets) do
		local picture_path = string.format("%s/%s.%d", path, self.name, i)
		v:save(picture_path, false)
	end

	-- save description file
	local body = ""
	for k,v in pairs(self.items) do
		if v.type == "picture" then
			body = body..self:_serialize_picture(v.id, k, v.data)
		end
	end
	for k,v in pairs(self.items) do
		if v.type == "animation" then
			body = body..self:_serialize_animation(v.id, k, v.data)
		end
	end

	local all = string.format(TEMPLATE_BODY, body)
	local lua_path = string.format("%s/%s.lua", path, self.name)
	libos:writefile(lua_path, all)
end

function pkg_mt:_serialize_picture(id, name, data)
	local tex = data[1]

	local sl = data[2]
	local sr = data[2] + data[4]
	local st = data[3]
	local sb = data[3] + data[5]

	local dl = -data[4]*8  -- left = -w/2 * 16
	local dr = data[4]*8
	local dt = -data[5]*8
	local db = data[5]*8

	return string.format(TEMPLATE_PICTURE, id, name, tex,
		sl, st, sl, sb, sr, sb, sr, st,
		dl, dt, dl, db, dr, db, dr, dt)
end

function pkg_mt:_serialize_animation(id, name, data)
	local com2idx = {}
	local idx2com = {}

	local str_a = ""  -- action section

	for _,action in ipairs(data) do
		str_a = str_a.."\t{\n"  -- start a new action
		for __,frame in ipairs(action) do
			local com_list = {}
			for ___,component in ipairs(frame) do
				if not com2idx[component] then
					table.insert(idx2com, component)
					com2idx[component] = #idx2com - 1  -- idx base 0
				end
				table.insert(com_list, com2idx[component])
			end
			local str_f = "\t\t{ "  -- start a new frame line
			for i,v in ipairs(com_list) do
				str_f = str_f..tostring(v)..", "  -- append component on this line
			end
			str_f = str_f.."},\n"  -- close frame line
			str_a = str_a..str_f  -- add frame to action, one frame one line
		end
		str_a = str_a.."\t},"  -- close action
	end

	local str_c = ""  -- component section
	for _,component in ipairs(idx2com) do
		local item = self.items[component]
		str_c = string.format("%s\t\t{ id = %d },\n", str_c, item.id)  -- one component one line
	end

	return string.format(TEMPLATE_ANIMATION, id, name, str_c, str_a)
end

function pkg_mt:_next_id()
	local ret = self._id
	self._id = self._id + 1
	return ret
end

-- ejoy2d package module
local M = {}

function M:new_pkg(name)
	local pkg = {}
	pkg._id = 0
	pkg.name = name
	pkg.sheets = {}
	pkg.items = {}  -- name:item
	return setmetatable(pkg, pkg_mt)
end

return M