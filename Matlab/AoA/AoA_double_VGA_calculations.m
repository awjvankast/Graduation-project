close all; clear; clc;
figure

max_resolution_table = [];
time = []; %time in microseconds for entire sweep to take place based on
% the settling time of the VGA
total_phase = [];
for h = 1:3
    % Gain range and steps of VGA's
    gain_res_table = [0.1, 0.5, .1];
    gain_dB = 0:gain_res_table(h):23;
   
    % Desired min resolution of the sweep to be performed
    minimum_desired_resolution = 1;
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

    total_phase_simple = [steps, steps+90];
    total_phase_inter = unique(total_phase_simple);

    % Eliminate steps based on minimum resolution
    step = total_phase_inter(1);
    total_phase=[];
    for k = 1:length(total_phase_inter)
            % If the step is achieved, save to new 
            if step + minimum_desired_resolution <= total_phase_inter(k)
                total_phase = [total_phase, total_phase_inter(k)];
                step = total_phase_inter(k);
            end
    end

    % 500 ns gain settling time from spec sheet
    time(h) = length(total_phase)*0.5 ;
    max_resolution_table(h) = max(diff(total_phase));
%
    z = plot(total_phase,1:length(total_phase),'-o','MarkerSize',3);
    z.MarkerFaceColor = z.Color;
    
    %text(0,100+h*10,sprintf("maximum resolution: %f",resolution));
    %text(0,105+h*10,sprintf("Amount of time for entire sweep: %.2f microseconds",time));
    % Indicates the step size of the gain if two VGA's would be connected to a
    % combiner with one VGA 0 degree phase shift and one VGA 180 degrees phase
    % shifted

    hold on;
end
    lgd = legend([sprintf("VGA gain step of %.1f dB",gain_res_table(1)),...
        sprintf("VGA gain step of %.1f dB",gain_res_table(2)),...
        sprintf("VGA gain step of %.1f dB",gain_res_table(3))],'Location','NW');
    textSize = 16;
    lgd.FontSize = 14;
    ylabel("Gain steps [-]",'Fontsize',textSize);
    xlabel("Achieved phase delay [°]",'Fontsize',textSize);
    grid on;
    

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
theta = 0;
gamma = 40;

Tx1_signal = 1*sin(x+total_phase'/360*2*pi+gamma/360*2*pi);
Tx2_signal_temp = 1*sin(x+theta/360*2*pi);
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
   %plot(x, Tx_total(indexMinColumn,:));
   error = abs(180-gamma-closest_phase);
   actual_phase_diff = 180-gamma;
else
   closest_phase = total_phase(indexMaxRow(indexMaxColumn));
   %plot(x, Tx_total(indexMaxColumn,:));
   error = abs(theta-closest_phase);
   actual_phase_diff = theta;
end

figure

max_last_step = 0;
amp_diff = 0;
lowest_change_amp = 1;
for k = 1:length(Tx_total(:,1))
    sin_plot = plot(x,[Tx1_signal(k,:);Tx2_signal_temp;Tx_total(k,:)]);
    legend(["Tx1 signal with variable phase delay", "Tx2 signal", "Tx1 + Tx2"]);
    text(0,0,sprintf("Phase difference %.2f",total_phase(k)));
    amp_diff = max(Tx_total(k,:))-max_last_step;
    if abs(amp_diff)<abs(lowest_change_amp)
        lowest_change_amp = amp_diff;
    end
    text(0,0.2,sprintf("Amp change since last step: %.5f",amp_diff));
    grid on;
    pause(0.001);
    
    max_last_step = max(Tx_total(k,:));
    if k ~= length(Tx_total(:,1))
        delete(sin_plot);
    end
end

disp(sprintf("Lowest amplitude change when sweeping: %.5f",lowest_change_amp));
fprintf("Theoretical error: %f \n",error)



text(0,1,sprintf("Actual phase difference antenna: %.2f \n Closest phase: %.2f degree\n Error: %.2f degree ",actual_phase_diff, closest_phase,error)); 

[~,indexMin] = min(Tx_total);   


