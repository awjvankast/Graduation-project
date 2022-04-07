close all; clear all; clc;

% fill in the right path
data = LoRa_data_import_func("..\data\LoRa_trilateration_test_1\LoRa_trilateration_test_1\Rx1.txt");

session_index_bin = ~isnan(data(:,2));
session_start_index = find(session_index_bin==1);

no_indices = sum(session_index_bin);
session_data = NaN(no_indices,size(data,1),3);

for k=1:no_indices
    if k ~=no_indices
        rel_rows = session_start_index(k)+1:session_start_index(k+1)-1;
    else
        rel_rows = session_start_index(k)+1:length(data);
    end
    session_data(k,1,1) = data(session_start_index(k),2);
    session_data(k,rel_rows,:) = data(rel_rows,[1, 3, 4]);
end

relevant_no_session = 2; % fill the how many-th receiver session is relevant

data_to_use = squeeze(session_data(relevant_no_session,:,:));
relevant_data_row_bin = ~isnan(data_to_use( :,1 ));
filtered_data = data_to_use(relevant_data_row_bin,:)

% stores data in relevant folder
save("..\data\LoRa_trilateration_test_1\Rx1.mat",'filtered_data');
