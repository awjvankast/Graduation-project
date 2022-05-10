close all; clear; clc;

%% Data import

load("..\data\LoRa_trilateration_test_2\Blue_Rx_t2.mat");
Rx_blue = Rxarr;

load("..\data\LoRa_trilateration_test_2\Green_Rx_t2.mat");
Rx_green = Rxarr;

load("..\data\LoRa_trilateration_test_2\Yellow_Rx_t2.mat");
Rx_yellow = Rxarr;
Rx_yellow(350,:) = [];

%% Making initial plots

field_plot = figure;
field_plot.WindowState = 'maximized';
field_fig = gca;
field_fig.FontSize = 16;
hold on;

dmod_plot = figure;
dmod_plot.WindowState = 'maximized';
dmod_fig = gca;
dmod_fig.FontSize = 16;
hold on;

%% Coordinate system
% see drawing in notebook for reference

tile_width = mean([252.8, 252, 253])/5;
tile_length = mean([300.5,301,301.5])/3;

% Rx coordinates in order of green, yellow, blue
Rx_coord = [0,0; 27, 0; 13, 8];
Rx_coord(:,1) = Rx_coord(:,1).*tile_width;
Rx_coord(:,2) = Rx_coord(:,2).*tile_length;

Tx_coord = [15,2; 13,2; 11,2; 9,2; 9,3; 7,3; 7,4; 7,5; 7,6; 9,6; 11,6; 13,6; 13,5; ...
    15,5; 17,5; 19,5; 19,6; 21,6; 23,6; 23,5; 21,5; 21,4; 19,4; 19,3; 17,3; 17,2; 15,2; 15,1];
Tx_coord(:,1) = Tx_coord(:,1).*tile_width;
Tx_coord(:,2) = Tx_coord(:,2).*tile_length;

dmod_dist = (2:2:16).*tile_width;

%% Plot distance model
% Conclusion: probably need to characterize every single antenna like this
% As the yellow antenna seems to work okay but the other ones are garbage
% with this model
figure(dmod_plot);
dmod_sig = Rx_yellow(466:502,:);
dmod_RSSI = zeros(15,8);

for k = 1:length(dmod_dist)-1
   if k == 1
       ID = dmod_sig(1,1);
       index_ID = 1;
   else
       index_ID = index_ID + sum(logic_ID(:,1));
       ID = dmod_sig(index_ID, 1);
   end
   logic_ID = ismember(dmod_sig,ID);
   dmod_RSSI(1:sum(logic_ID(:,1)),k) = dmod_sig(logic_ID(:,1),12);
end
dmod_RSSI_mean = sum(dmod_RSSI) ./ sum(~(dmod_RSSI==0));
pos_dmod_RSSI_mean = - dmod_RSSI_mean;
 
figure(dmod_plot);
plot(dmod_dist/100,dmod_RSSI_mean,'-*');
grid on; ylabel('RSSI [dBm]'); xlabel('Distance [m]');

%% Trilateration data matching

% Some packets are not received by a single Rx but are by the other Rx's 

% The RSSI data of the first and last packets are contaminated by the person standing
% next to the Tx so filter the data on this

% Use only IMU data when the value changes significantly

Rx_blue_bin = Rx_data_bin(Rx_blue);
Rx_yellow_bin = Rx_data_bin(Rx_yellow);
Rx_yellow_bin(28:34,:) = []; % Data used for the dmod tests, not relevant here
Rx_green_bin = Rx_data_bin(Rx_green);

Rx_blue_fitted = line_fit(-Rx_blue_bin(:,2));
Rx_green_fitted = line_fit(-Rx_green_bin(:,2));
Rx_yellow_fitted = line_fit(-Rx_yellow_bin(:,2));

Rx_fitted = [ Rx_blue_fitted'; Rx_yellow_fitted'; Rx_green_fitted', zeros(1,...
    length(Rx_blue_fitted)-length(Rx_green_fitted))]';

%% Magnetic north phone calculation

% angle clockwise from horizontal position plot field
angle = 360-263;

compass_coord = [0,7];
hypotheneuse = 1;
compass_arrow_end = [sind(angle-90)*hypotheneuse, cosd(angle-90)*hypotheneuse];
figure(field_plot);
quiv = quiver(compass_coord(1),compass_coord(2), compass_arrow_end(1),...
    compass_arrow_end(2), 'color','r','LineWidth',1.5);
compass_text = text(0.1,7.2,"North",'Fontsize',16,'color','r');

%% IMU data

% Process data of IMU where we have:
% Accelerometer -> [g]
% Gyroscope -> [degrees per second]
% Magnetometer -> [micro Tesla]



%% Trilateration data plotting

figure(field_plot);
ylabel('Y coordinates [m]'); xlabel('X coordinates [m]'); grid on;
xlim([-1 14.5]); ylim([ -1 9]);
Rx_plot = plot(Rx_coord(:,1)/100,Rx_coord(:,2)/100,'^','MarkerSize',10,...
    'MarkerEdgeColor',[40/255 156/255 86/255],'MarkerFaceColor',...
    [135/255 215/255 167/255]);

for k = 1:length(Rx_blue_bin(:,1))
        Tx_plot = plot( Tx_coord(k,1)/100, Tx_coord(k,2)/100 ,'d','MarkerSize',10,...
    'MarkerEdgeColor','red','MarkerFaceColor',[1 .6 .6]);

    circle_handle = circle(Rx_coord(:,1)'/100,Rx_coord(:,2)'/100,Rx_fitted(k,:)/100);
    
    % Least Squares: 
    % Take as input the distances of the inverse square model in a 3*1 matrix
    % Output a single point which minimizes the distances
    ls_point = least_squares_circles(Rx_coord, Rx_fitted(k,:));
    ls_marker = plot(ls_point(1)/100,ls_point(2)/100,'s','MarkerSize',11,...
        'MarkerEdgeColor',[0.121, 0.129, 0.576],'MarkerFaceColor',[0.521, 0.525, 0.839]);
    
    legend([Tx_plot, Rx_plot, ls_marker], 'Tx position', 'Rx position',...
        'Least Squares Estimate');

    pause(.1);
    delete(Tx_plot);
    delete(circle_handle);
    delete(ls_marker);
end

%% Line fitting model
% The cftool was used to generate a fit of the model
% f(x) = a*x^b
% Positive RSSI values needs to be inputted
function fit = line_fit(x)
    fit = 0.606 .*x.^1.756-278.8; % from cftool
    
    % self made
    %fit = 0.001936.*x.^3.02;
end


