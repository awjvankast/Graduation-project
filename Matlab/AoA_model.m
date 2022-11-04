close all; clear all; clc;

% What kind of attenuator do I need from Antonio?

% Assume paths except the 90 degree phase shift path are of equal
% electrical phase shift length
resolution = 1e4;
f = 433e6;
T = 1/f;
t = linspace(0, 3*T, resolution);

phase_shift_incoming = pi/3;

Tx1_signal = sin(t*2*pi/T);
Tx2_signal = sin(t*2*pi/T+ phase_shift_incoming);

f = figure;
f.Position = [100 100 1200 650];
for k = 1:64 
    
subplot(2,2,1);
plot(t, [Tx1_signal; Tx2_signal]);
grid on;
legend("Tx1", "Tx2");


%% Tx1 path
subplot(2,2,2);
static_attenuation_dB = -10;
variable_attenuation_dB = 0:-0.5:-31.5;

chosen_att_index = k;

Tx1_0D_path = repmat( Tx1_signal * db2pow(static_attenuation_dB), size(variable_attenuation_dB,2), 1 );
Tx1_90D_path = sin(t*2*pi/T-pi/2) .* db2pow(variable_attenuation_dB)';

Tx1_combined = Tx1_0D_path + Tx1_90D_path;

Tx_plot = plot(t, [Tx1_0D_path; Tx1_90D_path(chosen_att_index,:); Tx1_combined(chosen_att_index,:)]);
grid on;
legend("Tx1 0 degrees shifted", "Tx1 90 degrees shifted","Tx1 combined");

%% Combined path
subplot(2,2,3);
Tx12 = Tx1_combined + repmat(Tx2_signal, size(variable_attenuation_dB,2),1);
Tx12_plot = plot(t, Tx12(chosen_att_index,:));
grid on;

legend( sprintf("%d",k));
pause(0.001);
delete(Tx_plot);
delete(Tx12_plot);
end


