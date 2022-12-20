close all; clear all; clc;

x = 0:0.01:2*pi;
y1 = sin(x);
y2 = sin(x-pi/4);

plot(x,[y1;y2],'LineWidth' , 2);
legend('Signal antenna 1', 'Signal antenna 2','FontSize',14);
xlim([-0.1,2*pi+.1]);
ylim([-1.1,1.1]);
grid on;
%set(gca,'xtick',[])
%set(gca,'ytick',[0])
%set(gca,'visible','off');