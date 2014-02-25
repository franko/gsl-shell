local ffi = require'ffi'
local fftw = require'fftw3.init'
local gsl_matrix = ffi.typeof('gsl_matrix')
local gsl_matrix_complex = ffi.typeof('gsl_matrix_complex')

local FORWARD = 1
local BACKWARD = 2


local function compare_dimensions(dim1, dim2)
	for i = 1,#dim1 do
		if dim1[i] ~= dim2[i] then
			return false
		end
	end
	return true
end

local dftplans = {}
local function dft(invec, dimlist, outvec, direction)

	if ffi.istype(gsl_matrix_complex, invec) then
		local rank = #dimlist
		local size = 1
		for i=1,rank do size = size * dimlist[i] end
		local direction = direction or FORWARD
		local output_supplied = true

		--First check if output vector is given by the user, otherwise allocate it
		if not outvec then
			output_supplied = false
			outvec = matrix.calloc(size, 1)
		end

		local output = ffi.cast("fftw_complex*",outvec.data)
		local input = ffi.cast("fftw_complex*",invec.data)

		--Check if plan is existing, then use it
		if dftplans[rank] ~= nil and dftplans[rank][direction] ~= nil and
		   compare_dimensions(dimlist, dftplans[rank][direction].dimlist) and
		   invec.tda == dftplans[rank][direction].itda and
		   outvec.tda == dftplans[rank][direction].otda then

			local plan = dftplans[rank][direction].plan
			fftw.execute_dft(plan, input, output)
			if not output_supplied then return outvec end
		--Or create a new plan
		else
			--allcoate input for plan making but planning can invalidate input, therefore we use extra array
			local inputtest = ffi.new("fftw_complex[?]", size * invec.tda)

			local howmany = 1
			local idist,odist = 0,0
			local istride = invec.tda
			local ostride = outvec.tda
			local dim = ffi.new("int[?]", rank, dimlist)
			local plan = ffi.gc(fftw.plan_many_dft(rank, dim ,howmany,
											inputtest,nil, istride, idist,
											output,nil,ostride,odist,
											direction == FORWARD and fftw.FORWARD or fftw.BACKWARD, bit.bor(fftw.MEASURE, fftw.UNALIGNED)), fftw.destroy_plan)

			--Save the plan for next time
			if dftplans[rank] == nil then dftplans[rank] = {} end
			dftplans[rank][direction] = {plan=plan, dimlist=dimlist, itda=invec.tda, otda=outvec.tda}

			--Execute the plan with the supplied input
			fftw.execute_dft(plan, input, output)
			if not output_supplied then return outvec end
		end
	else
		error("Input must be complex valued.")
	end
end

local rdftplans = {}
local function rdft(invec, dimlist, outvec)

	if ffi.istype(gsl_matrix, invec) then
		local rank = #dimlist
		local size = 1
		for i=1,rank do size = size * dimlist[i] end
		local outputsize = size/dimlist[rank]*(math.floor(dimlist[rank]/2)+1)

		local direction = FORWARD
		local output_supplied = true

		--First check if output vector is given by the user, otherwise allocate it
		if not outvec then
			output_supplied = false
			outvec = matrix.calloc(outputsize, 1)
		end

		local output = ffi.cast("fftw_complex*",outvec.data)
		local input = invec.data

		--Check if plan is existing, then use it
		if rdftplans[rank] ~= nil and rdftplans[rank][direction] ~= nil and
		   compare_dimensions(dimlist, rdftplans[rank][direction].dimlist) and
		   invec.tda == rdftplans[rank][direction].itda and
		   outvec.tda == rdftplans[rank][direction].otda then

			local plan = rdftplans[rank][direction].plan
			fftw.execute_dft_r2c(plan, input, output)
			if not output_supplied then return outvec end

		--Or create a new plan
		else
			--allcoate input for plan making but planning can invalidate input, therefore we use extra array
            local inputtest = ffi.new("double[?]", size * invec.tda)

			local howmany = 1
			local idist,odist = 0,0
			local istride = invec.tda
			local ostride = outvec.tda
			local dim = ffi.new("int[?]", rank, dimlist)
			local plan = ffi.gc(fftw.plan_many_dft_r2c(rank, dim, howmany,
											 inputtest,nil,istride, idist,
											 output,nil, ostride, odist,
											 bit.bor(fftw.MEASURE, fftw.UNALIGNED)), fftw.destroy_plan)

			--Save the plan for next time
			if rdftplans[rank] == nil then rdftplans[rank] = {} end
			rdftplans[rank][direction] = {plan=plan, dimlist=dimlist, itda=invec.tda, otda=outvec.tda}

			--Execute the plan with the supplied input
			fftw.execute_dft_r2c(plan, input, output)
			if not output_supplied then return outvec end
		end
	else
		error("Input must be real valued.")
	end
end

