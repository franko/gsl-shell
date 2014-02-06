ffi = require'ffi'
fftw = require'fftw3.init'
local gsl_matrix = ffi.typeof('gsl_matrix')
local gsl_matrix_complex = ffi.typeof('gsl_matrix_complex')

function num.fft(vec)
	if ffi.istype(gsl_matrix_complex, vec) then
		local n = tonumber(vec.size1)
		local outvec = matrix.calloc(n, 1)
		local output = ffi.cast("fftw_complex*",outvec.data)
		local input = ffi.cast("fftw_complex*",vec.data)

		local plan = ffi.gc(fftw.plan_dft_1d(n, input, output, fftw.FORWARD, fftw.MEASURE), fftw.destroy_plan)
		fftw.execute(plan)
		return outvec, plan
	else
		error("Input must be complex valued.")
	end
end

function num.fftinv(vec)
	if ffi.istype(gsl_matrix_complex, vec) then
		local n = tonumber(vec.size1)
		local outvec = matrix.calloc(n, 1)
		local output = ffi.cast("fftw_complex*",outvec.data)
		local input = ffi.cast("fftw_complex*",vec.data)

		local plan = ffi.gc(fftw.plan_dft_1d(n, input, output, fftw.BACKWARD, fftw.MEASURE), fftw.destroy_plan)
		fftw.execute(plan)
		return outvec, plan
	else
		error("Input must be complex valued.")
	end
end

function num.rfft(vec)
	if ffi.istype(gsl_matrix, vec) then
		local n = tonumber(vec.size1)
		local outvec = matrix.calloc(math.floor(n/2)+1, 1)
		local output = ffi.cast("fftw_complex*",outvec.data)
		local plan = ffi.gc(fftw.plan_dft_r2c_1d(n, vec.data, output, fftw.MEASURE), fftw.destroy_plan)
		fftw.execute(plan)
		return outvec, plan
	else
		error("Input must be real valued.")
	end
end

function num.rfftinv(vec)
	if ffi.istype(gsl_matrix_complex, vec) then
		local n = tonumber(vec.size1)
		local nnew = (n-1)*2
		local outvec = matrix.alloc(nnew, 1)
		local input = ffi.cast("fftw_complex*",vec.data)
		local plan = ffi.gc(fftw.plan_dft_c2r_1d(nnew, input, outvec.data, fftw.MEASURE), fftw.destroy_plan)
		fftw.execute(plan)
		return outvec,plan
	else
		error("Input must be complex valued.")
	end
end

----------------------------------

function num.fft2(mat)
	if ffi.istype(gsl_matrix_complex, mat) then
		local n1 = tonumber(mat.size1)
		local n2 = tonumber(mat.size2)
		local outmat = matrix.calloc(n1, n2)
		local output = ffi.cast("fftw_complex*",outmat.data)
		local input = ffi.cast("fftw_complex*", mat.data)
		local plan = ffi.gc(fftw.plan_dft_2d(n1,n2, input, output, fftw.FORWARD, fftw.MEASURE), fftw.destroy_plan)
		fftw.execute(plan)
		return outmat, plan
	else
		error("Input must be complex valued.")
	end
end

function num.fft2inv(mat)
	if ffi.istype(gsl_matrix_complex, mat) then
		local n1 = tonumber(mat.size1)
		local n2 = tonumber(mat.size2)
		local outmat = matrix.calloc(n1, n2)
		local output = ffi.cast("fftw_complex*",outmat.data)
		local input = ffi.cast("fftw_complex*", mat.data)
		local plan = ffi.gc(fftw.plan_dft_2d(n1,n2, input, output, fftw.BACKWARD, fftw.MEASURE), fftw.destroy_plan)
		fftw.execute(plan)
		return outmat, plan
	else
		error("Input must be complex valued.")
	end
end

function num.rfft2(mat)
	if ffi.istype(gsl_matrix, mat) then
		local n1 = tonumber(mat.size1)
		local n2 = tonumber(mat.size2)
		local outmat = matrix.calloc(n1, math.floor(n2/2)+1)
		local output = ffi.cast("fftw_complex*",outmat.data)
		local plan = ffi.gc(fftw.plan_dft_r2c_2d(n1,n2, mat.data, output,fftw.MEASURE), fftw.destroy_plan)
		fftw.execute(plan)
		return outmat, plan
	else
		error("Input must be real valued.")
	end
