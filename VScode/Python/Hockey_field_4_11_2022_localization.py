import pandas as pd
import numpy as np
from PIL import Image, ImageDraw
import matplotlib.pyplot as plt


data = pd.read_csv('NoordZuidLopen_processed.csv', sep=',')

""" with Image.open("hockey_field.png") as im:
    draw = ImageDraw.Draw(im)
    draw.line((0, 0) + im.size, fill=128)
    draw.line((0, im.size[1], im.size[0], 0), fill=128)
    draw.ellipse((10,10,100,100), outline = (10,0,0))
    im.show()
    im.save("image_300.jpeg", dpi=(300, 300)) """


img = plt.imread("hockey_field.png")
fig, ax = plt.subplots()
ax.imshow(img)
plt.show()

# Make a math model here which relates RSSI values to the distance in metres

# Make a function which draws circles on map based on x,y of node and output of math distance model

# Implement the LS algorithm from Matlab here to get to a location

# Draw the location on the map and the aproximate actual location of Vincent 

print('end')