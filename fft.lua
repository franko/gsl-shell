fftw = require'fftw3.init'
local gsl_matrix = ffi.typeof('gsl_matrix')
local gsl_matrix_complex = ffi.typeof('gsl_matrix_complex')

function num.fft(vec)
	if ffi.istype(gsl_matrix, vec) then
		return num.rfft(vec)
	else
		local n = tonumber(vec.size1)
		local outvec = matrix.cnew(n, 1)
		local output = ffi.cast("fftw_complex*",outvec.data)
		local input = ffi.cast("fftw_complex*",vec.data)
		local plan = fftw.plan_dft_1d(n, input, output, fftw.FORWARD, fftw.MEASURE)
		fftw.execute(plan)
		fftw.destroy_plan(plan)
		return outvec
	end
end

function num.fft_inv(vec)
	if ffi.istype(gsl_matrix, vec) then
		return num.rfft_inv(vec)
	else
		local n = tonumber(vec.size1)
		local outvec = matrix.cnew(n, 1)
		local output = ffi.cast("fftw_complex*",outvec.data)
		local input = ffi.cast("fftw_complex*",vec.data)
		local plan = fftw.plan_dft_1d(n, input, output, fftw.BACKWARD, fftw.MEASURE)
		fftw.execute(plan)
		fftw.destroy_plan(plan)
		return outvec/n
	end
end

function num.rfft(vec)
	if ffi.istype(gsl_matrix, vec) then
		local n = tonumber(vec.size1)
		local outvec = matrix.cnew(math.floor(n/2)+1, 1)
		local output = ffi.cast("fftw_complex*",outvec.data)
		local plan = fftw.plan_dft_r2c_1d(n, vec.data, output, fftw.MEASURE )
		fftw.execute(plan)
		fftw.destroy_plan(plan)
		return outvec
	else
		error("Cannot handle nonreal input data.")
	end
end

function num.rfft_inv(vec)
	if ffi.istype(gsl_matrix_complex, vec) then
		local n = tonumber(vec.size1)
		local nnew = (n-1)*2
		local outvec = matrix.new(nnew, 1)
		local input = ffi.cast("fftw_complex*",vec.data)
		local plan = fftw.plan_dft_c2r_1d(nnew, input, outvec.data, fftw.MEASURE )
		fftw.execute(plan)
		fftw.destroy_plan(plan)
		return outvec/nnew
	else
		error("Cannot handle noncomplex input data.")
	end
end

----------------------------------

function num.fft2(mat)
	if ffi.istype(gsl_matrix, mat) then
		return num.rfft2(mat)
	else
		local n1 = tonumber(mat.size1)
		local n2 = tonumber(mat.size2)
		local outmat = matrix.cnew(n1, n2)
		local output = ffi.cast("fftw_complex*",outmat.data)
		local input = ffi.cast("fftw_complex*", mat.data)
		local plan = fftw.plan_dft_2d(n1,n2, input, output, fftw.FORWARD, fftw.MEASURE)
		fftw.execute(plan)
		fftw.destroy_plan(plan)
		return outmat
	end
end

function num.fft2_inv(mat)
	if ffi.istype(gsl_matrix, mat) then
		return num.rfft2_inv(mat)
	else
		local n1 = tonumber(mat.size1)
		local n2 = tonumber(mat.size2)
		local outmat = matrix.cnew(n1, n2)
		local output = ffi.cast("fftw_complex*",outmat.data)
		local input = ffi.cast("fftw_complex*", mat.data)
		local plan = fftw.plan_dft_2d(n1,n2, input, output, fftw.BACKWARD, fftw.MEASURE)
		fftw.execute(plan)
		fftw.destroy_plan(plan)
		return outmat
	end
end

function num.rfft2(mat)
	if ffi.istype(gsl_matrix, mat) then
		local n1 = tonumber(mat.size1)
		local n2 = tonumber(mat.size2)
		local outmat = matrix.cnew(n1, math.floor(n2/2)+1)
		local output = ffi.cast("fftw_complex*",outmat.data)
		local plan = fftw.plan_dft_r2c_2d(n1,n2, mat.data, output,fftw.MEASURE)
		fftw.execute(plan)
		fftw.destroy_plan(plan)
		return outmat
	else
		error("Cannot handle nonreal input matrix.")
	end
end

function num.rfft2_inv(mat)
	if ffi.istype(gsl_matrix_complex, mat) then
		local n1 = tonumber(mat.size1)
		local n2 = tonumber(mat.size2)
		local n2new = (n2-1)*2
		local outmat = matrix.new(n1, n2new)
		local input = ffi.cast("fftw_complex*",mat.data)
		local plan = fftw.plan_dft_c2r_2d(n1,n2new, input, outmat.data,fftw.MEASURE)
		fftw.execute(plan)
		fftw.destroy_plan(plan)
		return outmat/(n1*n2new)
	else
		error("Cannot handle noncomplex input matrix.")
	end
end

--------------------------------------

