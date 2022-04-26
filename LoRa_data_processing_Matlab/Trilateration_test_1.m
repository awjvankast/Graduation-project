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
hold on;

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

marker_plot = plot(Dx, Dy, '+', 'MarkerSize',14,'Linewidth', 1.5,...
    'MarkerEdgeColor','green');

% Making the unit vectors

directional_vec = Rx_coord - [Dx Dy].*ones(3,2);
length_vec = sqrt( sum( directional_vec.^2, 2 ) );
unit_vec = directional_vec ./ length_vec;

inverse_signal_strength =  length_vec(1) - d_inv_sqr_model;

%quiver( ones(3,1).*Dx,ones(3,1).*Dy, unit_vec(:,1), unit_vec(:,2) );

%% Least squares model

syms x y ; syms w [1 3]; syms f [1 3]; syms m [1 3]
syms E(x,y);
syms deriv(x,y); syms deriv_x(x,y); syms deriv_y(x,y);

% Testing data
ft = [0, -.5, -1]; wt = [ -3 0 2]; mt = [ .9 0.75 1.5];
%hold on; grid on;
%circle(wt,ft,mt);

E(x,y) = sum( ( (x-w).^2 + (y-f).^2 - m.^2 ).^2 );
deriv_x(x,y) = diff(E,x);
deriv_y(x,y) = diff(E,y);

% deriv_x_data = subs(deriv_x,[w,f,m],[wt,ft,mt]);
% deriv_y_data = subs(deriv_y,[w,f,m],[wt,ft,mt]);
% 
% solution = vpasolve([deriv_x_data == 0, deriv_y_data == 0]);
% x_ls = vpa(solution.x(5)); y_ls = vpa(solution.y(5));
% plot(x_ls,y_ls,'*');



%% Map making and plotting

grid on; xlim([-1 5]); ylim([-1 5]);
ylabel('Y coordinates'); xlabel('X coordinates');

Tx_plot = plot( Tx_start_coord(:,1), Tx_start_coord(:,2) ,'d','MarkerSize',10,...
    'MarkerEdgeColor','red','MarkerFaceColor',[1 .6 .6]);
    
Rx_plot = plot(Rx_coord(:,1),Rx_coord(:,2),'^','MarkerSize',10,...
    'MarkerEdgeColor',[40/255 156/255 86/255],'MarkerFaceColor',...
    [135/255 215/255 167/255]);

uistack(marker_plot,'top');

for k = 1:length(Pr_ground_truth)
    circle_handle = circle(Rx_coord(:,1)',Rx_coord(:,2)',d_inv_sqr_model(k,:));
    count = text(-.75,-.5, "Data entry " + k + "/340",'FontSize',16);
    
    %% Vector model
    vec_model_x = unit_vec(:,1)' .* inverse_signal_strength(k,:);
    vec_model_y = unit_vec(:,2)' .* inverse_signal_strength(k,:);
    vector_plot = quiver( ones(3,1).*Dx, ones(3,1).*Dy, vec_model_x', vec_model_y', 'r' );
    
    fin_vec_x = (sum(vec_model_x) + Dx) ;
    fin_vec_y = (sum(vec_model_y) + Dy) ;
    
    vec_point = plot( fin_vec_x, fin_vec_y, '*', 'Linewidth', 12, 'MarkerEdgeColor', 'blue');
    
    %% Least squares model  
    deriv_x_data = subs(deriv_x,[w,f,m],[Rx_coord(:,1)',Rx_coord(:,2)',d_inv_sqr_model(k,:)]);
    deriv_y_data = subs(deriv_y,[w,f,m],[Rx_coord(:,1)',Rx_coord(:,2)',d_inv_sqr_model(k,:)]);

    solution_ls = vpasolve([deriv_x_data == 0, deriv_y_data == 0]);
    solution_ls_x = vpa(solution_ls.x); solution_ls_y = vpa(solution_ls.y);
    sol_ls_x_real = solution_ls_x( imag(solution_ls_x) == 0);
    sol_ls_y_real = solution_ls_y( imag(solution_ls_y) == 0);
    
    ls_marker = plot(sol_ls_x_real,sol_ls_y_real,'o','MarkerSize',10,'Linewidth',6,'Color',[0.2 .9 0.1]);
    
    %% Plot stuff
    uistack([ls_marker, marker_plot,vec_point], 'top');
    legend([Tx_plot, Rx_plot, marker_plot, vec_point, ls_marker],'Transmitter positions','Receivers',...
        'Middle point of Rx','Estimated Tx position vector model','Least Squares marker');
    pause(.1);
    
    delete(ls_marker);
    delete(circle_handle);
    delete(vec_point);
    delete(vector_plot);
    delete(count);
end


   
    


 
