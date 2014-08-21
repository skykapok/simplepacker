local libos = require "libos"
local image = require "image"
local ejpackage = require "ejpackage"

local usage = [[
Usage: simplepacker inputdir [-o path] [-ni] [-np] [-ps packsize] [-na] [-mid] [-v]
	-o: specify output path
	-ni: no image, ignore raw image files
	-np: no pack, do not pack raw images to image sheet
	-ps: specify image sheet pack size, up to 2048. default value is 1024
	-na: no animation, ignore animation description files(.a.lua)
	-mid: write all data instead of ejoy2d package
	      output folder can used as the input to create ejoy2d package
	-v: show verbose info
]]

-- arguments
local g_mid = false  -- true for write all data, false for write ejoy2d package
local g_rimg = true  -- whether to read raw image
local g_pack = true  -- whether to pack raw iamges
local g_anim = true  -- whether to read anim

local g_output_path = false
local g_pack_size = 1024

-- log
local function _logf(...)
	print("[LUA] "..string.format(...))
end
local logf = function (...) end

-- helpers
local function _check_ext(file_name, ext)
	return string.find(file_name, ext, 1, true) == #file_name - #ext + 1
end

local function _trim_slash(path)
	if path[#path] == "\\" or path[#path] == "/" then
		return string.sub(path, 1, -2)
	else
		return path
	end
end

-- functions
local function _parse_args(args)
	if not args[2] then  -- check input path
		return false
	end

	local i = 3
	while i <= #args do
		local arg = args[i]
		if arg == "-o" then
			g_output_path = args[i + 1]
			if g_output_path[1] == "-" then
				return false
			end
			i = i + 1
		elseif arg == "-ni" then
			g_rimg = false
		elseif arg == "-np" then
			g_pack = false
		elseif arg == "-ps" then
			g_pack_size = tonumber(args[i + 1])
			if g_pack_size <= 0 or g_pack_size > 2048 then
				return false
			end
			i = i + 1
		elseif arg == "-na" then
			g_anim = false
		elseif arg == "-mid" then
			g_mid = true
		elseif arg == "-v" then
			logf = _logf
		else
			return false
		end
	end

	return true
end

local function _check_anims(imgs)
	local anims = {}
	-- TODO
	return anims
end

-- entry point
function run(args)
	-- init arguments
	if not _parse_args(args) then
		print(usage)
		return
	end

	-- init work path
	local input = _trim_slash(args[2])
	local output = input.."_out"
	if g_output_path then
		output = _trim_slash(g_output_path)
	end
	libos:makedir(output)

	-- walk input folder
	local file_list = libos.walkdir(input)
	if not file_list then
		logf("error input path")
		print(usage)
		return
	end

	-- process input files
	local imgs = {}  -- raw images, only png supported
	local sheets = {}  -- iamgesheets
	local anims = {}  -- animation description

	for _,v in ipairs(file_list) do
		if g_rimg and _check_ext(v, ".png") then
			local name = string.sub(v, 1, -5)
			local img = image:load_img(input.."/"..v, name)
			if img then
				logf("load img %s success (%d,%d)", name, img.w, img.h)
				table.insert(imgs, img)
			else
				logf("load img %s failed", name)
			end
		end

		if _check_ext(v, ".p.lua") then
			-- TODO
		end

		if g_anim and _check_ext(v, ".a.lua") then
			-- TODO
		end
	end

	-- guess anim from image filename
	if g_anim and #imgs > 0 then
		local _anims = _check_anims(imgs)
		for _,v in ipairs(_anims) do
			table.insert(anims, v)
		end
	end

	-- pack raw images onto imagesheet
	if g_pack and #imgs > 0 then
		local sheet_map = {}

		for _,v in ipairs(imgs) do
			local sheet = sheet_map[v.pixfmt]
			if not sheet then
				sheet = image:new_sheet(g_pack_size, v.pixfmt)
				sheet_map[v.pixfmt] = sheet
			end
			sheet:pack_img(v)
		end

		for _,v in pairs(sheet_map) do
			table.insert(sheets, v)
		end

		imgs = {}
	end

	-- output
	if g_mid then
		for _,v in ipairs(imgs) do
			v:save(output.."/"..v.name)
		end

		for i,v in ipairs(sheets) do
			v:save(output.."/imagesheet"..tostring(i))
		end
	else
		local pkg = ejpackage:new_pkg("output")

		-- raw images
		for _,v in ipairs(imgs) do
			pkg:add_img(v)
		end

		-- packed images
		for _,v in ipairs(sheets) do
			pkg:add_sheet(v)
		end

		-- anims
		for _,v in ipairs(anims) do
			pkg:add_anim(v)
		end

		-- save to disk
		pkg:save(output)
	end
end