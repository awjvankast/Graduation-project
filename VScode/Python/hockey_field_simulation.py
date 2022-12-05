import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib 
from matplotlib.animation import FuncAnimation
import matplotlib.animation as animation
import PIL as pl
from shapely.geometry import Polygon
from shapely.geometry import Point
from shapely.ops import cascaded_union
from itertools import combinations
from scipy import interpolate

#TODO
# Simulate with discrete steps in receiver resolution
save = 1

heatmap_on = 0
pixel_res = 50
cone_plot = 0

animation_on = 0

# Uncertainty of the direction of nodes
res_angle = 5
res_angle_arr = np.arange(0, 181-res_angle, res_angle)

res_angle_rad = res_angle/360*2*np.pi

corner_point_coordinates = np.array([[480,780],[22,774],[28,391],[37,11],[495,16]])
sweep_first_point_coordinates = np.array([[-1200,600],[-300, -900],[500, -7300],[900,-50],[800, 1000]])

Tx_coordinates = np.array([340,410])

hockey_field_width = 55
hockey_field_height = 45.8*2
normalization_factor = hockey_field_width/458
size_hockey_field = hockey_field_width*hockey_field_height

def pixels_to_metres(pixels):
    return pixels*normalization_factor
def metres_to_pixels(metres):
    return metres*(1/normalization_factor)

def pixels_to_metres_sqrt(pixels):
    return pixels*normalization_factor**2

fig, ax = plt.subplots()
img = plt.imread("hockey_field.png")

xlim_img = img.shape[1]
ylim_img = img.shape[0]
if cone_plot:
    plt.scatter(corner_point_coordinates[2,0],corner_point_coordinates[2,1],marker = "2",clip_on = True, s= 300, label = 'Rx position',zorder = 2)
else:
    plt.scatter(corner_point_coordinates[:,0],corner_point_coordinates[:,1],marker = "2",clip_on = True, s= 300, label = 'Rx position',zorder = 2)

plt.xlim( 0,xlim_img)
plt.ylim( 0,ylim_img)
ax.set_ylim(ax.get_ylim()[::-1])

ax.imshow(img)
fig.set_size_inches(11, 7)

hockey_field_width = 55

char_arr = ["B","C","E","F","G"]
pol_tri_plot = {}
triangle = {}
tri_area_calc = {}
certain_area_plot = []
Tx_plot = []
select_triangles = {}
frame_number = ax.text(50,100, "", fontsize=15)

def update_triangles():
    if len(ax.lines) != 0:
        for k in range(0,len(char_arr)):
            ax.patches[0].remove()
        ax.lines[0].remove()
        ax.collections[1].remove()

    cur_node =corner_point_coordinates[1,:]
    first_point = np.array([-200,-500]) + cur_node
    # Raster of detection triangles for each node
    for j in char_arr:
        cur_node = corner_point_coordinates[char_arr.index(j)]
        first_point = sweep_first_point_coordinates[char_arr.index(j),:] + cur_node
        for k in res_angle_arr:
            
            first_point_vec = first_point - cur_node
            #sec_point = np.array([np.tan(res_angle_rad)*first_point_vec[0], -pow(first_point_vec[0],2)*np.tan(res_angle_rad)/first_point_vec[1] ])
            sec_point = np.array([(first_point_vec[1]*np.tan(res_angle_rad/2)), -(first_point_vec[0]*np.tan(res_angle_rad/2))])
            #pol_tri = plt.Polygon([cur_node,cur_node+first_point_vec,sec_point+first_point_vec+cur_node],color='m', fill = True)
            pol_tri = Polygon([cur_node,cur_node+first_point_vec+sec_point, cur_node+first_point_vec-sec_point])
            if j == 'E' and cone_plot:
                if k == 0:
                    fan_plot= plt.Polygon([cur_node,cur_node+first_point_vec+sec_point, cur_node+first_point_vec-sec_point],color='m', fill = True, alpha =0.3,label = 'Angle bin')
                else:
                    fan_plot= plt.Polygon([cur_node,cur_node+first_point_vec+sec_point, cur_node+first_point_vec-sec_point],color='m', fill = True, alpha =0.3)
                ax.add_patch(fan_plot)
            if pol_tri.contains(Point(Tx_coordinates[0],Tx_coordinates[1])):
                tri_area_calc[j] = pol_tri
                if j == 'B':
                    pol_tri_plot = plt.Polygon([cur_node,cur_node+first_point_vec+sec_point, cur_node+first_point_vec-sec_point],color='b', fill = True, alpha =0.3, label = 'Selected angle bin')
                else:
                    pol_tri_plot = plt.Polygon([cur_node,cur_node+first_point_vec+sec_point, cur_node+first_point_vec-sec_point],color='b', fill = True, alpha =0.3)
                if not cone_plot:
                    ax.add_patch(pol_tri_plot)
            rotate_origin = np.array([np.cos(res_angle_rad)*first_point_vec[0]-np.sin(res_angle_rad)*first_point_vec[1], np.sin(res_angle_rad)*first_point_vec[0] + np.cos(res_angle_rad)*first_point_vec[1] ])
            first_point = cur_node+rotate_origin

    # for j in char_arr:
    #     current_node = corner_point_coordinates[char_arr.index(j)]
    #     scale_factor = 100
    #     Node_to_Tx_vec = (Tx_coordinates-current_node) * scale_factor

    #     point_triangle = np.array([(Node_to_Tx_vec[1]*np.tan(res_angle_rad/2)), -(Node_to_Tx_vec[0]*np.tan(res_angle_rad/2))])
    #     triangle[j] = plt.Polygon( [current_node, point_triangle+Node_to_Tx_vec+current_node, -point_triangle+Node_to_Tx_vec+current_node], color='b', fill = True, alpha =0.3)
        
    #     tri_area_calc[j] = Polygon([current_node,point_triangle+Node_to_Tx_vec+current_node,-point_triangle+Node_to_Tx_vec+current_node])

    #     #ax.add_patch(triangle[j])

    intersect = tri_area_calc["B"]
    def intersection(shape1, shape2):
        return shape1.intersection(shape2)

    for j in tri_area_calc:
        intersect = intersection(intersect, tri_area_calc[j])

    if not cone_plot:
        certain_area_plot = ax.plot(*intersect.exterior.xy,label = 'Area of 100% certainty', color = 'r')
    certain_area = intersect.area

    #print("Area where Tx is with 100% certainty: " + str(pixels_to_metres_sqrt(certain_area))+ "m^2, hockey field is " + str(size_hockey_field)+"m^2")
    #print("Which is " + str(pixels_to_metres_sqrt(certain_area)/size_hockey_field*100)+ "% of the entire field")
    if not cone_plot:
        Tx_plot = ax.scatter(Tx_coordinates[0],Tx_coordinates[1] ,marker="+", s = 100, color = 'darkred', label = 'Tx position',zorder = 2)
    if heatmap_on:
        return pixels_to_metres_sqrt(certain_area)
    else:
        return 

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
    
    #Tx, c_area, Tri = update_triangles()
    update_triangles()
    return ax,

