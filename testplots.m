clear, clc;
clear plot;

valuesF5000 = [24.326
177.935
318.291
608.932
4914.577
375.292
85.477
68.542
39.703
58.281
144.235
86.871
108.419
16.235
5.13
79.537
62.231
34.989
65.783
61.111];

deltaF = 1000:1000:20000;

subplot(1,3,1);
stem(deltaF, valuesF5000);
xlabel('Frequency (Hz)');
ylabel('Magnitude');
title('Target Frequency = 5000');

valuesF10000 = [9.858
111.953
129.471
127.461
83.515
82.347
79.472
134.958
276.647
2373.92
751.767
319.321
249.564
58.406
88.359
52.626
69.13
16.134
109.26
80.032];

subplot(1,3,2);
stem(deltaF, valuesF10000);
xlabel('Frequency (Hz)');
ylabel('Magnitude');
title('Target Frequency = 10000');

deltaV = .075:.075:1.5;

valuesV = [121.697
243.74
359.028
478.026
564.888
708.476
793.774
945.128
1068.34
1134.303
1303.325
1363.351
1551.713
1589.727
1723.291
1904.642
1930.974
2069.105
2180.935
2348.221];

subplot(1,3,3);
plot(deltaV, valuesV);
xlabel('Amplitude (V)');
ylabel('Magnitude');
title('Magnitude with varying V');