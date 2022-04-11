close all; clearvars; clc;

load("..\data\LoRa_trilateration_test_1\Rx1.mat");
Rx1 = filtered_data;

load("..\data\LoRa_trilateration_test_1\Rx2.mat");
Rx2 = filtered_data;

load("..\data\LoRa_trilateration_test_1\Rx3.mat");
Rx3 = filtered_data;

session_1_Rx1 = Rx1(22:361,:);
session_1_Rx2 = Rx2(22:361,:);
session_1_Rx3 = Rx3(22:361,:);

% data_selection = [ 5:16, 18:35, 38:59, 65:81, 84:109, 111:126, 130:150];

%% Making initial plot

f = figure;
f.WindowState = 'maximized';
fig = gca;
fig.FontSize = 16;

%% Free space path loss model

c = 3e8;
f = 433e6;

PtdBm = 20;
Pt = 1e-1;

PrdBm_test = session_1_Rx1(:,3);
Pr1 = 10.^(PrdBm_test./10-3);

d = c*sqrt(Pt)./(4*pi*f*sqrt(Pr1));



%% Coordinate system, 
% see drawing in notebook for reference

% coordinates for Rx1, Rx2 and Rx3 respectively
Rx_coord = [ 0, 14*.23 ; 12*.23, 14*.23 ; 4*0.23, 0 ];
% sequence for transmitter movement
Tx_start_coord = 0.23*[ 6, 8; 6, 10; 6, 12; 8, 12; 8, 14; 10, 14; 10, 12; 10, 10; 8, 10; 8, 8; 8, 6; 6, 6; 6,4; 4,4; 4,6; 4,8; 6,8];

%% Inverse square model
% with distance calculations and unit conversion,

% Take the first measurement with known distance as ground truth
d_ground_truth_temp =  ones(3,2).*Tx_start_coord(1,:) - Rx_coord;
d_ground_truth_temp2 = sqrt(d_ground_truth_temp(:,1).^2+d_ground_truth_temp(:,2).^2);

Pr_dBm = [session_1_Rx1(:,3), session_1_Rx2(:,3), session_1_Rx3(:,3) ];
Pr = dBm2W(Pr_dBm);

Pr_ground_truth_dBm = Pr_dBm(8,:) .* ones(size(Pr));
Pr_ground_truth = Pr(8,:) .* ones(size(Pr));
d_ground_truth = d_ground_truth_temp2' .* ones(size(Pr));

% Scaling factor of attenuation for inverse square model
% Theoretically should be 0.5 but has to be set higher for more accurat
% results
scaling_factor = 1.4;

% Model with power in wats
%d_inv_sqr_model = d_ground_truth .* ( Pr_ground_truth ./ Pr ).^scaling_factor; 

% Model with power in dBm (reciprocal because lower power is higher number)
d_inv_sqr_model = d_ground_truth .* ( Pr_dBm ./ Pr_ground_truth_dBm   ).^scaling_factor;

%% Vector model
% Take the reciprocal of the signal strength as a vector from a point which
% is the center from the three receivers. These vectors should point
% towards their respective receivers. Then calculate the new position.

% First start by calculating the midpoint of three points.
% This can be done by drawing a circle which goes through these 3 points. 
m_AC = Rx_coord(3,:) - Rx_coord(1,:);
m_AC_ = m_AC(2)/m_AC(1);
m_vec = -1/m_AC_;
mid_point = [Rx_coord(1,1) + Rx_coord(3,1) , Rx_coord(1,2) + Rx_coord(3,2)]/2;
b_vec = mid_point(2) - m_vec*mid_point(1);

Dx = (Rx_coord(2,1)^2+Rx_coord(2,2)^2-Rx_coord(1,2)^2+2*b_vec*(Rx_coord(1,2)-Rx_coord(2,2)))/...
     (2*(Rx_coord(2,1)-Rx_coord(1,1)+m_vec*(Rx_coord(2,2)-Rx_coord(1,2))));
Dy = m_vec*Dx + b_vec;
plot(Dx,Dy,'*','Linewidth',16);



%% Map making and plotting

hold on; grid on; xlim([-1 5]); ylim([-1 5]);
ylabel('Y coordinates'); xlabel('X coordinates');

Tx_plot = plot( Tx_start_coord(:,1), Tx_start_coord(:,2) ,'d','MarkerSize',10,...
    'MarkerEdgeColor','red','MarkerFaceColor',[1 .6 .6]);
    
Rx_plot = plot(Rx_coord(:,1),Rx_coord(:,2),'^','MarkerSize',10,...
    'MarkerEdgeColor',[40/255 156/255 86/255],'MarkerFaceColor',...
    [135/255 215/255 167/255]);

legend([Tx_plot, Rx_plot],'Transmitter positions','Receivers');

for k = 1:length(Pr_ground_truth)
    circle_handle = circle(Rx_coord(:,1)',Rx_coord(:,2)',d_inv_sqr_model(k,:));
    count = text(-.75,-.5, "Data entry " + k + "/340",'FontSize',16);
    pause(.2);
    delete(circle_handle);
    delete(count);
end
   
    


 