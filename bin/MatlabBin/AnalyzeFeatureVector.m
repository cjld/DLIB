function [ ] = AnalyzeFeatureVector( input_args )
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here
    gs = size(input_args);
    gs = gs(2)/3;
    sfv = ones(3,3,gs);
    maxdet = 1;
    mindet = 1;
    for i=1:gs
        sfv(:,:,i) = input_args(:,(i*3-2):(i*3));
        tp = sfv(:,:,i);
        dete = det(tp);
        maxdet = max(dete, maxdet);
        mindet = min(dete, mindet);
        if (dete>20 || dete<0)
            disp(i);
            disp(dete);
            disp(tp);
        end
    end
    disp('Max determinant :');
    disp(maxdet);
    disp('Min determinant :');
    disp(mindet);
end

