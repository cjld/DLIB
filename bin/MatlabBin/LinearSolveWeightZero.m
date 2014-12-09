function [ x ] = LinearSolveWeightZero( A, b )
    sz = size(A,1);
    A = [0,ones(1,sz);ones(sz,1),A];
    b = [0;b];
    x = A\b;
    x = x(2:sz+1);
end