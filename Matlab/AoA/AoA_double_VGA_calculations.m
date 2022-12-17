close all; clear; clc;
% Make a plot as well for the different minimum resolutions

    % Desired min resolution of the sweep to be performed
minimum_desired_resolution_table = [1 2 5];
gain_res_table = [.5];
max_resolution_table = [];
time = []; %time in microseconds for entire sweep to take place based on
% the settling time of the VGA
total_phase = [];
for h = 1:3
    % Gain range and steps of VGA's
    if length(minimum_desired_resolution_table) ~= 1
        minimum_desired_resolution = minimum_desired_resolution_table(h);
    else
        minimum_desired_resolution = minimum_desired_resolution_table;
    end

    if length(gain_res_table) == 1
        gain_dB = 0:gain_res_table:31.5;
    else   
        if h == 3
            gain_dB = 0:gain_res_table(h):23;
        elseif h == 2
             gain_dB = 0:gain_res_table(h):31.5;
        else
            gain_dB = 2:gain_res_table(h):18;
        end
    end
  
    gain_pow = db2pow(gain_dB);

    %A = repmat(gain_pow,24,1);
    A = gain_pow;
    B = A';
    one_eighty_degree = repmat( gain_pow, 24, 1)';

    phase_matrix = atan(B./A)./(2.*pi).*360;

    [steps,row_index] = min(phase_matrix);
    [steps,column_index] = min(steps);
    alpha(1)  = sqrt(B(row_index(column_index))^2+B(column_index)^2);
    phase_matrix(row_index(column_index),column_index) = -1e9;
    for k = 1:length(gain_dB)^2-1
        [temp, row_index] = min( abs( phase_matrix - steps(k) ) );
        [temp2, column_index] = min(temp);
        steps(k+1) = phase_matrix(row_index(column_index), column_index);

        alpha(k+1)  = sqrt(B(column_index)^2+B(row_index(column_index))^2);

        phase_matrix( row_index(column_index), column_index ) = -1e9;
    end

    total_phase_simple = [steps, steps+90];
    alpha = [alpha, alpha];
    [total_phase_inter,unique_index] = unique(total_phase_simple);
    alpha_unique = alpha(unique_index);

    % Eliminate steps based on minimum resolution
    step = total_phase_inter(1);
    alpha_fin = alpha_unique(1);
    total_phase=total_phase_inter(1);
    for k = 1:length(total_phase_inter)
            % If the step is achieved, save to new 
            if step + minimum_desired_resolution <= total_phase_inter(k)
                total_phase = [total_phase, total_phase_inter(k)];
                step = total_phase_inter(k);
                alpha_fin = [alpha_fin, alpha_unique(k)];
            end
    end

    % 500 ns gain settling time from spec sheet
    time(h) = length(total_phase)*0.5 ;
    max_resolution_table(h) = max(diff(total_phase))

    z = plot(total_phase,1:length(total_phase),'-o','MarkerSize',3);
    z.MarkerFaceColor = z.Color;
    
    %text(0,100+h*10,sprintf("maximum resolution: %f",resolution));
    %text(0,105+h*10,sprintf("Amount of time for entire sweep: %.2f microseconds",time));
    % Indicates the step size of the gain if two VGA's would be connected to a
    % combiner with one VGA 0 degree phase shift and one VGA 180 degrees phase
    % shifted

    hold on;
end
    if length(gain_res_table) ~= 1
    lgd = legend([sprintf("%.3f dB VGA gain step",gain_res_table(1)),...
        sprintf("%.1f dB VGA gain step",gain_res_table(2)),...
        sprintf("%.1f dB VGA gain step",gain_res_table(3))],'Location','NW');
    else 
            lgd = legend([sprintf("Minimum resolution of %.1f°",minimum_desired_resolution_table(1)),...
        sprintf("Minimum resolution of %.1f°",minimum_desired_resolution_table(2)),...
        sprintf("Minimum resolution of %.1f°",minimum_desired_resolution_table(3))],'Location','NW');
    end
    textSize = 16;
    lgd.FontSize = 14;
    ylabel("Gain steps [-]",'Fontsize',textSize);
    xlabel("\phi_{sweep} [°]",'Fontsize',textSize);
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
theta = 40;
gamma = 0;

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
    sin_plot = plot(x,[Tx1_signal(k,:);alpha(k)*Tx2_signal_temp;Tx1_signal + alpha(k)*Tx2_signal(k,:)]);

    rms_plot = yline(rms(Tx_total(k,:)));

    legend(["Tx1 signal with variable phase delay", "Tx2 signal", "Tx1 + Tx2"]);
    text(0,0,sprintf("Phase difference %.2f",total_phase(k)));
    rms_diff = rms(Tx_total(k,:))-max_last_step;
    if abs(rms_diff)<abs(lowest_change_amp)
        lowest_change_amp = rms_diff;
    end
    text(0,0.2,sprintf("Amp change since last step: %.5f",rms_diff));
    %ylim([-1 2.5]);
    grid on;
    %imwrite(gcf,'testAnimated.gif');
    %exportgraphics(gcf,'testAnimated.gif','Append',true);
    pause(.0001);
    
    max_last_step = rms(Tx_total(k,:));
    if k ~= length(Tx_total(:,1))
        delete(sin_plot);
        delete(rms_plot)
    end
end

disp(sprintf("Lowest amplitude change when sweeping: %.5f",lowest_change_amp));
fprintf("Theoretical error: %f \n",error)



%text(0,1,sprintf("Actual phase difference antenna: %.2f \n Closest phase: %.2f degree\n Error: %.2f degree ",actual_phase_diff, closest_phase,error)); 

[~,indexMin] = min(Tx_total);   


