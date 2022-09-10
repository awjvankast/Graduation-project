function [point] = least_squares_circles(Rx_coord,Rx_fitted)
% Least Squares: 
% Take as input the distances of the inverse square model in 3*1 matrix
% and the Rx coordinates in a 3*3 matrix
% Output a single point which minimizes the distances

% Initializing all the symbolic variables
syms x y ; syms w [1 3]; syms f [1 3]; syms m [1 3]
syms E(x,y);
syms deriv(x,y); syms deriv_x(x,y); syms deriv_y(x,y);

% Defining the error function
E(x,y) = sum( ( (x-w).^2 + (y-f).^2 - m.^2 ).^2 );

% Taking the derivative wrt the two variables
deriv_x(x,y) = diff(E,x);
deriv_y(x,y) = diff(E,y);

% Substituting the given data into the derivative functions
deriv_x_data = subs(deriv_x,[w,f,m],[Rx_coord(:,1)',Rx_coord(:,2)',Rx_fitted]);
deriv_y_data = subs(deriv_y,[w,f,m],[Rx_coord(:,1)',Rx_coord(:,2)',Rx_fitted]);

% Solving the equations for minimal error (derivative equal to zero)
% Vpa solve for discrete solution
solution_ls = vpasolve([deriv_x_data == 0, deriv_y_data == 0]);
solution_ls_x = vpa(solution_ls.x); solution_ls_y = vpa(solution_ls.y);

% Only getting the real parts
sol_ls_x_real = solution_ls_x( imag(solution_ls_x) == 0);
sol_ls_y_real = solution_ls_y( imag(solution_ls_y) == 0);

% Returning the final position in single vector
point = [sol_ls_x_real, sol_ls_y_real];

end

