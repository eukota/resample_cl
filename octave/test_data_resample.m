% generate sample data for resample kernel
% kernel calculates:
% n0*x^0 + n1*x^1 + n2*x^2 + n3*x^3 + n4*x^4 + n5*x^5 + n6*x^6 + n7*x^7
% 

n=ones(1,8)*2;
x=1:4;
csvwrite('..\sessions\data\resample_in.csv',x')
y=n(1)*x.^0 + n(2)*x.^1 + n(3)*x.^2 + n(4)*x.^3 + n(5)*x.^4 + n(6)*x.^5 + n(7)*x.^6 + n(8)*x.^7;
csvwrite('..\sessions\data\resample_out.csv',y')