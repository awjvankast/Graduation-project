function [plot_handle] =circle(x,y,r, color)
%x and y are the coordinates of the center of the circle
%r is the radius of the circle
%0.01 is the angle step, bigger values will draw the circle faster but
%you might notice imperfections (not very smooth)

arguments
    x
    y
    r
    color (1,1) string = 'k'
end

ang=0:0.01:2*pi; 
plot_handle = [];
for k = 1:length(x) 
    xp=r(k)*cos(ang);
    yp=r(k)*sin(ang);
    plot_handle = [plot_handle plot(x(k)+xp,y(k)+yp, color)];
end