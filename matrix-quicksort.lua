
local function quicksort_array(m, f)
	local t = m.data

    local function quicksort(start, endi)
        if start >= endi then return end
        local pivot = start
        for i = start + 1, endi do
            if f(t[i], t[pivot]) then
                local temp = t[pivot + 1]
                t[pivot + 1] = t[pivot]
                if(i == pivot + 1) then
                    t[pivot] = temp
                else
                    t[pivot] = t[i]
                    t[i] = temp
                end
                pivot = pivot + 1
            end
        end

        quicksort(start, pivot - 1)
        quicksort(pivot + 1, endi)
    end

    local n, ncols = m:dim()
    assert(ncols == 1, "expecting column matrix")
    quicksort(0, n - 1)
end

return quicksort_array
