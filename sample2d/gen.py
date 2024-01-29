#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import cv2
import os

def main():
    os.makedirs("data", exist_ok=True)
    orig = cv2.imread("original.jpg")
    orig = cv2.resize(orig, (750, 500))

    for i in range(197):
        blur = i % 99 
        blur = blur + 1
        if i < 99:
            blur = 99 - blur
        blur = blur + 1

        #print("%3d: %d" % (i, blur))
        img = cv2.blur(orig, (blur, blur))
        cv2.imwrite("data/blur_" + str(i).zfill(3) + ".png", img)

if __name__ == '__main__':
    main()
