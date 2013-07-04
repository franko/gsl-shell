
local floor = math.floor

local insertion_thresold = 32

local function less_than(a, b)
    return a < b
end

local function idiv(a, b)
    return floor(a / b)
end

local function insertion_sort(array, compare, istart, iend)
    for i = istart + 1, iend do
        local current_value = array[i]
        local hole_index = i
        while hole_index > istart and compare(current_value, array[hole_index - 1]) do
            array[hole_index] = array[hole_index - 1]
            hole_index = hole_index - 1
        end
        array[hole_index] = current_value
    end
end

local function quicksort_array(m, f)
    local array = m.data
    local indices = {}

    f = f or less_than

    local function less_than_by_index(ia, ib)
        return f(array[ia], array[ib])
    end

    local function partition(istart, iend, pivot_index)
        local pivot_value = array[pivot_index]
        array[pivot_index], array[iend] = array[iend], array[pivot_index]
        local store_index = istart
        for i = istart, iend - 1 do
            if f(array[i], pivot_value) then
                array[i], array[store_index] = array[store_index], array[i]
                store_index = store_index + 1
            end
        end
        array[store_index], array[iend] = array[iend], array[store_index]
        return store_index
    end

    local function choose_pivot(istart, iend)
        local n = iend - istart + 1
        if n < 5 then
            return istart
        elseif n < 32 then
            local imid = idiv(istart + iend, 2)
            indices[1], indices[2], indices[3] = istart, imid, iend
            insertion_sort(indices, less_than_by_index, 1, 3)
            if less_than_by_index(indices[1], indices[2]) then return indices[2] end
            return indices[3]
        else
            local im1 = idiv(3*istart +   iend, 4)
            local im2 = idiv(2*istart + 2*iend, 4)
            local im3 = idiv(  istart + 3*iend, 4)
            indices[1], indices[2], indices[3], indices[4], indices[5] = istart, im1, im2, im3, iend
            insertion_sort(indices, less_than_by_index, 1, 5)
            local iref = indices[1]
            if less_than_by_index(iref, indices[3]) then return indices[3] end
            if less_than_by_index(iref, indices[4]) then return indices[4] end
            return indices[5]
        end
    end

    local function quicksort(istart, iend)
        local n = iend - istart + 1
        if n < insertion_thresold then
            insertion_sort(array, f, istart, iend)
        else
            local pivot_index = choose_pivot(istart, iend)
            local part_index = partition(istart, iend, pivot_index)
            if part_index - istart == 0 then
                -- odd case,should  happen only for constant array
                insertion_sort(array, f, istart, iend)
            else
                quicksort(istart, part_index - 1)
                quicksort(part_index + 1, iend)
            end
        end
    end

    local n, ncols = m:dim()
    assert(ncols == 1, "expecting column matrix")
    quicksort(0, n - 1)
end

return quicksort_array
