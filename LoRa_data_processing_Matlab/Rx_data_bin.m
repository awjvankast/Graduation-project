function [Blue_RSSI_mean] = Rx_data_bin(Rx)
% Function which takes as input a matrix with first column of ID's and the
% next columns of packets numbers, IMU data and RSSI data
% It outputs a matrix with the mean of half the numbers of samples per ID
% This mean is taken over the middle 50% of the samples, the first and
% last quarter of the samples are discarded

[no_ID_not_ordered, ID_not_ordered] = groupcounts(Rx(:,1));
Blue_ID_received = unique(Rx(:,1),'stable');
no_ID_ordered = zeros(1,length(no_ID_not_ordered));

% Neccesary to order it again because the function groupcounts() does not
% preserve the order while unique() does
for k = 1:length(ID_not_ordered)
    for j = 1:length(ID_not_ordered)
        if Blue_ID_received(k) == ID_not_ordered(j)
            no_ID_ordered(k) = no_ID_not_ordered(j);
        end
    end
end

% Initializing size of array based on largest number of samples
largest_no_ID = max(no_ID_ordered);
Blue_RSSI = zeros(length(no_ID_ordered), largest_no_ID);

% Formatting the matrix
for k = 1:length(Blue_ID_received)
    index_ID = Blue_ID_received(k)==Rx;
    RSSI = Rx(index_ID(:,1),12);
    Blue_RSSI(k,1:length(RSSI)+1) = [Blue_ID_received(k) RSSI'];
end

% Taking half of the total measurements of each ID, discarding the first
% and last quarter of measurements
Blue_RSSI_filtered = zeros(length(no_ID_ordered), largest_no_ID);
Blue_RSSI_mean = zeros(length(no_ID_ordered),2);
Blue_RSSI_filtered(:,1) = Blue_RSSI(:,1);
Blue_RSSI_mean(:,1) = Blue_RSSI(:,1);
for k = 1:length(no_ID_ordered)
    tot_samples = sum(Blue_RSSI(k,:)~=0)-1;
    samples_to_take = tot_samples/2;
    samples_to_discard = [round(tot_samples/4) floor(tot_samples/4)];
    samples_to_take = floor(samples_to_take);
    
    Blue_RSSI_filtered(k,2:samples_to_take+1) = ...
        Blue_RSSI(k,2+samples_to_discard(1):samples_to_take+samples_to_discard(1)+1);
    sum_index = Blue_RSSI_filtered(k,:)~=0;
    sum_index(1) = 0;
    Blue_RSSI_mean(k,2) = sum( Blue_RSSI_filtered(k,sum_index) ) / sum(sum_index);
end
end