local function rdftinv(invec, dimlist, outvec)

	if ffi.istype(gsl_matrix_complex, invec) then
		local rank = #dimlist
		local size = 1
		for i=1,rank do size = size * dimlist[i] end
		local inputsize = size/dimlist[rank]*(math.floor(dimlist[rank]/2)+1)

		local direction = BACKWARD
		local output_supplied = true

		--First check if output vector is given by the user, otherwise allocate it
		if not outvec then
			output_supplied = false
			outvec = matrix.alloc(size, 1)
		end

		local output = outvec.data
		local input = ffi.cast("fftw_complex*",invec.data)

		--Check if plan is existing, then use it
		if rdftplans[rank] ~= nil and rdftplans[rank][direction] ~= nil and
		   compare_dimensions(dimlist, rdftplans[rank][direction].dimlist) and
		   invec.tda == rdftplans[rank][direction].itda and
		   outvec.tda == rdftplans[rank][direction].otda then

			local plan = rdftplans[rank][direction].plan
			fftw.execute_dft_c2r(plan, input, output)
			if not output_supplied then return outvec end

		--Or create a new plan
		else
			--allcoate input for plan making but planning can invalidate input, therefore we use extra array
			local inputtest = ffi.new("fftw_complex[?]", inputsize * invec.tda)

			local howmany = 1
			local idist,odist = 0,0
			local istride = invec.tda
			local ostride = outvec.tda
			local dim = ffi.new("int[?]", rank, dimlist)
			local plan = ffi.gc(fftw.plan_many_dft_c2r(rank, dim, howmany,
											inputtest, nil, istride, idist,
											output, nil, ostride, odist,
											bit.bor(fftw.MEASURE, fftw.UNALIGNED)), fftw.destroy_plan)

			--Save the plan for next time
			if rdftplans[rank] == nil then rdftplans[rank] = {} end
			rdftplans[rank][direction] = {plan=plan, dimlist=dimlist, itda=invec.tda, otda=outvec.tda}

			--Execute the plan with the supplied input
			fftw.execute_dft_c2r(plan, input, output)
			if not output_supplied then return outvec end
		end
	else
		error("Input must be complex valued.")
	end
end

------------------------------------------------

function num.fft(invec, outvec)
	return dft(invec, {#invec}, outvec, FORWARD)
end

function num.fftinv(invec, outvec)
	return dft(invec, {#invec}, outvec, BACKWARD)
end

function num.rfft(invec, outvec)
	return rdft(invec, {#invec}, outvec)
end

function num.rfftinv(invec, outvec)
	return rdftinv(invec, {(#invec-1)*2}, outvec)
end

function num.fft2(inmat, outmat)
	local n1 = tonumber(inmat.size1)
	local n2 = tonumber(inmat.size2)

	if outmat == nil then
		local retvec = dft(inmat, {n1, n2}, outmat, FORWARD)
		local b = ffi.cast('gsl_block_complex*', ffi.C.malloc(ffi.sizeof('gsl_block_complex')))
	   	b.size, b.data, b.ref_count = n1*n2, retvec.data, 2
	   	return gsl_matrix_complex(n1, n2, n2, b.data, b, 1)
	else
		dft(inmat, {n1, n2}, outmat, FORWARD)
	end
end

function num.fft2inv(inmat, outmat)
	local n1 = tonumber(inmat.size1)
	local n2 = tonumber(inmat.size2)

	if outmat == nil then
		local retvec = dft(inmat, {n1, n2}, outmat, BACKWARD)
		local b = ffi.cast('gsl_block_complex*', ffi.C.malloc(ffi.sizeof('gsl_block_complex')))
	   	b.size, b.data, b.ref_count = n1*n2, retvec.data, 2
	   	return gsl_matrix_complex(n1, n2, n2, b.data, b, 1)
	else
		dft(inmat, {n1, n2}, outmat, BACKWARD)
	end
end

function num.rfft2(inmat, outmat)
	local n1 = tonumber(inmat.size1)
	local n2 = tonumber(inmat.size2)

	if outmat == nil then
		local retvec = rdft(inmat, {n1, n2}, outmat)
		local b = ffi.cast('gsl_block_complex*', ffi.C.malloc(ffi.sizeof('gsl_block_complex')))
		local n2new = math.floor(n2/2)+1
	   	b.size, b.data, b.ref_count = n1*n2new, retvec.data, 2
	   	return gsl_matrix_complex(n1, n2new, n2new, b.data, b, 1)
	else
		rdft(inmat, {n1, n2}, outmat)
	end
end

function num.rfft2inv(inmat, outmat)
	local n1 = tonumber(inmat.size1)
	local n2 = (tonumber(inmat.size2)-1)*2

	if outmat == nil then
		local retvec = rdftinv(inmat, {n1, n2}, outmat)
		local b = ffi.cast('gsl_block*', ffi.C.malloc(ffi.sizeof('gsl_block')))
	   	b.size, b.data, b.ref_count = n1*n2, retvec.data, 2
	   	return gsl_matrix(n1, n2, n2, b.data, b, 1)
	else
		rdft(inmat, {n1, n2}, outmat)
	end
end

function num.fftn(invec, dimlist, outvec)
	return dft(invec, dimlist, outvec, FORWARD)
end

function num.fftninv(invec, dimlist, outvec)
	return dft(invec, dimlist, outvec, BACKWARD)
end

function num.rfftn(invec, dimlist, outvec)
	return rdft(invec, dimlist, outvec)
end

function num.rfftninv(invec, dimlist, outvec)
	return rdftinv(invec, dimlist, outvec)
end
