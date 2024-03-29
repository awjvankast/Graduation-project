close all; clear all;

load('RSSIdistmodeldata.mat')

% Take the average out of every 10 bin
bins = 1:82;
cur_bin = 10;
cur_dat=[];
for k = 1:length(RSSIdata)
    if mod(k-1,10) == 0 && k ~=1
        bins((k-1)/10) = mean( RSSIdata(k-10:k-1,2) );
    end
end
bins(82) = [];
dist = 1:81;
bins = -bins;
a =   1.421e-11;
b =       6.323;
x = 60:0.001:105;
fit = a.*x.^b;

scat=scatter(-bins,dist,'o','MarkerFaceColor','b','MarkerEdgeColor','b');
al = .8;
scat.MarkerFaceAlpha = al;
scat.MarkerEdgeAlpha = al;
hold on;
grid on;
    textsize = 16;
plot(-x,fit,'LineWidth', 2);
ax = gca;
ax.FontSize = 12;
xlim([-105,-60]);
ylabel(['Distance [m]'],FontSize=textsize);
xlabel('RSSI [dBm]',FontSize = textsize);
legend('Measurement points', 'Inverse-square based fit',location = 'NE',FontSize = textsize);


%ax.XAxis.TickValues = -flip(ax.XAxis.TickValues);
%ax.XAxis.Limits = [ -105,-60];
%save('RSSIdistmodeldata.mat');
% Relate average out of 10 bin to actual distance

% Make fit 