function num.fft3(datavec, n1, n2, n3)
	if ffi.istype(gsl_matrix, datavec) then
		return num.rfft3(datavec, n1, n2, n3)
	else
		local outvec = matrix.cnew(n1*n2*n3, 1)
		local output = ffi.cast("fftw_complex*",outvec.data)
		local input = ffi.cast("fftw_complex*", datavec.data)
		local plan = fftw.plan_dft_3d(n1,n2,n3, input, output, fftw.FORWARD, fftw.MEASURE)
		fftw.execute(plan)
		fftw.destroy_plan(plan)
		return outvec
	end
end

function num.fft3_inv(datavec, n1, n2, n3)
	if ffi.istype(gsl_matrix, datavec) then
		return num.rfft3_inv(datavec, n1, n2, n3)
	else
		local outvec = matrix.cnew(n1*n2*n3, 1)
		local output = ffi.cast("fftw_complex*",outvec.data)
		local input = ffi.cast("fftw_complex*", datavec.data)
		local plan = fftw.plan_dft_3d(n1,n2,n3, input, output, fftw.BACKWARD, fftw.MEASURE)
		fftw.execute(plan)
		fftw.destroy_plan(plan)
		return outvec/(n1*n2*n3)
	end
end

function num.rfft3(datavec, n1, n2, n3)
	if ffi.istype(gsl_matrix, datavec) then
		local outvec = matrix.cnew(n1*n2*(math.floor(n3/2)+1), 1)
		local output = ffi.cast("fftw_complex*",outvec.data)
		local plan = fftw.plan_dft_r2c_3d(n1,n2,n3, datavec.data, output, fftw.MEASURE)
		fftw.execute(plan)
		fftw.destroy_plan(plan)
		return outvec
	else
		error("Cannot handle nonreal input data.")
	end
end

function num.rfft3_inv(datavec, n1, n2, n3)
	if ffi.istype(gsl_matrix, datavec) then
		local outvec = matrix.new(n1*n2*n3, 1)
		local input = ffi.cast("fftw_complex*",datavec.data)
		local plan = fftw.plan_dft_r2c_3d(n1,n2,n3, input, outvec.data, fftw.MEASURE)
		fftw.execute(plan)
		fftw.destroy_plan(plan)
		return outvec/(n1*n2*n3)
	else
		error("Cannot handle nonreal input data.")
	end
end

--------------------------------------

function num.fftn(datavec, dimensionvec)
	if ffi.istype(gsl_matrix, datavec) then
		return num.rfftn(datavec, dimensionvec)
	else
		local outvec = matrix.cnew(tonumber(datavec.size1), 1)
		local output = ffi.cast("fftw_complex*",outvec.data)
		local input = ffi.cast("fftw_complex*", datavec.data)
		local plan = fftw.plan_dft(#dimensionvec, dimensionvec.data, input, output, fftw.FORWARD, fftw.MEASURE)
		fftw.execute(plan)
		fftw.destroy_plan(plan)
		return outvec
	end
end

function num.fftn_inv(datavec, dimensionvec)
	if ffi.istype(gsl_matrix, datavec) then
		return num.rfftn_inv(datavec, dimensionvec)
	else
		local outvec = matrix.cnew(tonumber(datavec.size1), 1)
		local output = ffi.cast("fftw_complex*",outvec.data)
		local input = ffi.cast("fftw_complex*", datavec.data)
		local plan = fftw.plan_dft(#dimensionvec, dimensionvec.data, input, output, fftw.BACKWARD, fftw.MEASURE)
		fftw.execute(plan)
		fftw.destroy_plan(plan)
		return outvec
	end
end

function num.rfftn(datavec, dimensionvec)
	if ffi.istype(gsl_matrix, datavec) then
		local d = #dimensionvec
		local newsize = 1
		for i=1,d-1 do
			newsize = newsize * dimensionvec[i]
		end
		newsize = newsize * math.floor(dimensionvec[d]/2)+1

		local outvec = matrix.cnew(newsize, 1)
		local output = ffi.cast("fftw_complex*",outvec.data)
		local plan = fftw.plan_dft_r2c(d, dimensionvec.data, datavec.data, output, fftw.MEASURE)
		fftw.execute(plan)
		fftw.destroy_plan(plan)
		return outvec
	else
		error("Cannot handle nonreal input data.")
	end
end

function num.rfftn_inv(datavec, dimensionvec)
	if ffi.istype(gsl_matrix, datavec) then
		local d = #dimensionvec
		local newsize = 1
		for i=1,d do
			newsize = newsize * dimensionvec[i]
		end

		local outvec = matrix.new(newsize, 1)
		local output = ffi.cast("fftw_complex*",outvec.data)
		local plan = fftw.plan_dft_r2c(d, dimensionvec.data, datavec.data, output, fftw.MEASURE)
		fftw.execute(plan)
		fftw.destroy_plan(plan)
		return outvec
	else
		error("Cannot handle nonreal input data.")
	end
end
