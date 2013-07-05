
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

local function quicksort(m, f)
    local array = m.data
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

    local n, ncols = m:dim()
    assert(ncols == 1, "expecting column matrix")
    quicksort_loop(0, n - 1)
    insertion_sort(array, f, 0, n - 1)
end

return quicksort
