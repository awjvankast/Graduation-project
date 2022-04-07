close all; clear all; clc;

load("C:\Users\Bram\Documents\GitHub\Graduation-project\data\LoRa_trilateration_test_1\Rx1.mat");
Rx1 = filtered_data;

load("C:\Users\Bram\Documents\GitHub\Graduation-project\data\LoRa_trilateration_test_1\Rx2.mat");
Rx2 = filtered_data;

load("C:\Users\Bram\Documents\GitHub\Graduation-project\data\LoRa_trilateration_test_1\Rx3.mat");
Rx3 = filtered_data;

session_1_Rx1 = Rx1(22:361,:);
session_1_Rx2 = Rx2(22:373,:);
session_1_Rx3 = Rx3(22:361,:);

% data_selection = [ 5:16, 18:35, 38:59, 65:81, 84:109, 111:126, 130:150];

%% Free space path loss model

c = 3e8;
f = 433e6;
Pt = 1e-1;

PtdBm = 20;
PrdBm = session_1_Rx1(:,3);
Pr = 10.^(PrdBm/10-3);


d = c*sqrt(Pt)/(4*pi*f*sqrt(Pr));
%d = c/(4*pi*f)*sqrt(PtdBm/PrdBm);
