# optical_flow_gpu_opencv3
Use GPU to extract videos' dense optical flow by opencv3

## Authorship
The code is mainly borrowed from [Limin Wang's repo](https://github.com/wanglimin/dense_flow).

I've changed the code to apply to opencv3.

## How to use
Option | Default | Note 
:---   | :---    | :---
f  | ex.avi  | filename of video
x  | img/x   | filename prefix of flow x component
y  | img/y   | filename prefix of flow y component
i  | img/i   | filename prefix of image
b  | 20      | specify the maximum (px) of optical flow
t  | 1       | specify the optical flow algorithm
d  | 0       | specify gpu id
s  | 1       | specify the step for frame sampling
h  | 0       | specify the height of saved flows, 0: keep original height
w  | 0       | specify the width of saved flows,  0: keep original width

First of all, you need opencv3 in your computer.
```
pkg-config --modversion opencv
3.x.x
```
Then just do this.
```
./compile.sh
mkdir img

# you can simply use it by
./get_flow_gpu -f=ex.avi

# or you can set some parameters
./get_flow_gpu -f=ex.avi -x=img/x -y=img/y -i=img/i -b=20 -t=1 -d=0 -s=1 -h=0 -w=0
```
All images will be saved in *./img/* default.
