function [ x ] = LinearSolveSumFixed( A, b, x )
    sz = size(A,1);
    A = [0,ones(1,sz);ones(sz,1),A];
    b = [x;b];
    x = A\b;
    x = x(2:sz+1);
end