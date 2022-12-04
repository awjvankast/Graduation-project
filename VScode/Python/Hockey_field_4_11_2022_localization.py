import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib 
import time
from matplotlib.animation import FuncAnimation
import matplotlib.animation as animation
import PIL as pl


ani_on = 0
pixel_step_size = 1

data = pd.read_csv('NoordOostZuidWestLopen_processed.csv', sep=',')

# Make a function which draws the coordinates of the nodes on the map 
corner_point_coordinates =  np.array([[480,780],[22,774],[28,391],[37,11],[495,16]])
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
    return 1.42e-11 *pow(-x, 6.323)

# OG: 0.606 *pow(-x, 1.756)-278.8
def round_zero(x):
    if x<0:
        return 0
    else:
        return x

#distance = data.copy().fillna(0).apply(dist_model).apply(lambda x: x/100)
distance = data.copy().fillna(0).apply(dist_model)
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
    Tx_plot = ax.scatter(-100,-100 ,marker="+", s = 100, color = 'darkred', label = 'Tx estimated position',zorder = 2)
    return ax,

def update(frame):
    # Draw the circles which relate the RSSI to distance 
    #frame = frame +300
    ax.collections[3].remove()
    for j in char_arr:
        circle[j].radius = distance_normalized[j][frame]
    frame_number.set_text(frame)

    least_dist = 1e4
    least_coord = []
    for k in range (0,img.shape[1],pixel_step_size):
        for l in range(0,img.shape[0],pixel_step_size):
            tot_dist = 0
            for m in char_arr:
                # Minimum circle size 
                if circle[m].radius>10:
                    tot_dist = tot_dist + abs( np.linalg.norm(np.array([k,l])-circle[m].center)-circle[m].radius )
            
            if tot_dist < least_dist:
                least_dist = tot_dist
                least_coord = np.array([k,l])
    #print(least_dist)
    #print(least_coord)
    Tx_plot = ax.scatter(least_coord[0],least_coord[1] ,marker="+", s = 100, color = 'darkred', label = 'Tx estimated position',zorder = 2)
   
    return ax,

# Necessary to let the image remain at the same boundary
if ani_on:
    ani = FuncAnimation(fig, update, frames=700, interval = 1,
                    init_func=init, blit=True, repeat = False)
else:
    cur_frame = 10
    init()
    for j in char_arr:
        circle[j].radius = distance_normalized[j][cur_frame]
    frame_number.set_text(cur_frame)

    pixel_step_size = 5
    least_dist = 1e4
    least_coord = []
    for k in range (0,img.shape[1],pixel_step_size):
        for l in range(0,img.shape[0],pixel_step_size):
            tot_dist = 0
            for m in char_arr:
                # Minimum circle size 
                if circle[m].radius>10:
                    tot_dist = tot_dist + abs( np.linalg.norm(np.array([k,l])-circle[m].center)-circle[m].radius )
            
            if tot_dist < least_dist:
                least_dist = tot_dist
                least_coord = np.array([k,l])
    print(least_dist)
    print(least_coord)
    Tx_plot = ax.scatter(least_coord[0],least_coord[1] ,marker="+", s = 100, color = 'darkred', label = 'Tx estimated position',zorder = 2)

            
            
    


# matplotlib.rcParams['animation.ffmpeg_path'] = "C:\\Users\\s153480\\Desktop\\ffmpeg-2022-11-03-git-5ccd4d3060-full_build\\bin\\ffmpeg.exe"
# writer = animation.FFMpegWriter(fps=24, metadata=dict(artist='Me'))
# ani.save('anim.mp4', writer=writer) 
# writervideo = animation.FFMpegFileWriter(fps=60)



# plt.show()

plt.show()


print('end')