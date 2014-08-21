local libos = require "libos"
local image = require "image"
local binpacking = require "binpacking"
local ejpackage = require "ejpackage"

local usage = [[
Usage: simplepacker inputdir
	-o:
	-c:
	-r:
	-np:
	-nd:
	-na:
	-v:
]]

local g_step1 = true
local g_step2 = true
local g_pack = true
local g_desc = true
local g_anim = true

local function _logf(...)
	print("[LUA] "..string.format(...))
end

local function _check_ext(file_name, ext)
	return string.find(file_name, ext, 1, true) == #file_name - #ext + 1
end

function run(args)
	local logf = function(...) end
	logf = _logf  -- test

	local input = args[2]
	if not input then
		print(usage)
		return
	end
	if input[#input] == "\\" or input[#input] == "/" then
		input = string.sub(input, 1, -2)
	end
	local output = input.."_out"
	libos:makedir(output)

	local file_list = libos.walkdir(input)
	if not file_list then
		print(usage)
		return
	end

	local imgs = {}  -- raw images, only png supported
	local sheets = {}  -- iamgesheets
	local anims = {}  -- animation description

	for _,v in ipairs(file_list) do
		if g_step1 and _check_ext(v, ".png") then
			local img = image:load_img(input.."/"..v)
			img.name = string.sub(v, 1, -5)
			if img then
				table.insert(imgs, img)
			end
		end

		if g_step2 and _check_ext(v, ".p.lua") then
		end

		if g_step2 and g_anim and _check_ext(v, ".a.lua") then
		end
	end

	-- pack raw images onto imagesheet
	if g_pack and #imgs > 0 then
		local sheet = image:new_sheet()
		for _,v in ipairs(imgs) do
			sheet:pack_img(v)
		end
		table.insert(sheets, sheet)
	end

	if g_step2 then
		local pkg = ejpackage:new_pkg()

		-- guess anim from image filename
		if g_anim and #imgs > 0 then
			local _anims = pkg:check_anims(imgs)
			for _,v in ipairs(_anims) do
				table.insert(anims, v)
			end
		end

		-- packed images
		for _,v in ipairs(sheets) do
			pkg:add_img(v)
		end

		-- single images
		for _,v in ipairs(imgs) do
			pkg:add_img(v)
		end

		-- anims read from .a.lua file
		for _,v in ipairs(anims) do
			pkg:add_anim(v)
		end

		-- save to disk
		pkg:save(output)
	end

	-- for i,v in ipairs(imgs) do
	-- 	v:save(output.."/"..v.name)
	-- end

	for i,v in ipairs(sheets) do
		v:save(output.."/packed"..tostring(i))
	end
end