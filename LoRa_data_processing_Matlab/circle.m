function [plot_handle] =circle(x,y,r)
%x and y are the coordinates of the center of the circle
%r is the radius of the circle
%0.01 is the angle step, bigger values will draw the circle faster but
%you might notice imperfections (not very smooth)
ang=0:0.01:2*pi; 
plot_handle = [];
for k = 1:length(x) 
    xp=r(k)*cos(ang);
    yp=r(k)*sin(ang);
    plot_handle = [plot_handle plot(x(k)+xp,y(k)+yp, 'k')];
end