end

function num.rfft2inv(mat)
	if ffi.istype(gsl_matrix_complex, mat) then
		local n1 = tonumber(mat.size1)
		local n2 = tonumber(mat.size2)
		local n2new = (n2-1)*2
		local outmat = matrix.alloc(n1, n2new)
		local input = ffi.cast("fftw_complex*",mat.data)
		local plan = ffi.gc(fftw.plan_dft_c2r_2d(n1,n2new, input, outmat.data,fftw.MEASURE), fftw.destroy_plan)
		fftw.execute(plan)
		return outmat, plan
	else
		error("Input must be complex valued.")
	end
end

--------------------------------------

function num.fftn(datavec, dimlist)
	if ffi.istype(gsl_matrix_complex, datavec) then
		local outvec = matrix.calloc(tonumber(datavec.size1), 1)
		local output = ffi.cast("fftw_complex*",outvec.data)
		local input = ffi.cast("fftw_complex*", datavec.data)
		local plan = ffi.gc(fftw.plan_dft(#dimlist, ffi.new("int[?]", #dimlist, dimlist), input, output, fftw.FORWARD, fftw.MEASURE), fftw.destroy_plan)
		fftw.execute(plan)
		return outvec, plan
	else
		error("Input must be complex valued.")
	end
end

function num.fftninv(datavec, dimlist)
	if ffi.istype(gsl_matrix_complex, datavec) then
		local outvec = matrix.calloc(tonumber(datavec.size1), 1)
		local output = ffi.cast("fftw_complex*",outvec.data)
		local input = ffi.cast("fftw_complex*", datavec.data)

		local plan = ffi.gc(fftw.plan_dft(#dimlist, ffi.new("int[?]", #dimlist, dimlist), input, output, fftw.BACKWARD, fftw.MEASURE), fftw.destroy_plan)
		fftw.execute(plan)
		return outvec, plan
	else
		error("Input must be complex valued.")
	end
end

function num.rfftn(datavec, dimlist)
	if ffi.istype(gsl_matrix, datavec) then
		local d = #dimlist
		local newsize = 1
		for i=1,d-1 do
			newsize = newsize * dimlist[i]
		end
		newsize = newsize * math.floor(dimlist[d]/2)+1

		local outvec = matrix.calloc(newsize, 1)
		local output = ffi.cast("fftw_complex*",outvec.data)
		local plan = ffi.gc(fftw.plan_dft_r2c(d, ffi.new("int[?]", #dimlist, dimlist), datavec.data, output, fftw.MEASURE), fftw.destroy_plan)
		fftw.execute(plan)
		return outvec, plan
	else
		error("Input must be real valued.")
	end
end

function num.rfftninv(datavec, dimlist)
	if ffi.istype(gsl_matrix_complex, datavec) then
		local d = #dimlist
		local newsize = 1
		for i=1,d do
			newsize = newsize * dimlist[i]
		end

		local outvec = matrix.alloc(newsize, 1)
		local output = ffi.cast("fftw_complex*",outvec.data)
		local plan = ffi.gc(fftw.plan_dft_r2c(d, ffi.new("int[?]", #dimlist, dimlist), datavec.data, output, fftw.MEASURE), fftw.destroy_plan)
		fftw.execute(plan)
		return outvec
	else
		error("Input must be complex valued.")
	end
end

------------------------------------------

function num.fft_plan(plan, input, output)
	local fftw_in = ffi.cast("fftw_complex*",input.data)
	local fftw_out = ffi.cast("fftw_complex*",output.data)
	return fftw.execute_dft(plan, fftw_in, fftw_out)
end

function num.rfft_plan(plan, input, output)
	local fftw_out = ffi.cast("fftw_complex*",output.data)
	return fftw.execute_dft_r2c(plan, input.data, fftw_out)
end

function num.rfftinv_plan(plan, input, output)
	local fftw_in = ffi.cast("fftw_complex*",input.data)
	return fftw.execute_dft_c2r(plan, fftw_in, output.data)
end