dist_node_hm = 100
heatmap_xmin = np.min(corner_point_coordinates[:,0])+dist_node_hm
heatmap_ymin = np.min(corner_point_coordinates[:,1])+dist_node_hm
heatmap_xmax = np.max(corner_point_coordinates[:,0])-dist_node_hm
heatmap_ymax = np.max(corner_point_coordinates[:,1])-dist_node_hm
heatmap_data = np.empty((img.shape[1],img.shape[0]))
heatmap_data[:] = np.nan

if animation_on:
    ani = FuncAnimation(fig, update, frames=600, interval = 10,init_func=init, blit=True, repeat = False)
    #update_triangles()
    #matplotlib.rcParams['animation.ffmpeg_path'] = "C:\\Users\\s153480\\Desktop\\ffmpeg-2022-11-03-git-5ccd4d3060-full_build\\bin\\ffmpeg.exe"
    #writer = animation.FFMpegWriter(fps=24, metadata=dict(artist='Me'))
    #ani.save('Hockey_field_simulation.mp4', writer = writer)
elif heatmap_on:
    Tx_coordinates = np.array([heatmap_xmin,heatmap_ymin])
    for k in range(heatmap_xmin,heatmap_xmax):
        for j in range(heatmap_ymin,heatmap_ymax):
            if (j-heatmap_ymin) % pixel_res ==0 and (k-heatmap_xmin) % pixel_res == 0:
                heatmap_data[k][j] = update_triangles()
            Tx_coordinates = Tx_coordinates + np.array([0,1])
        Tx_coordinates = Tx_coordinates - np.array([0,heatmap_ymax-heatmap_ymin]) + np.array([1,0])
    
    # Removing triangle lines
    for k in range(0,len(char_arr)):
        ax.patches[0].remove()
    ax.lines[0].remove()
    ax.collections[1].remove()

    plt.imshow(np.transpose(heatmap_data), cmap='jet', interpolation='nearest', label = "Minimal search area [m$^2$]")
    clrbar = plt.colorbar()
    clrbar.ax.set_ylabel('Size of search area [m$^2]$')
    for item in ([clrbar.ax.yaxis.label] + clrbar.ax.get_yticklabels()):
        item.set_fontsize(16)
else:
    update_triangles()
          



xax_name_list = np.array([0,10,20,30,40,50,60])
xax_val_list = metres_to_pixels(xax_name_list)

ax.xaxis.set_major_locator(matplotlib.ticker.FixedLocator((xax_val_list)))
ax.xaxis.set_major_formatter(matplotlib.ticker.FixedFormatter((xax_name_list)))

ax.set_ylim(ax.get_ylim()[::-1])

yax_name_list = np.array([0,10,20,30,40,50,60,70,80,90,100,110])
yax_val_list = metres_to_pixels(yax_name_list)

ax.yaxis.set_major_locator(matplotlib.ticker.FixedLocator((yax_val_list)))
ax.yaxis.set_major_formatter(matplotlib.ticker.FixedFormatter((yax_name_list)))


ax.set_xlabel('X coordinates [m]')
ax.set_ylabel('Y coordinates [m]')


for item in ([ax.title, ax.xaxis.label, ax.yaxis.label] + 
             ax.get_xticklabels() + ax.get_yticklabels()):
    item.set_fontsize(16)

#ax.legend(fontsize = 16)

if cone_plot:
    ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1),fontsize = 13,framealpha=1)
    plt.show()
    fig.savefig("Cone_bin_example.png",bbox_inches='tight', format = 'png')

if save and not heatmap_on:
    ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.12),fontsize = 13,framealpha=1)
    plt.show()
    fig.savefig("Cones_example_hockey_field_v3.png",bbox_inches='tight', format = 'png')
    fig.savefig("Cones_example_hockey_field_v3.eps",bbox_inches='tight', format = 'eps')
elif save and heatmap_on:
    ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.0),fontsize = 13,framealpha=1)
    plt.show()
    fig.savefig("Heatmap_hockey_field_v3.png",bbox_inches='tight', format = 'png')
    fig.savefig("Heatmap_hockey_field_v3.eps",bbox_inches='tight', format = 'eps')


print('end')