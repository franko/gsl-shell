local matrix = require("matrix")

a = matrix.new(5, 5)
for i = 1, 5 do
    for j = 1, 5 do
        a:set(i, j, 1 / (i + j - 1))
    end
end

c = a * a
for i = 1, 5 do
    local ls = {}
    for j = 1, 5 do
        ls[j] = c:get(i, j)
    end
    print(table.concat(ls, ","))
end
