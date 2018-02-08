## Development Environment
* Device --- MacBook Pro
* OS --- MacOS highSierra 10.13.3
* IDE --- XCode (9.2 (9C40b))
* OpenGL --- 4.1

## Part 1 (Controls)
Control | Key
------------- | -------------
Change Position  | `Left mouse click` and move
Change Orientation  | `Right mouse click` and move about the `x-axis`
Magnification | `Mouse scroll wheel` to zoom in and out
Image 1 | `#1`
Image 2 | `#2`
Image 3 | `#3`
Image 4 | `#4`
Image 5 | `#5`
Image 6 (Image I chose) | `#6`

### Part 1 (Limitations)
* When rotating the image, the image does not always move in the direction of your mouse drag
* When rotating, the image does not rotate about the center of the window, rather it rotates about the center of the image

## Part 2 (Colour Effects)

Effect | Key
------------- | -------------
L = 0.333 R + 0.333 G + 0.333 B  | `Z`
L = 0.299 R + 0.587 G + 0.114 B | `X`
L = 0.213 R + 0.715 G + 0.072 B | `C`
Chosen colour effect (drastically darken image) | `V`
Original Image | `B`

### Part 1 (Limitations)
* n/a

### Which greyscale effect looks the most correct ?
* 'L = 0.213 R + 0.715 G + 0.072 B' looks the most correct in my opinion, because the mandrill's nose is a bright red and the greyscale version, it is displayed as a much darker tone of grey
* Different formulas exist for various artisitic effects and to put emphasis on certain parts/colours in the image

## Part 3 (Edge Effects)

Effect | Key
------------- | -------------
Horizontal Sobel  | `S`
Vertical Sobel | `A`
Unsharp mask operator | `D`

### Part 3 (Limitations)
* n/a

## Part 4 (Gaussian Blur)

Effect | Key
------------- | -------------
3 x 3 Gaussian  | `L`
5 x 5 Gaussian  | `K`
7 x 7 Gaussian  | `J`

### Part 4 (Limitations)
* n/a


## REFERENCES
For mouse event handling, code was inspired by this open github repo:
https://github.com/SonarSystems/OpenGL-Tutorials/blob/master/GLFW%20Mouse%20Input/main.cpp

For Sobel/unsharp filter, code was inspired by these links:
https://gist.github.com/Hebali/6ebfc66106459aacee6a9fac029d0115
http://www.ozone3d.net/tutorials/image_filtering.php

For Gaussian Blur, code was inspired from this link:
https://www.opengl.org/discussion_boards/showthread.php/191072-Real-time-2D-camera-lens-blur-effect-achieveable

My image choice:
https://www.banffjaspercollection.com/Brewster/media/Images/Stories/2017/11/CL-Banff-Avenue.jpg?ext=.jpg&requiredtype=image&defaultfilepath=~/App_Themes/Default/Images/No-Card-Image.png&maxsidesize=220

