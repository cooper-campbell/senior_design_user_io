const short int wavetable[1746] = {
32760, 32877, 32995, 33113, 33231, 33349, 33467, 33585, 
33702, 33820, 33938, 34056, 34174, 34292, 34409, 34527, 
34645, 34762, 34880, 34998, 35115, 35233, 35350, 35468, 
35585, 35703, 35820, 35938, 36055, 36172, 36289, 36407, 
36524, 36641, 36758, 36875, 36992, 37109, 37225, 37342, 
37459, 37576, 37692, 37809, 37925, 38041, 38158, 38274, 
38390, 38506, 38622, 38738, 38854, 38970, 39086, 39201, 
39317, 39432, 39548, 39663, 39778, 39893, 40008, 40123, 
40238, 40353, 40467, 40582, 40696, 40811, 40925, 41039, 
41153, 41267, 41381, 41494, 41608, 41721, 41835, 41948, 
42061, 42174, 42287, 42400, 42512, 42625, 42737, 42849, 
42961, 43073, 43185, 43297, 43408, 43520, 43631, 43742, 
43853, 43964, 44075, 44185, 44296, 44406, 44516, 44626, 
44736, 44846, 44955, 45064, 45174, 45283, 45391, 45500, 
45609, 45717, 45825, 45933, 46041, 46149, 46256, 46364, 
46471, 46578, 46685, 46791, 46898, 47004, 47110, 47216, 
47322, 47427, 47532, 47637, 47742, 47847, 47952, 48056, 
48160, 48264, 48368, 48471, 48575, 48678, 48781, 48884, 
48986, 49088, 49191, 49292, 49394, 49496, 49597, 49698, 
49799, 49899, 50000, 50100, 50200, 50299, 50399, 50498, 
50597, 50696, 50794, 50893, 50991, 51088, 51186, 51283, 
51380, 51477, 51574, 51670, 51767, 51862, 51958, 52053, 
52149, 52244, 52338, 52433, 52527, 52621, 52714, 52808, 
52901, 52994, 53086, 53178, 53270, 53362, 53454, 53545, 
53636, 53727, 53817, 53907, 53997, 54087, 54176, 54265, 
54354, 54443, 54531, 54619, 54706, 54794, 54881, 54968, 
55054, 55141, 55226, 55312, 55397, 55483, 55567, 55652, 
55736, 55820, 55903, 55987, 56070, 56152, 56235, 56317, 
56399, 56480, 56561, 56642, 56723, 56803, 56883, 56962, 
57042, 57121, 57199, 57278, 57356, 57433, 57511, 57588, 
57665, 57741, 57817, 57893, 57968, 58044, 58118, 58193, 
58267, 58341, 58414, 58487, 58560, 58633, 58705, 58777, 
58848, 58919, 58990, 59061, 59131, 59200, 59270, 59339, 
59408, 59476, 59544, 59612, 59679, 59746, 59813, 59879, 
59945, 60011, 60076, 60141, 60205, 60270, 60333, 60397, 
60460, 60523, 60585, 60647, 60709, 60770, 60831, 60892, 
60952, 61012, 61071, 61130, 61189, 61248, 61306, 61363, 
61421, 61478, 61534, 61590, 61646, 61701, 61756, 61811, 
61865, 61919, 61973, 62026, 62079, 62131, 62183, 62235, 
62286, 62337, 62388, 62438, 62487, 62537, 62586, 62634, 
62682, 62730, 62778, 62825, 62871, 62918, 62963, 63009, 
63054, 63099, 63143, 63187, 63230, 63273, 63316, 63358, 
63400, 63442, 63483, 63524, 63564, 63604, 63643, 63683, 
63721, 63760, 63797, 63835, 63872, 63909, 63945, 63981, 
64017, 64052, 64086, 64121, 64154, 64188, 64221, 64254, 
64286, 64318, 64349, 64380, 64411, 64441, 64471, 64500, 
64529, 64558, 64586, 64614, 64641, 64668, 64694, 64720, 
64746, 64771, 64796, 64821, 64845, 64868, 64891, 64914, 
64937, 64959, 64980, 65001, 65022, 65042, 65062, 65081, 
65100, 65119, 65137, 65155, 65172, 65189, 65206, 65222, 
65237, 65253, 65267, 65282, 65296, 65309, 65322, 65335, 
65347, 65359, 65371, 65382, 65392, 65402, 65412, 65421, 
65430, 65439, 65447, 65455, 65462, 65469, 65475, 65481, 
65486, 65491, 65496, 65500, 65504, 65508, 65511, 65513, 
65515, 65517, 65518, 65519, 65519, 65519, 65519, 65518, 
65517, 65515, 65513, 65511, 65508, 65504, 65500, 65496, 
65491, 65486, 65481, 65475, 65469, 65462, 65455, 65447, 
65439, 65430, 65421, 65412, 65402, 65392, 65382, 65371, 
65359, 65347, 65335, 65322, 65309, 65296, 65282, 65267, 
65253, 65237, 65222, 65206, 65189, 65172, 65155, 65137, 
65119, 65100, 65081, 65062, 65042, 65022, 65001, 64980, 
64959, 64937, 64914, 64891, 64868, 64845, 64821, 64796, 
64771, 64746, 64720, 64694, 64668, 64641, 64614, 64586, 
64558, 64529, 64500, 64471, 64441, 64411, 64380, 64349, 
64318, 64286, 64254, 64221, 64188, 64154, 64121, 64086, 
64052, 64017, 63981, 63945, 63909, 63872, 63835, 63797, 
63760, 63721, 63683, 63643, 63604, 63564, 63524, 63483, 
63442, 63400, 63358, 63316, 63273, 63230, 63187, 63143, 
63099, 63054, 63009, 62963, 62918, 62871, 62825, 62778, 
62730, 62682, 62634, 62586, 62537, 62487, 62438, 62388, 
62337, 62286, 62235, 62183, 62131, 62079, 62026, 61973, 
61919, 61865, 61811, 61756, 61701, 61646, 61590, 61534, 
61478, 61421, 61363, 61306, 61248, 61189, 61130, 61071, 
61012, 60952, 60892, 60831, 60770, 60709, 60647, 60585, 
60523, 60460, 60397, 60333, 60270, 60205, 60141, 60076, 
60011, 59945, 59879, 59813, 59746, 59679, 59612, 59544, 
59476, 59408, 59339, 59270, 59200, 59131, 59061, 58990, 
58919, 58848, 58777, 58705, 58633, 58560, 58487, 58414, 
58341, 58267, 58193, 58118, 58044, 57968, 57893, 57817, 
57741, 57665, 57588, 57511, 57433, 57356, 57278, 57199, 
57121, 57042, 56962, 56883, 56803, 56723, 56642, 56561, 
56480, 56399, 56317, 56235, 56152, 56070, 55987, 55903, 
55820, 55736, 55652, 55567, 55483, 55397, 55312, 55226, 
55141, 55054, 54968, 54881, 54794, 54706, 54619, 54531, 
54443, 54354, 54265, 54176, 54087, 53997, 53907, 53817, 
53727, 53636, 53545, 53454, 53362, 53270, 53178, 53086, 
52994, 52901, 52808, 52714, 52621, 52527, 52433, 52338, 
52244, 52149, 52053, 51958, 51862, 51767, 51670, 51574, 
51477, 51380, 51283, 51186, 51088, 50991, 50893, 50794, 
50696, 50597, 50498, 50399, 50299, 50200, 50100, 50000, 
49899, 49799, 49698, 49597, 49496, 49394, 49292, 49191, 
49088, 48986, 48884, 48781, 48678, 48575, 48471, 48368, 
48264, 48160, 48056, 47952, 47847, 47742, 47637, 47532, 
47427, 47322, 47216, 47110, 47004, 46898, 46791, 46685, 
46578, 46471, 46364, 46256, 46149, 46041, 45933, 45825, 
45717, 45609, 45500, 45391, 45283, 45174, 45064, 44955, 
44846, 44736, 44626, 44516, 44406, 44296, 44185, 44075, 
43964, 43853, 43742, 43631, 43520, 43408, 43297, 43185, 
43073, 42961, 42849, 42737, 42625, 42512, 42400, 42287, 
42174, 42061, 41948, 41835, 41721, 41608, 41494, 41381, 
41267, 41153, 41039, 40925, 40811, 40696, 40582, 40467, 
40353, 40238, 40123, 40008, 39893, 39778, 39663, 39548, 
39432, 39317, 39201, 39086, 38970, 38854, 38738, 38622, 
38506, 38390, 38274, 38158, 38041, 37925, 37809, 37692, 
37576, 37459, 37342, 37225, 37109, 36992, 36875, 36758, 
36641, 36524, 36407, 36289, 36172, 36055, 35938, 35820, 
35703, 35585, 35468, 35350, 35233, 35115, 34998, 34880, 
34762, 34645, 34527, 34409, 34292, 34174, 34056, 33938, 
33820, 33702, 33585, 33467, 33349, 33231, 33113, 32995, 
32877, 32760, 32642, 32524, 32406, 32288, 32170, 32052, 
31934, 31817, 31699, 31581, 31463, 31345, 31227, 31110, 
30992, 30874, 30757, 30639, 30521, 30404, 30286, 30169, 
30051, 29934, 29816, 29699, 29581, 29464, 29347, 29230, 
29112, 28995, 28878, 28761, 28644, 28527, 28410, 28294, 
28177, 28060, 27943, 27827, 27710, 27594, 27478, 27361, 
27245, 27129, 27013, 26897, 26781, 26665, 26549, 26433, 
26318, 26202, 26087, 25971, 25856, 25741, 25626, 25511, 
25396, 25281, 25166, 25052, 24937, 24823, 24708, 24594, 
24480, 24366, 24252, 24138, 24025, 23911, 23798, 23684, 
23571, 23458, 23345, 23232, 23119, 23007, 22894, 22782, 
22670, 22558, 22446, 22334, 22222, 22111, 21999, 21888, 
21777, 21666, 21555, 21444, 21334, 21223, 21113, 21003, 
20893, 20783, 20673, 20564, 20455, 20345, 20236, 20128, 
20019, 19910, 19802, 19694, 19586, 19478, 19370, 19263, 
19155, 19048, 18941, 18834, 18728, 18621, 18515, 18409, 
18303, 18197, 18092, 17987, 17882, 17777, 17672, 17567, 
17463, 17359, 17255, 17151, 17048, 16944, 16841, 16738, 
16635, 16533, 16431, 16328, 16227, 16125, 16023, 15922, 
15821, 15720, 15620, 15519, 15419, 15319, 15220, 15120, 
15021, 14922, 14823, 14725, 14626, 14528, 14431, 14333, 
14236, 14139, 14042, 13945, 13849, 13752, 13657, 13561, 
13466, 13370, 13275, 13181, 13086, 12992, 12898, 12805, 
12711, 12618, 12525, 12433, 12341, 12249, 12157, 12065, 
11974, 11883, 11792, 11702, 11612, 11522, 11432, 11343, 
11254, 11165, 11076, 10988, 10900, 10813, 10725, 10638, 
10551, 10465, 10378, 10293, 10207, 10122, 10036, 9952, 
9867, 9783, 9699, 9616, 9532, 9449, 9367, 9284, 
9202, 9120, 9039, 8958, 8877, 8796, 8716, 8636, 
8557, 8477, 8398, 8320, 8241, 8163, 8086, 8008, 
7931, 7854, 7778, 7702, 7626, 7551, 7475, 7401, 
7326, 7252, 7178, 7105, 7032, 6959, 6886, 6814, 
6742, 6671, 6600, 6529, 6458, 6388, 6319, 6249, 
6180, 6111, 6043, 5975, 5907, 5840, 5773, 5706, 
5640, 5574, 5508, 5443, 5378, 5314, 5249, 5186, 
5122, 5059, 4996, 4934, 4872, 4810, 4749, 4688, 
4627, 4567, 4507, 4448, 4389, 4330, 4271, 4213, 
4156, 4098, 4041, 3985, 3929, 3873, 3818, 3763, 
3708, 3654, 3600, 3546, 3493, 3440, 3388, 3336, 
3284, 3233, 3182, 3131, 3081, 3032, 2982, 2933, 
2885, 2837, 2789, 2741, 2694, 2648, 2601, 2556, 
2510, 2465, 2420, 2376, 2332, 2289, 2246, 2203, 
2161, 2119, 2077, 2036, 1995, 1955, 1915, 1876, 
1836, 1798, 1759, 1722, 1684, 1647, 1610, 1574, 
1538, 1502, 1467, 1433, 1398, 1365, 1331, 1298, 
1265, 1233, 1201, 1170, 1139, 1108, 1078, 1048, 
1019, 990, 961, 933, 905, 878, 851, 825, 
799, 773, 748, 723, 698, 674, 651, 628, 
605, 582, 560, 539, 518, 497, 477, 457, 
438, 419, 400, 382, 364, 347, 330, 313, 
297, 282, 266, 252, 237, 223, 210, 197, 
184, 172, 160, 148, 137, 127, 117, 107, 
98, 89, 80, 72, 64, 57, 50, 44, 
38, 33, 28, 23, 19, 15, 11, 8, 
6, 4, 2, 1, 0, 0, 0, 0, 
1, 2, 4, 6, 8, 11, 15, 19, 
23, 28, 33, 38, 44, 50, 57, 64, 
72, 80, 89, 98, 107, 117, 127, 137, 
148, 160, 172, 184, 197, 210, 223, 237, 
252, 266, 282, 297, 313, 330, 347, 364, 
382, 400, 419, 438, 457, 477, 497, 518, 
539, 560, 582, 605, 628, 651, 674, 698, 
723, 748, 773, 799, 825, 851, 878, 905, 
933, 961, 990, 1019, 1048, 1078, 1108, 1139, 
1170, 1201, 1233, 1265, 1298, 1331, 1365, 1398, 
1433, 1467, 1502, 1538, 1574, 1610, 1647, 1684, 
1722, 1759, 1798, 1836, 1876, 1915, 1955, 1995, 
2036, 2077, 2119, 2161, 2203, 2246, 2289, 2332, 
2376, 2420, 2465, 2510, 2556, 2601, 2648, 2694, 
2741, 2789, 2837, 2885, 2933, 2982, 3032, 3081, 
3131, 3182, 3233, 3284, 3336, 3388, 3440, 3493, 
3546, 3600, 3654, 3708, 3763, 3818, 3873, 3929, 
3985, 4041, 4098, 4156, 4213, 4271, 4330, 4389, 
4448, 4507, 4567, 4627, 4688, 4749, 4810, 4872, 
4934, 4996, 5059, 5122, 5186, 5249, 5314, 5378, 
5443, 5508, 5574, 5640, 5706, 5773, 5840, 5907, 
5975, 6043, 6111, 6180, 6249, 6319, 6388, 6458, 
6529, 6600, 6671, 6742, 6814, 6886, 6959, 7032, 
7105, 7178, 7252, 7326, 7401, 7475, 7551, 7626, 
7702, 7778, 7854, 7931, 8008, 8086, 8163, 8241, 
8320, 8398, 8477, 8557, 8636, 8716, 8796, 8877, 
8958, 9039, 9120, 9202, 9284, 9367, 9449, 9532, 
9616, 9699, 9783, 9867, 9952, 10036, 10122, 10207, 
10293, 10378, 10465, 10551, 10638, 10725, 10813, 10900, 
10988, 11076, 11165, 11254, 11343, 11432, 11522, 11612, 
11702, 11792, 11883, 11974, 12065, 12157, 12249, 12341, 
12433, 12525, 12618, 12711, 12805, 12898, 12992, 13086, 
13181, 13275, 13370, 13466, 13561, 13657, 13752, 13849, 
13945, 14042, 14139, 14236, 14333, 14431, 14528, 14626, 
14725, 14823, 14922, 15021, 15120, 15220, 15319, 15419, 
15519, 15620, 15720, 15821, 15922, 16023, 16125, 16227, 
16328, 16431, 16533, 16635, 16738, 16841, 16944, 17048, 
17151, 17255, 17359, 17463, 17567, 17672, 17777, 17882, 
17987, 18092, 18197, 18303, 18409, 18515, 18621, 18728, 
18834, 18941, 19048, 19155, 19263, 19370, 19478, 19586, 
19694, 19802, 19910, 20019, 20128, 20236, 20345, 20455, 
20564, 20673, 20783, 20893, 21003, 21113, 21223, 21334, 
21444, 21555, 21666, 21777, 21888, 21999, 22111, 22222, 
22334, 22446, 22558, 22670, 22782, 22894, 23007, 23119, 
23232, 23345, 23458, 23571, 23684, 23798, 23911, 24025, 
24138, 24252, 24366, 24480, 24594, 24708, 24823, 24937, 
25052, 25166, 25281, 25396, 25511, 25626, 25741, 25856, 
25971, 26087, 26202, 26318, 26433, 26549, 26665, 26781, 
26897, 27013, 27129, 27245, 27361, 27478, 27594, 27710, 
27827, 27943, 28060, 28177, 28294, 28410, 28527, 28644, 
28761, 28878, 28995, 29112, 29230, 29347, 29464, 29581, 
29699, 29816, 29934, 30051, 30169, 30286, 30404, 30521, 
30639, 30757, 30874, 30992, 31110, 31227, 31345, 31463, 
31581, 31699, 31817, 31934, 32052, 32170, 32288, 32406, 
32524, 32642, };
