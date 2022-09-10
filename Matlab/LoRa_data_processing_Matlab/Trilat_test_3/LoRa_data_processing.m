%close all; clear all; clc;

% Replace this with the data imported
data = Rxgreen;

% Check the data for the correct format, if a single data entry is wrong
% throw away the entire row
nan_check = isnan(data);
for k = 1:size(nan_check,1)
    if any( nan_check(k,[1,3:13]) == 1) % if any of the values in these columns is NaN
        
        % checks if the row is a session identifier row, if not, delete the
        % row
        if  ~nan_check(k,2) && ~any( nan_check( k, [1,3:13] ) == 0)    
        else
        data(k,:) = [];
        end

    end
end
    
session_index_bin = ~isnan(data(:,2));
session_start_index = find(session_index_bin==1);

no_indices = sum(session_index_bin);
session_data = NaN(no_indices,size(data,1),size(data,2)-1);

for k=1:no_indices
    if k ~=no_indices % if it's not the last session
        % now skip the first row of the session because only the Rx 
        % session identifier is in there
        % substract one fom the next start index to get the last row of the
        % session
        rel_rows = session_start_index(k)+1:session_start_index(k+1)-1;
    else
        % if it's the last session of the batch just go to the last element
        % of the dataset
        rel_rows = session_start_index(k)+1:length(data);
    end
    % Now put the data in bins per session TODO
    session_data(k,1,1) = data(session_start_index(k),2);
    session_data(k,rel_rows,:) = data(rel_rows,[1, 3:13]);
end

relevant_no_session = 8; % fill the how many-th receiver session is relevant

data_to_use = squeeze(session_data(relevant_no_session,:,:));
relevant_data_row_bin = ~isnan(data_to_use( :,1 ));
filtered_data = data_to_use(relevant_data_row_bin,:)

% stores data in relevant folder
save("..\..\data\LoRa_trilateration_test_3\Rx_ygreen_distance.mat",'filtered_data');
