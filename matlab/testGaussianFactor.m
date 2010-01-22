%-----------------------------------------------------------------------
% eliminate

% the combined linear factor
Ax2 = [
-5., 0.
+0.,-5.
10., 0.
+0.,10.
];

Al1 = [
5., 0.
0., 5.
0., 0.
0., 0.
];

Ax1 = [
0.00,  0. % f4
0.00,  0. % f4
-10.,  0. % f2
0.00,-10. % f2
];

% the RHS
b2=[-1;1.5;2;-1];
model = SharedDiagonal([1;1]);
combined = GaussianFactor('x2', Ax2,  'l1', Al1, 'x1', Ax1, b2, model);

% eliminate the combined factor
% NOT WORKING
% this is not working because the eliminate has to be added back to gtsam.h
[actualCG,actualLF] = combined.eliminate('x2');

% create expected Conditional Gaussian
R11 = [
11.1803,  0.00
0.00, 11.1803
];
S12 = [
-2.23607, 0.00
+0.00,-2.23607
];
S13 = [
-8.94427, 0.00
+0.00,-8.94427
];
d=[2.23607;-1.56525];
expectedCG = GaussianConditional('x2',d,R11,'l1',S12,'x1',S13,model);

% the expected linear factor
Bl1 = [
4.47214, 0.00
0.00, 4.47214
];

Bx1 = [
% x1
-4.47214,  0.00
+0.00, -4.47214
];

% the RHS
b1= [0.0;0.894427];

expectedLF = GaussianFactor('l1', Bl1, 'x1', Bx1, b1, 1);

% check if the result matches
% NOT WORKING 
% because can not be computed with GaussianFactor.eliminate
if(~actualCG.equals(expectedCG)), warning('GTSAM:unit','~actualCG.equals(expectedCG)'); end
if(~actualLF.equals(expectedLF,1e-5)), warning('GTSAM:unit','~actualLF.equals(expectedLF,1e-5)');end
