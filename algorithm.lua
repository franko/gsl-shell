--
-- GSL shell interactive interface to GSL library
-- Based on the Lua programming language
--
-- Copyright (C) 2009-2013 Francesco Abbate.
-- See Copyright Notice in gsl-shell-jit.c
--
-- This file provided an implementation of the quicksort algorithm.
-- Based on the libstdc++ std::sort implementation included with GCC.
--

local floor = math.floor

local insertion_thresold = 16

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

local function quicksort(array, i0, i1, f)
    f = f or less_than

    local function move_median_first(a, b, c)
        if f(array[a], array[b]) then
            if f(array[b], array[c]) then
                array[a], array[b] = array[b], array[a]
            else
                array[a], array[c] = array[c], array[a]
            end
        elseif f(array[a], array[c]) then
            return
        elseif f(array[b], array[c]) then
            array[a], array[c] = array[c], array[a]
        else
            array[a], array[b] = array[b], array[a]
        end
    end

    local function partition(first, last, pivot_value)
        while true do
            while f(array[first], pivot_value) do
                first = first + 1
            end
            while f(pivot_value, array[last]) do
                last = last - 1
            end
            if first >= last then
                return first
            end
            array[first], array[last] = array[last], array[first]
            first = first + 1
            last = last - 1
        end
    end

    local function partition_pivot(first, last)
        local mid = idiv(first + last, 2)
        move_median_first(first, mid, last)
        return partition(first + 1, last, array[first])
    end

    local function quicksort_loop(first, last)
        while last - first > insertion_thresold do
            local cut = partition_pivot(first, last)
            quicksort_loop(cut, last)
            array[first], array[first + 1] = array[first + 1], array[first]
            last = cut - 1
        end
    end

    quicksort_loop(i0, i1)
    insertion_sort(array, f, i0, i1)
end

local function array_search(array, i0, i1, val)
    for k = i0, i1 do
        if array[k] == val then return k end
    end
end

-- sort arrays "array" and "slave" in place for indices from i0 to i1
-- based on values of "array" using the comparison function "f"
local function quicksort_mirror(array, slave, i0, i1, f)

    local function swap(index, a, b)
        array[a], array[b] = array[b], array[a]
        slave[a], slave[b] = slave[b], slave[a]
        index[a], index[b] = index[b], index[a]
    end

    local n = i1 - i0 + 1
    local id, iv = {}, {}
    for k = 1, n do id[k], iv[k] = k, k end
    quicksort(id, i0, i1, function(a, b) return f(array[a], array[b]) end)
    for k = 1, n do
        local val = id[k]
        if val > k then
            swap(iv, k, val)
        elseif val < k then
            val = array_search(iv, k, n, val)
            swap(iv, k, val)
        end
    end
end

return {quicksort = quicksort, quicksort_mirror = quicksort_mirror}
