import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib 
import time
from matplotlib.animation import FuncAnimation
import matplotlib.animation as animation
import PIL as pl

ani_on = 0
save_ani_data = 0
pixel_step_size = 1
plot_tot_Tx = 1
plot_single = 0


hockey_field_width = 55
normalization_factor = 458/hockey_field_width

marksize = 3
scat_width = 3

def pixels_to_metres(pixels):
    return pixels/normalization_factor
def metres_to_pixels(metres):
    return metres*normalization_factor

# def pixels_to_metres_sqrt(pixels):
#     return pixels*normalization_factor**2

data = pd.read_csv('NoordOostZuidWestLopen_processed.csv', sep=',')

# Make a function which draws the coordinates of the nodes on the map 
corner_point_coordinates =  np.array([[480,780],[22,774],[28,391],[37,11],[495,16]])
fig, ax = plt.subplots()
img = plt.imread("hockey_field.png")
xlim_img = img.shape[1]
ylim_img = img.shape[0]
plt.scatter(corner_point_coordinates[:,0],corner_point_coordinates[:,1],marker = "2",clip_on = True, s= 20*2**marksize, label = 'Rx position',zorder = 2,linewidths=scat_width)
plt.xlim( 0,xlim_img)
plt.ylim( 0,ylim_img)
ax.set_ylim(ax.get_ylim()[::-1])
ax.imshow(img)
fig.set_size_inches(11, 7)

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
    if j == 'B':
        circle[j] = plt.Circle((corner_point_coordinates[char_arr.index(j),0], corner_point_coordinates[char_arr.index(j),1]), distance_normalized[j][0], color='b', fill = False,clip_on = True,label = 'RSSI distance')
    else:
        circle[j] = plt.Circle((corner_point_coordinates[char_arr.index(j),0], corner_point_coordinates[char_arr.index(j),1]), distance_normalized[j][0], color='b', fill = False,clip_on = True)

    ax.add_patch(circle[j])

total_walked_dist_pix = 2*np.linalg.norm(corner_point_coordinates[4]-corner_point_coordinates[1])
norm_direc = (corner_point_coordinates[4]-corner_point_coordinates[1])/np.linalg.norm(corner_point_coordinates[4]-corner_point_coordinates[1])
tot_frames = 608
dist_pix_per_frame = total_walked_dist_pix/tot_frames
real_Tx_start_coord = (corner_point_coordinates[4]+corner_point_coordinates[1])/2

tot_Tx_error = []
tot_Tx_coord = []
#real_Tx_start_coord = np.array([240,395])
#plt.scatter(real_Tx_start_coord[0],real_Tx_start_coord[1])
#plt.show()
def plot_real_Tx(frame):
    if frame <tot_frames/4:
        real_Tx = real_Tx_start_coord + frame*dist_pix_per_frame*norm_direc
    elif frame <tot_frames/4*3:
        real_Tx = real_Tx_start_coord + tot_frames/4*dist_pix_per_frame*norm_direc- (frame-tot_frames/4)*dist_pix_per_frame*norm_direc
    else:
        real_Tx = real_Tx_start_coord + tot_frames/4*dist_pix_per_frame*norm_direc- (tot_frames/2)*dist_pix_per_frame*norm_direc +(frame-tot_frames/4*3)*dist_pix_per_frame*norm_direc
    real_Tx_plot = ax.scatter(real_Tx[0],real_Tx[1] ,marker="x", s =  20*2**marksize, color = 'black', label = 'Tx real position',zorder = 2,linewidths=scat_width)
    return real_Tx
def init():
    ax.imshow(img)
    if ani_on:
        frame_number.set_text("0")
    Tx_plot = ax.scatter(-100,-100 ,marker="+", s = 100, color = 'darkred',zorder = 2,linewidths=scat_width)
    return ax,

def update(frame):
    # Draw the circles which relate the RSSI to distance 
    start_frame = 0
    frame = frame + start_frame
    if frame != start_frame:
        ax.collections[6].remove()
        ax.collections[5].remove()
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
    Tx_plot = ax.scatter(least_coord[0],least_coord[1] ,marker="+", s =  20*2**marksize, color = 'darkred', label = 'Tx estimated position',zorder = 2,linewidths=scat_width)
    real_Tx = plot_real_Tx(frame)
    global tot_Tx_error
    global tot_Tx_coord
    tot_Tx_error = np.append(tot_Tx_error, np.linalg.norm(real_Tx - least_coord))
    if tot_Tx_coord == []:
        tot_Tx_coord = least_coord
    else:
        tot_Tx_coord = np.vstack([tot_Tx_coord, least_coord])
    return ax,

# Necessary to let the image remain at the same boundary
if ani_on:
    ani = FuncAnimation(fig, update, frames=607, interval = 1,
                    init_func=init, blit=True, repeat = False)
elif plot_single:
    cur_frame = 10
    init()
    for j in char_arr:
        circle[j].radius = distance_normalized[j][cur_frame]

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
    Tx_plot = ax.scatter(least_coord[0],least_coord[1] ,marker="+", s = 20*2**marksize, color = 'darkred', label = 'Tx estimated position',zorder = 2,linewidths=scat_width)
    plot_real_Tx(cur_frame)
elif plot_real_Tx:
    tcoord = np.load('TotTxcoord.npy')
    terror = np.load('TotTxerror.npy')
    merror = pixels_to_metres(terror)
    mean_err = np.mean(merror)    
    print(mean_err)   
    plt.scatter(tcoord[:,0],tcoord[:,1],marker = ".", s = 25,color = 'darkred',label = 'Tx estimated position')
    plt.plot( [corner_point_coordinates[1,0],corner_point_coordinates[4,0]] ,[corner_point_coordinates[1,1],corner_point_coordinates[4,1]] ,color = 'black',label = 'Path traveled')
    for k in range(0,len(char_arr)):
        ax.patches[0].remove()
            
    


# matplotlib.rcParams['animation.ffmpeg_path'] = "C:\\Users\\s153480\\Desktop\\ffmpeg-2022-11-03-git-5ccd4d3060-full_build\\bin\\ffmpeg.exe"
# writer = animation.FFMpegWriter(fps=24, metadata=dict(artist='Me'))
# ani.save('anim.mp4', writer=writer) 
# writervideo = animation.FFMpegFileWriter(fps=60)



# plt.show()

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
print('Total error')
print(pixels_to_metres(np.sum(tot_Tx_error)))
print('Mean error')
print(pixels_to_metres(np.mean(tot_Tx_error)))

for item in ([ax.title, ax.xaxis.label, ax.yaxis.label] + 
             ax.get_xticklabels() + ax.get_yticklabels()):
    item.set_fontsize(16)
ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15),fontsize = 13,framealpha=1)



if ani_on and save_ani_data:
    np.save('totTxerror',tot_Tx_error)
    np.save('TotTxcoord',tot_Tx_coord)

if plot_single:
    plt.show()
    fig.savefig("RSSI_example.png",bbox_inches='tight', format = 'png')
    fig.savefig("RSSI_example.eps",bbox_inches='tight', format = 'eps')
elif plot_tot_Tx:
    plt.show()
    fig.savefig("RSSI_walk.png",bbox_inches='tight', format = 'png')
    fig.savefig("RSSI_walk.eps",bbox_inches='tight', format = 'eps')
else:
    plt.show()


print('end')