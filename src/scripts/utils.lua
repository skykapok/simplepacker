local M = {
	logf = function (...) end
}

function M:_logf(...)
	print("[SCRIPT] "..string.format(...))
end

function M:enable_log()
	self.logf = self._logf
end

function M:_matrix_identity()
	return {1, 0, 0, 1, 0, 0}
end

function M:_matrix_multiply(m1, m2)
	return {
		m1[1]*m2[1] + m1[2]*m2[3],
		m1[1]*m2[2] + m1[2]*m2[4],
		m1[3]*m2[1] + m1[4]*m2[3],
		m1[3]*m2[2] + m1[4]*m2[4],
		m1[5]*m2[1] + m1[6]*m2[3] + m2[5],
		m1[5]*m2[2] + m1[6]*m2[4] + m2[6],
	}
end

function M:create_matrix(scale, rot, trans)
	local s = self:_matrix_identity()
	local r = self:_matrix_identity()
	local t = self:_matrix_identity()

	if scale then
		s[1] = scale[1]
		s[4] = scale[2]
	end

	if rot then
		local rad = math.rad(rot)
		local c = math.cos(rad)
		local s = math.sin(rad)
		r[1] = c
		r[2] = s
		r[3] = -s
		r[4] = c
	end

	if trans then
		t[5] = trans[1]
		t[6] = trans[2]
	end

	local ret = self:_matrix_multiply(self:_matrix_multiply(s, r), t)
	ret[1] = math.floor(ret[1] * 1024)
	ret[2] = math.floor(ret[2] * 1024)
	ret[3] = math.floor(ret[3] * 1024)
	ret[4] = math.floor(ret[4] * 1024)
	ret[5] = math.floor(ret[5] * 16)
	ret[6] = math.floor(ret[6] * 16)

	return ret
end

return M