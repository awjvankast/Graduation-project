close all; clear; clc;


field_image = imread('..\data\Hockey_field_4_11_2022\field_geolocation.png');
imshow(field_image);

size_image = size(field_image);

data_NS = ("..\data\Hockey_field_4_11_2022\NoordZuidLopen.txt");