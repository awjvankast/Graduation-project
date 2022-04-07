function [W] = dBm2W(dBm)

W = 10.^(dBm./10-3);

end

