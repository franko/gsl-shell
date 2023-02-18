local function plot_verify(data, n_plot, x_min, x_max)
    x_min = x_min or data:get(1, "x")
    x_max = x_max or data:get(#data, "x")
    -- Setting the linear interpolation to cubic will reveal some
    -- pathologic behavior with data5 and data7 defined below
    local fn = gdt.interp(data, "y ~ x", "linear")
    local lnd = graph.path()
    for x, y in iter.sequence(function(i) return data:get(i, "x"), data:get(i, "y") end, #data) do
        if x >= x_min and x <= x_max then
            lnd:line_to(x, y)
        end
    end
    local lnf = graph.fxline(fn, x_min, x_max, n_plot or 1024)
    local p = graph.plot()
    p:addline(lnf, "blue")
    p:add(lnd, "black", {{"stroke"}, {"marker", size= 8}})
    p:show()
    return p
end

local data1 = gdt.read_string [[
x	y
0	0
0.523598775598299	0.5
1.0471975511966	0.866025403784439
1.5707963267949	1
2.0943951023932	0.866025403784439
2.61799387799149	0.5
3.14159265358979	0
3.66519142918809	-0.5
4.18879020478639	-0.866025403784438
4.71238898038469	-1
5.23598775598299	-0.866025403784439
5.75958653158129	-0.5
6.28318530717959	0
]]

local data2 = gdt.read_string [[
x	y
6.28318530717959	0
5.75958653158129	-0.5
5.23598775598299	-0.866025403784439
4.71238898038469	-1
4.18879020478639	-0.866025403784438
3.66519142918809	-0.5
3.14159265358979	0
2.61799387799149	0.5
2.09439510239319	0.866025403784439
1.5707963267949	1
1.0471975511966	0.866025403784438
0.523598775598299	0.5
0	0
]]


local data3 = gdt.read_string [[
x	y
0	0
0.523598775598299	0.5
1.0471975511966	0.866025403784439
1.0471975511966	0.866025403784439
1.0471975511966	0.866025403784439
1.0471975511966	0.866025403784439
1.5707963267949	1
2.0943951023932	0.866025403784439
2.61799387799149	0.5
3.14159265358979	0
3.66519142918809	-0.5
4.18879020478639	-0.866025403784438
4.18879020478639	-0.866025403784438
4.71238898038469	-1
5.23598775598299	-0.866025403784439
5.75958653158129	-0.5
6.28318530717959	0
]]

local data3b = gdt.read_string [[
x	y
0	0
0.523598775598299	0.5
1.0471975511966	0.846025403784439
1.0471975511966	0.856025403784439
1.0471975511966	0.876025403784439
1.0471975511966	0.886025403784439
1.5707963267949	1
2.0943951023932	0.866025403784439
2.61799387799149	0.5
3.14159265358979	0
3.66519142918809	-0.5
4.18879020478639	-0.856025403784438
4.18879020478639	-0.876025403784438
4.71238898038469	-1
5.23598775598299	-0.866025403784439
5.75958653158129	-0.5
6.28318530717959	0
]]

local data3c = gdt.read_string [[
x	y
0	0
0.523598775598299	0.5
1.0471975511966	0.866025403784439
1.5707963267949	1
2.0943951023932	0.866025403784439
2.61799387799149	0.5
3.14159265358979	0
3.66519142918809	-0.5
4.18879020478639	-0.866025403784438
4.71238898038469	-1
5.23598775598299	-0.866025403784439
5.75958653158129	-0.5
6.28318530717959	-0.7
6.28318530717959	0.3
6.28318530717959	0.4
]]

local data3d = gdt.read_string [[
x	y
0	0.7
0	-0.3
0	-0.4
0.523598775598299	0.5
1.0471975511966	0.866025403784439
1.5707963267949	1
2.0943951023932	0.866025403784439
2.61799387799149	0.5
3.14159265358979	0
3.66519142918809	-0.5
4.18879020478639	-0.866025403784438
4.71238898038469	-1
5.23598775598299	-0.866025403784439
5.75958653158129	-0.5
6.28318530717959	0
]]

local data4 = gdt.read_string [[
x	y
6.28318530717959	0
5.75958653158129	-0.5
5.23598775598299	-0.826025403784439
5.23598775598299	-0.866025403784439
5.23598775598299	-0.906025403784439
4.71238898038469	-1
4.18879020478639	-0.866025403784438
3.66519142918809	-0.5
3.14159265358979	0
2.61799387799149	0.5
2.09439510239319	0.866025403784439
1.5707963267949	1
1.0471975511966	0.866025403784438
0.523598775598299	0.5
0	0
]]

local data3e = gdt.read_string [[
x	y
0	0
0.523598775598299	0.5
1.0471975512966	0.846025403784439
1.0471975511966	0.856025403784439
1.0471975510966	0.876025403784439
1.0471975513966	0.886025403784439
1.5707963267949	1
2.0943951023932	0.866025403784439
2.61799387799149	0.5
3.14159265358979	0
3.66519142918809	-0.5
4.18879020478639	-0.856025403784438
4.18879020488639	-0.876025403784438
4.71238898038469	-1
5.23598775598299	-0.866025403784439
5.75958653158129	-0.5
6.28318530717959	0
]]

local data5 = gdt.read_string [[
x	y
0	0
0.523598775598299	0.5
1.0471975511966	0.866025403784439
1.5707963267949	1
2.0943951023932	0.866025403784439
2.61799387799149	0.5
3.14159265358979	5.66553889764798E-16
3.66519142918809	-0.5
4.18879020478639	-0.866025403784438
4.71238898038469	-1
5.23598775598299	-0.866025403784439
5.75958653158129	-0.5
6.28318530717959	6.43249059870655E-16
634.601716025138	-1.76324379851755E-14
635.125314800737	0.5
635.648913576335	0.866025403784447
636.172512351933	1
636.696111127532	0.866025403784413
637.21970990313	0.499999999999941
637.743308678728	-8.52737940201243E-14
638.266907454326	-0.500000000000089
638.790506229925	-0.866025403784498
639.314105005523	-1
639.837703781121	-0.866025403784362
640.36130255672	-0.499999999999852
1268.67983327468	-0.49999999999975
1269.20343205028	1.92108799472881E-13
1269.72703082587	0.500000000000083
1270.25062960147	0.866025403784438
1270.77422837707	1
1271.29782715267	0.866025403784536
1271.82142592827	0.500000000000252
1272.34502470387	3.87105994851515E-13
1272.86862347946	-0.499999999999581
1273.39222225506	-0.866025403784149
1273.91582103066	-1
1274.43941980626	-0.866025403784825
]]

-- add errors in the y to a pure sin function
local data6 = gdt.read_string [[
x	y
0	0.003254018628873
0.261799387799149	0.263676244591206
0.523598775598299	0.574138998901037
0.785398163397448	0.759946555317806
1.0471975511966	0.94523164036826
1.30899693899575	0.9731574305205
1.5707963267949	1.02149678860851
1.83259571459405	0.991408319461498
2.0943951023932	0.892233136688228
2.35619449019234	0.775097166193707
2.61799387799149	0.568102476340258
2.87979326579064	0.305855762568069
3.14159265358979	0.000531360225808
3.40339204138894	-0.243212613503221
3.66519142918809	-0.439695162506695
3.92699081698724	-0.676825214437593
4.18879020478639	-0.859616861328796
4.45058959258554	-0.92646479835483
4.71238898038469	-0.919692720192127
4.97418836818384	-0.881372955917321
5.23598775598299	-0.803518032661881
5.49778714378214	-0.61835783011251
5.75958653158129	-0.403933682664995
6.02138591938044	-0.233467465227535
6.28318530717958	0.069901349145722
]]

-- data set with irregularly spaced xs and ys with important
-- random error from a pure sin function
local data7 = gdt.read_string [[
x	y
0	0.074028933383044
0.232814579126301	0.29505004992769
0.266168985899687	0.303458778349064
0.376726280293937	0.428493485405359
0.402561218107445	0.398006626065468
0.504120440065855	0.555045463097633
0.610951538649616	0.621996567674095
0.757333406895634	0.732587191008176
0.940290442688697	0.867290882894891
1.1421190723076	0.941375854033823
1.39373554534874	1.03470232490376
1.51801753634852	1.08074071791339
1.65819669286121	1.00201174841162
1.78641827391938	1.04584209779014
1.83332900803745	1.06472665667356
1.91822621200957	0.985964991517333
1.9334026798334	1.0192107460104
2.03610412043703	0.942711834708965
2.29703574705453	0.808749142247625
2.39042837507481	0.710216003866274
2.63948884745189	0.517925955208467
2.77145028431125	0.457490380923337
2.90091460287605	0.315806297306499
3.11357414980946	0.037399045736675
3.25128447353267	-0.091204215254085
3.38350823724852	-0.185197203184685
3.4678825221127	-0.270998482924071
3.5622483405201	-0.330349955445292
3.62321266496475	-0.429018880116479
3.77542203877365	-0.505582383807651
3.78581461761195	-0.50692175187132
4.02778833687316	-0.764941445034597
4.02826384750717	-0.712415542425449
4.22651986816917	-0.854750143939611
4.4394023840497	-0.915396001791585
4.64912076718373	-0.928741144180866
4.69748840658369	-0.910675735123193
4.75352004812881	-0.934808889299621
4.90678575762649	-0.978570342373851
4.9096536891246	-0.93705339196818
5.05217865786725	-0.924405441446902
5.12759423216246	-0.853916178689537
5.35212926424829	-0.769936304801647
5.5368686232182	-0.601723554529666
5.62771631773418	-0.543390273975671
5.7696855522588	-0.448213455856266
5.99630530685886	-0.256570219432913
6.03324288578375	-0.197218258136291
6.09828393952138	-0.137734474694528
6.34307236534758	0.083321683622352
]]

plot_verify(data3b)
plot_verify(data5, 4 * 8192)

plot_verify(data5, 1024, 0, 10)

plot_verify(data6)
plot_verify(data7)

