-- this file demonstrate advanced animation usage

local function _get_color(alpha)
	return alpha * (2^24) + 0xffffff  -- ejoy2d use ARGB color format
end

local blink_action = {action="blink"}
for i=0,255,5 do
	local component = {name="actor", color=_get_color(i)}
	local frame = {component}
	table.insert(blink_action, frame)
end
for i=255,0,-5 do
	local component = {name="actor", color=_get_color(i)}
	local frame = {component}
	table.insert(blink_action, frame)
end

return {

blink_action,

}