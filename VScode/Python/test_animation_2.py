import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import matplotlib 

img = plt.imread("hockey_field.png")

fig, ax = plt.subplots()
img = plt.imread("hockey_field.png")
xlim_img = img.shape[1]
ylim_img = img.shape[0]

patch = {}


def init():
    global patch
    patch = plt.Circle((5, -5), 0.75, fc='b')
    patch.center = (20, 20)
    ax.add_patch(patch)
    return patch,

def animate(i):
    x = 10 + 3 * np.sin(np.radians(i))
    y = 10 + 3 * np.cos(np.radians(i))
    patch.center = (x, y)
    return patch,

anim = animation.FuncAnimation(fig, animate, 
                               init_func=init, 
                               frames=360, 
                               interval=1,
                               blit=True)

plt.imshow(img,zorder=0,  extent=[0.1, 20.0, 0.1, 20.0])

# WORKS!!!
matplotlib.rcParams['animation.ffmpeg_path'] = "C:\\Users\\s153480\\Desktop\\ffmpeg-2022-11-03-git-5ccd4d3060-full_build\\bin\\ffmpeg.exe"
writer = animation.FFMpegWriter(fps=24, metadata=dict(artist='Me'))
anim.save('the_movie.mp4', writer = writer)
plt.show()