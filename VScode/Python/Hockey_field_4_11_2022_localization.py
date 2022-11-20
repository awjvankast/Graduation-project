import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib 
import time
from matplotlib.animation import FuncAnimation
import matplotlib.animation as animation
import PIL as pl

#TODO 
# saving gives whitescreen

data = pd.read_csv('NoordOostZuidWestLopen_processed.csv', sep=',')

# Make a function which draws the coordinates of the nodes on the map 
corner_point_coordinates = np.array([[480,780],[22,774],[28,391],[37,11],[495,16]])
fig, ax = plt.subplots()
img = plt.imread("hockey_field.png")
xlim_img = img.shape[1]
ylim_img = img.shape[0]
plt.scatter(corner_point_coordinates[:,0],corner_point_coordinates[:,1],marker = "2",clip_on = True, s= 75)
plt.xlim( 0,xlim_img)
plt.ylim( 0,ylim_img)
ax.set_ylim(ax.get_ylim()[::-1])
ax.imshow(img)


# Make a math model here which relates RSSI values to the distance in metres
def dist_model(x):
    return 0.9 *pow(-x, 1.905)-100

# OG: 0.606 *pow(-x, 1.756)-278.8
def round_zero(x):
    if x<0:
        return 0
    else:
        return x

distance = data.copy().fillna(0).apply(dist_model).apply(lambda x: x/100)
distance[distance<0] = 0

hockey_field_width = 55
normalization_factor = 458/hockey_field_width
distance_normalized = distance.apply(lambda x: x*normalization_factor)

# Draw the walked path of this dataset on the map
# plt.show()
# char_arr = ["B","C","E","F","G"]
# for k in range(0,5):
#     # Draw the circles which relate the RSSI to distance 
#     for j in char_arr:
#         circle = plt.Circle((corner_point_coordinates[char_arr.index(j),0], corner_point_coordinates[char_arr.index(j),1]), distance_normalized[j][k], color='r', fill = False,clip_on = True)
#         ax.add_patch(circle)
#     plt.close()
#     plt.show()
#     time.sleep(1)


    # Implement the LS algorithm from Matlab here to get to a location

char_arr = ["B","C","E","F","G"]
circle = {}

frame_number = ax.text(50,100, "", fontsize=15)

for j in char_arr:
    circle[j] = plt.Circle((corner_point_coordinates[char_arr.index(j),0], corner_point_coordinates[char_arr.index(j),1]), distance_normalized[j][0], color='r', fill = False,clip_on = True)
    ax.add_patch(circle[j])

def init():
    ax.imshow(img)
    frame_number.set_text("0")
    return ax,

def update(frame):
    # Draw the circles which relate the RSSI to distance 
    for j in char_arr:
        circle[j].radius = distance_normalized[j][frame]
    frame_number.set_text(frame)
   
    return ax,

# Necessary to let the image remain at the same boundary

ani = FuncAnimation(fig, update, frames=500, interval = 50,
                    init_func=init, blit=True, repeat = False)



# matplotlib.rcParams['animation.ffmpeg_path'] = "C:\\Users\\s153480\\Desktop\\ffmpeg-2022-11-03-git-5ccd4d3060-full_build\\bin\\ffmpeg.exe"
# writer = animation.FFMpegWriter(fps=24, metadata=dict(artist='Me'))
# ani.save('anim.mp4', writer=writer) 
# writervideo = animation.FFMpegFileWriter(fps=60)



# plt.show()

plt.show()


print('end')