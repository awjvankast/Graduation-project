close all; clear; clc;

gain_dB = 0:1:23;
gain_pow = db2pow(gain_dB);

%A = repmat(gain_pow,24,1);
A = gain_pow;
B = A';
one_eighty_degree = repmat( gain_pow, 24, 1)';

phase_matrix = atan(B./A)./(2.*pi).*360;

[steps,row_index] = min(phase_matrix);
[steps,column_index] = min(steps);
phase_matrix(row_index(column_index),column_index) = -1e9;

for k = 1:length(gain_dB)^2-1
    [temp, row_index] = min( abs( phase_matrix - steps(k) ) );
    [temp2, column_index] = min(temp);
    steps(k+1) = phase_matrix(row_index(column_index), column_index);
    phase_matrix( row_index(column_index), column_index ) = -1e9;
end

resolution = max(diff(steps));
total_phase_simple = [steps, steps+90];
total_phase = unique(total_phase_simple);
% 500 ns gain settling time from spec sheet
time = length(total_phase)*0.5 ;

plot(1:length(total_phase),total_phase);
text(0,20,sprintf("maximum resolution: %f",resolution));
text(0,50,sprintf("Amount of time for entire sweep: %.2f microseconds",time));
grid on;
% Indicates the step size of the gain if two VGA's would be connected to a
% combiner with one VGA 0 degree phase shift and one VGA 180 degrees phase
% shifted

legend("Double VGA 0 and 90 degree phase");
ylabel("Achieved phase delay [°]");
xlabel("Gain steps [-]");

%% Check if math is good
% figure
% t = 0:0.1:4*pi;
% A = 0.5; B = 10;
% real_sig = A*  sin(t)+ B * cos(t);
% alpha = sqrt(A^2+B^2);
% beta = atan(B/A);
% math_sig = alpha*sin(t+beta);
% 
% plot(t,[real_sig; math_sig]);

%% Do a test
x = 0:0.1:4*pi;
% Difference in arrival phase
theta = 48;
gamma = 0;

Tx1_signal = sin(x+total_phase'/360*2*pi+gamma/360*2*pi);
Tx2_signal_temp = sin(x+theta/360*2*pi);
Tx2_signal = repmat(Tx2_signal_temp, length(total_phase),1);

Tx_total = Tx1_signal + Tx2_signal;

[tempMax,indexMaxRow] = max(Tx_total);
[tempMax2,indexMaxColumn] = max(tempMax);
Max_index = [indexMaxRow(indexMaxColumn), indexMaxColumn ];

% closest to zero
[tempMin,indexMinRow] = min(abs(Tx_total));
[tempMin2,indexMinColumn] = min(tempMax);
Min_index = [indexMinRow(indexMinColumn), indexMinColumn ];

figure
if Max_index(1) == 1
   closest_phase = total_phase(indexMinRow(indexMinColumn));
   plot(x, Tx_total(indexMinColumn,:));
   error = abs(180-gamma-closest_phase);
   actual_phase_diff = 180-gamma;
else
   closest_phase = total_phase(indexMaxRow(indexMaxColumn));
   plot(x, Tx_total(indexMaxColumn,:));
   error = abs(theta-closest_phase);
   actual_phase_diff = theta;
end



text(0,0,sprintf("Actual phase difference antenna: %.2f \n Closest phase: %.2f degree\n Error: %.2f degree",actual_phase_diff, closest_phase,error)); 

[~,indexMin] = min(Tx_total);   


