%% Import data from text file
% Script for importing data from the following text file:
%
%    filename: C:\Users\s153480\Documents\GitHub\Graduation-project\data\LoRa_trilateration_test_2\Green_Rx.TXT
%
% Auto-generated by MATLAB on 02-May-2022 11:53:00

%% Setup the Import Options and import the data
opts = delimitedTextImportOptions("NumVariables", 22);

% Specify range and delimiter
opts.DataLines = [1, Inf];
opts.Delimiter = [" ", ",", ":"];

% Specify column names and types
opts.VariableNames = ["Var1", "Var2", "Var3", "VarName4", "Var5", "VarName6", "Var7", "VarName8", "VarName9", "VarName10", "Var11", "VarName12", "VarName13", "VarName14", "Var15", "VarName16", "VarName17", "VarName18", "Var19", "Var20", "Var21", "VarName22"];
opts.SelectedVariableNames = ["VarName4", "VarName6", "VarName8", "VarName9", "VarName10", "VarName12", "VarName13", "VarName14", "VarName16", "VarName17", "VarName18", "VarName22"];
opts.VariableTypes = ["string", "string", "string", "double", "string", "double", "string", "double", "double", "double", "string", "double", "double", "double", "string", "double", "double", "double", "string", "string", "string", "double"];

% Specify file level properties
opts.ExtraColumnsRule = "ignore";
opts.EmptyLineRule = "read";

% Specify variable properties
opts = setvaropts(opts, ["Var1", "Var2", "Var3", "Var5", "Var7", "Var11", "Var15", "Var19", "Var20", "Var21"], "WhitespaceRule", "preserve");
opts = setvaropts(opts, ["Var1", "Var2", "Var3", "Var5", "Var7", "Var11", "Var15", "Var19", "Var20", "Var21"], "EmptyFieldRule", "auto");

% Import the data
Rx = readtable("C:\Users\s153480\Documents\GitHub\Graduation-project\data\LoRa_trilateration_test_2\Yellow_Rx.TXT", opts);

%% Convert to output type
Rxarr = table2array(Rx);
Rxarr([1,433,434,487],:) = [];

save("..\data\LoRa_trilateration_test_2\Yellow_Rx_t2.mat",'Rxarr');

%% Clear temporary variables
clear opts