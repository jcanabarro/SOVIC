#!/usr/bin/python

import datetime
import mraa
import sys
import time
import cv2
import cv
import argparse
import numpy as np

ap = argparse.ArgumentParser()
ap.add_argument("-i", "--image", default='teste.png')
args = vars(ap.parse_args())


if __name__ == "__main__":
    print "Opening camera"

camera= cv2.VideoCapture(0)

#camera.set(0, 0) #Current position of the video file in miliseconds
#camera.set(1, 0) 
#camera.set(2, 0) #Relative position of the video file

camera.set(3, 1280)  #width
camera.set(4, 720) #height

#camera.set(5, 60) #Frame rate

#camera.set(10, 167) #Brightness

#camera.set(11, 100) #Contrast

#camera.set(12, 200) #Saturation

#camera.set(13, 300) #HUE of the image
#camera.set(14, 20) #Gain of the image
#camera.set(15, 0.5) #exposure
#camera.set(16, 1) #Flag indicating whether images should be converted to RGB
#camera.set(17, 10000) #White balance
#camera.set(18, 0) #Rectification flag for stereo cameras

time.sleep(1)
retval, im = camera.read()

img = im[20:650, 0: 1900]
file = "/www/pages/algoritmo_png_gray/"+args["image"]
cv2.imwrite(file, img)
del(camera)

