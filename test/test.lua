local ej = require "ejoy2d"
local fw = require "ejoy2d.framework"
local pack = require "ejoy2d.simplepackage"

pack.load {
	pattern = fw.WorkDir..[[examples/asset/?]],
    "input",
	"input_advance",
}

local idle = ej.sprite("input", "idle")
local die = ej.sprite("input", "die")
local stone = ej.sprite("input", "stone")
idle:ps(100, 300)
die:ps(100, 300)
die.visible = false

local blink = ej.sprite("input_advance", "actor_anim")
blink:ps(240, 300)

local y = 100

local game = {}

function game.update()
    if stone.visible and y < 200 then
        y = y + 5
        stone:ps(100, y)
        if y >= 200 then
            stone.visible = false
            idle.visible = false
            die.visible = true
        end
    end

    if die.visible and die.frame < 4 then
        die.frame = die.frame + 1
    end

	blink.frame = blink.frame + 1
end

function game.drawframe()
	ej.clear(0xff808080)

    idle:draw()
    die:draw()
    stone:draw()
	blink:draw()
end

function game.touch(what, x, y)
end

function game.message(...)
end

function game.handle_error(...)
end

function game.on_resume()
end

function game.on_pause()
end

ej.start(game)