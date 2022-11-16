close all; clear; clc;

% What kind of attenuator do I need from Antonio?

% Assume paths except the 90 degree phase shift path are of equal
% electrical phase shift length
resolution = 1e4;
f = 433e6;
T = 1/f;
t = linspace(0, 3*T, resolution);

phase_shift_degree = -40;
phase_shift_incoming = phase_shift_degree/360*2*pi;
    
Tx1_signal = sin(t*2*pi/T);
Tx2_signal = sin(t*2*pi/T+ phase_shift_incoming);

f = figure;
f.Position = [100 100 1200 650];

subplot(2,2,1);
plot(t, [Tx1_signal; Tx2_signal]);
grid on;
legend("Tx1", "Tx2");

subplot(2,2,2);
static_attenuation_dB = -5;
variable_attenuation_dB = 0:-0.5:-31.5;

Tx1_0D_path = repmat( Tx1_signal * db2pow(static_attenuation_dB), size(variable_attenuation_dB,2), 1 );
Tx1_90D_path = sin(t*2*pi/T-pi/2) .* db2pow(variable_attenuation_dB)';
Tx1_combined = Tx1_0D_path + Tx1_90D_path;

Tx1_gain_compensated = 1./max(Tx1_combined,[],2) .* Tx1_combined;
Tx12 = Tx1_gain_compensated + repmat(Tx2_signal, size(variable_attenuation_dB,2),1);

for k = 1:64 
%% Tx1 paths
subplot(2,2,2);
Tx_plot = plot(t, [Tx1_0D_path; Tx1_90D_path(k,:); Tx1_combined(k,:)]);
grid on;
legend("Tx1 0 degrees shifted", sprintf("Tx1 90 degrees shifted with %.2f dB att",variable_attenuation_dB(k)),"Tx1 combined");

%% Combined path
subplot(2,2,3);

Tx_combined_plot = plot(t,[ Tx1_gain_compensated(k,:);Tx2_signal; Tx12(k,:)]);
grid on;
legend( "Tx1 combined with gain correction", "Tx2", "Total output");
ylim([-1.5, 4]);
pause(0.001);
if k ~= 64
delete(Tx_plot);
delete(Tx_combined_plot);
end


end


