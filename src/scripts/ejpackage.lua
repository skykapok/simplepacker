local pkg_mt = {}
pkg_mt.__index = pkg_mt

function pkg_mt:add_img(img)
end

function pkg_mt:add_anim(anim)
end

function pkg_mt:check_anims(imgs)
	local anims = {}
	return anims
end

function pkg_mt:save(path)
end

-- package module
local M = {}

function M:new_pkg()
	return setmetatable({}, pkg_mt)
end

return M