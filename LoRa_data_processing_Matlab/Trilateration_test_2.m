close all; clear; clc;

%% Data import

load("..\data\LoRa_trilateration_test_2\Blue_Rx_t2.mat");
Rx_blue = Rxarr;

load("..\data\LoRa_trilateration_test_2\Green_Rx_t2.mat");
Rx_green = Rxarr;

load("..\data\LoRa_trilateration_test_2\Yellow_Rx_t2.mat");
Rx_yellow = Rxarr;

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

tile_width = mean([300.5,301,301.5])/3;
tile_length = mean([252.8, 252, 253])/5;

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

figure(dmod_plot);
dmod_sig = Rx_yellow(467:503,:);
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
 
figure(dmod_plot);
plot(dmod_dist/100,dmod_RSSI_mean,'-*');
grid on; ylabel('RSSI [dBm]'); xlabel('Distance [m]');

%% Trilateration data matching
close all;
% Some packets are not received by a single Rx but are by the other Rx's 

% The RSSI data of the first and last packets are contaminated by the person standing
% next to the Tx so filter the data on this

% Use only IMU data when the value changes significantly

% TODO: this one is not ordered correctly, need to fix it
[no_ID, ~] = groupcounts(Rx_blue(:,1));

Blue_ID_received = unique(Rx_blue(:,1),'stable');
largest_no_ID = max(no_ID);

Blue_RSSI = zeros(length(no_ID), largest_no_ID);

for k = 1:length(Blue_ID_received)
    index_ID = Blue_ID_received(k)==Rx_blue;
    RSSI = Rx_blue(index_ID(:,1),12);
    Blue_RSSI(k,1:length(RSSI)+1) = [Blue_ID_received(k) RSSI'];
end










