-- container for rects
local bin_mt = {}
bin_mt.__index = bin_mt

function bin_mt:place_rect(w, h)
end

-- binpacking module
local M = {}

function M:pack_rect(w, h)
	return setmetatable({
		w = w,
		h = h,
		}, bin_mt)
end

return M