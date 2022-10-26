close all; clear all; clc;

x = 0:.1:4*pi;

A = .5*sin(x);
B = sin(x + .5*pi);

C = A + B;

plot(x, [A; B; C]);
legend(["sin(x)","sin(x+.5*pi)","A+B"]);