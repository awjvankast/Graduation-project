import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib 
from matplotlib.animation import FuncAnimation
import matplotlib.animation as animation
import PIL as pl
from shapely.geometry import Polygon
from shapely.ops import cascaded_union
from itertools import combinations

#TODO
# Calculate correct area under the final polygon, can be done by:


# Uncertainty of the direction of nodes
res_angle = 8
res_angle_rad = res_angle/360*2*np.pi

corner_point_coordinates = np.array([[480,780],[22,774],[28,391],[37,11],[495,16]])
Tx_coordinates = np.array([50,50])

hockey_field_width = 55
hockey_field_height = 45.8*2
normalization_factor = hockey_field_width/458
size_hockey_field = hockey_field_width*hockey_field_height
def pixels_to_metres(pixels):
    return pixels*normalization_factor

def pixels_to_metres_sqrt(pixels):
    return pixels*normalization_factor**2


fig, ax = plt.subplots()
img = plt.imread("hockey_field.png")
xlim_img = img.shape[1]
ylim_img = img.shape[0]
plt.scatter(corner_point_coordinates[:,0],corner_point_coordinates[:,1],marker = "2",clip_on = True, s= 75)


plt.xlim( 0,xlim_img)
plt.ylim( 0,ylim_img)
ax.set_ylim(ax.get_ylim()[::-1])

ax.imshow(img)
fig.set_size_inches(15, 8)

hockey_field_width = 55

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
triangle = {}
tri_area_calc = {}
certain_area_plot = []
Tx_plot = []
frame_number = ax.text(50,100, "", fontsize=15)

def update_triangles():
    if len(triangle) != 0:
        for k in range(0,len(char_arr)):
            ax.patches[0].remove()
        ax.lines[0].remove()
        ax.collections[1].remove()

    for j in char_arr:
        current_node = corner_point_coordinates[char_arr.index(j)]
        scale_factor = 100
        Node_to_Tx_vec = (Tx_coordinates-current_node) * scale_factor

        point_triangle = np.array([(Node_to_Tx_vec[1]*np.tan(res_angle_rad/2)), -(Node_to_Tx_vec[0]*np.tan(res_angle_rad/2))])
        triangle[j] = plt.Polygon( [current_node, point_triangle+Node_to_Tx_vec+current_node, -point_triangle+Node_to_Tx_vec+current_node], color='b', fill = True, alpha =0.3)
        
        tri_area_calc[j] = Polygon([current_node,point_triangle+Node_to_Tx_vec+current_node,-point_triangle+Node_to_Tx_vec+current_node])

        ax.add_patch(triangle[j])

    intersect = tri_area_calc["B"]
    def intersection(shape1, shape2):
        return shape1.intersection(shape2)

    for j in tri_area_calc:
        intersect = intersection(intersect, tri_area_calc[j])

    certain_area_plot = ax.plot(*intersect.exterior.xy,label = 'Area of 100% certainty', color = 'r')
    certain_area = intersect.area

    #print("Area where Tx is with 100% certainty: " + str(pixels_to_metres_sqrt(certain_area))+ "m^2, hockey field is " + str(size_hockey_field)+"m^2")
    #print("Which is " + str(pixels_to_metres_sqrt(certain_area)/size_hockey_field*100)+ "% of the entire field")

    Tx_plot = ax.scatter(Tx_coordinates[0],Tx_coordinates[1] ,marker="+", s = 30, color = 'darkred')

    return Tx_plot, certain_area_plot, triangle



def init():
  #  ax.imshow(img)
 #   frame_number.set_text("0")
   # ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.1),
     #     ncol=3, fancybox=True)
  #  ax.legend()
    return ax, # this part seems to be the problem to return the white plots

def update(frame):
    # Draw the circles which relate the RSSI to distance 
    # for j in char_arr:
    #     circle[j].radius = distance_normalized[j][frame]
    # frame_number.set_text(frame)
    global Tx_coordinates 
    x = 250 + 90 * np.sin(np.radians(frame))
    y = 400 + 175 * np.cos(np.radians(frame))
    #Tx_coordinates = Tx_coordinates + np.array([x,y])
    Tx_coordinates = np.array([x,y])
    
    Tx, c_area, Tri = update_triangles()
    return ax,

ani = FuncAnimation(fig, update, frames=600, interval = 10,
                    init_func=init, blit=True, repeat = False)

matplotlib.rcParams['animation.ffmpeg_path'] = "C:\\Users\\s153480\\Desktop\\ffmpeg-2022-11-03-git-5ccd4d3060-full_build\\bin\\ffmpeg.exe"
writer = animation.FFMpegWriter(fps=24, metadata=dict(artist='Me'))
ani.save('Hockey_field_simulation.mp4', writer = writer)


plt.show()

print('end')