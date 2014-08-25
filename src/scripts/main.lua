local libos = require "libos"
local ejresource = require "ejresource"
local ejpackage = require "ejpackage"
local utils = require "utils"

local usage = [[
Usage: simplepacker inputdir [-o path] [-ni] [-ps packsize] [-na] [-raw] [-v]
  -o: specify output directory
  -ni: no image, ignore raw image files(png) from the input
  -ps: specify image sheet size, up to 2048. default value is 1024
  -na: no animation, ignore animation description files(.a.lua)
  -raw: write down all data instead of exporting a ejoy2d package
        output folder can used as the input to create ejoy2d package later
  -v: show verbose log
]]

-- configuration
local config = {
	proc_img = true,  -- whether to read raw image
	proc_anim = true,  -- whether to read anim
	output_path = false,
	pack_size = 1024,
	output_raw = false,  -- true for write down all data, false for export ejoy2d package
}

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
		print("require input path")
		return false
	end

	local i = 3
	while i <= #args do
		local arg = args[i]
		if arg == "-o" then
			local op = args[i + 1]
			if op[1] == "-" then
				print("illegal output path")
				return false
			end
			config.output_path = op
			i = i + 1
		elseif arg == "-ni" then
			config.proc_img = false
		elseif arg == "-ps" then
			local ps = tonumber(args[i + 1])
			if ps <= 0 or ps > 2048 then
				print("illegal pack size")
				return false
			end
			config.pack_size = ps
			i = i + 1
		elseif arg == "-na" then
			config.proc_anim = false
		elseif arg == "-raw" then
			config.output_raw = true
		elseif arg == "-v" then
			utils:enable_log()
		else
			print("illegal argument", arg)
			return false
		end
		i = i + 1
	end

	return true
end

local function _check_anims(imgs)
	local anims = {}

	table.sort(imgs, function (a, b)
			return a.name < b.name
		end)

	local i = 1
	while i < #imgs do
		local _,_,name = string.find(imgs[i].name, "(%a+)1")
		if name then
			local idx = 2
			local found
			repeat
				found = false
				i = i + 1
				if imgs[i].name == name..tostring(idx) then
					found = true
					idx = idx + 1
				end
			until not found

			if idx > 2 then
				local anim = ejresource:new_anim(name)
				local frames = {}
				for j=1,idx-1 do
					table.insert(frames, {{pic=name..tostring(j)}})
				end
				anim:add_action(frames)
				table.insert(anims, anim)
				utils:logf("auto create anim %s(%d)", name, idx-1)
			end
		else
			i = i + 1
		end
	end

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
	if config.output_path then
		output = _trim_slash(config.output_path)
	end
	libos:makedir(output)

	-- walk input folder
	local file_list = libos.walkdir(input)
	if not file_list then
		utils:logf("error input path")
		print(usage)
		return
	end

	-- process input files
	local all_imgs = {}  -- raw images, only png supported
	local all_sheets = {}  -- iamge sheets
	local all_anims = {}  -- animation description

	for _,v in ipairs(file_list) do
		local full_name = input.."/"..v
		local name = string.sub(v, 1, string.find(v, ".", 1, true) - 1)

		if config.proc_img and _check_ext(v, ".png") then
			local img = ejresource:load_img(full_name, name)
			if img then
				utils:logf("load img %s success (%d,%d)", name, img.w, img.h)
				table.insert(all_imgs, img)
			else
				utils:logf("load img %s failed", name)
			end
		end

		if _check_ext(v, ".p.lua") then
			local sheet = ejresource:load_sheet(full_name)
			if sheet then
				utils:logf("load sheet %s success", name)
				table.insert(all_sheets, sheet)
			else
				utils:logf("load sheet %s failed", name)
			end
		end

		if config.proc_anim and _check_ext(v, ".a.lua") then
			local anim = ejresource:load_anim(full_name, name)
			if anim then
				utils:logf("load anim %s success", name)
				table.insert(all_anims, anim)
			else
				utils:logf("load anim %s failed", name)
			end
		end
	end

	-- guess anim from image filename
	if config.proc_anim then
		local _anims = _check_anims(all_imgs)
		for _,v in ipairs(_anims) do
			table.insert(all_anims, v)
		end
	end

	-- pack raw images onto image sheet
	if #all_imgs > 0 then
		local sheet_map = {}
		local left_imgs = {}

		for _,v in ipairs(all_imgs) do
			local sheet = sheet_map[v.pixfmt]
			if not sheet then
				sheet = ejresource:new_sheet(config.pack_size, v.pixfmt)
				sheet_map[v.pixfmt] = sheet
			end
			if not sheet:pack_img(v) then
				table.insert(left_imgs, v)
			end
		end

		for _,v in pairs(sheet_map) do
			table.insert(all_sheets, v)
		end

		all_imgs = left_imgs  -- too big for packing
	end

	-- output
	if config.output_raw then
		for _,v in ipairs(all_imgs) do
			local path = string.format("%s/%s", output, v.name)
			v:save(path, true)
		end

		for i,v in ipairs(all_sheets) do
			local path = string.format("%s/imagesheet%d", output, i)
			v:save(path, true)
		end

		for _,v in ipairs(all_anims) do
			local path = string.format("%s/%s.a.lua", output, v.name)
			v:save(path)
		end
	else
		local _,_,name = string.find(input, ".-([^\\/]+)$")
		local pkg = ejpackage:new_pkg(name)

		-- raw images
		for _,v in ipairs(all_imgs) do
			pkg:add_img(v)
		end

		-- packed images
		for _,v in ipairs(all_sheets) do
			pkg:add_sheet(v)
		end

		-- anims
		for _,v in ipairs(all_anims) do
			pkg:add_anim(v)
		end

		-- save to disk
		pkg:save(output)
	end
end