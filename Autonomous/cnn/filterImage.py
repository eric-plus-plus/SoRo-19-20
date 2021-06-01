import tensorflow as tf
from tensorflow import keras
import numpy as np
import matplotlib.pyplot as plt
import cv2
import argparse

def execute(args):
    #loads in the model
    model = keras.models.load_model(args.model)

    #takes a picture
    cam = cv2.VideoCapture(args.camera)
    ret, im = cam.read()
    cam.release()

    im = cv2.resize(im, (1920, 1080))
    #fixes the shape of the image
    sh = im.shape
    newSh = (1, sh[0], sh[1], sh[2])
    fixedIm = [np.reshape(im, newshape = newSh)]
    fixedIm = np.concatenate(fixedIm, axis=0)

    #makes a prediction of where the ar tag is
    pred = model.predict(fixedIm)
    labels=np.argmax(pred, axis=3)
    predIm = labels[0,:,:]

    #Converts the image to just black and white
    predIm = (np.where(predIm > .5, 255, 0))
    predIm = np.uint8(predIm)
    predIm = cv2.cvtColor(predIm, cv2.COLOR_GRAY2BGR)

    #combines the taken image with the predicted image and writes to disk
    finalIm = np.concatenate((im, predIm), axis=0)
    cv2.imwrite(args.image, finalIm)


def createParser():
    parser = argparse.ArgumentParser(description='Semantic Labeling', fromfile_prefix_chars='@')
    parser.add_argument('-camera', type=str, default='/dev/video2', help='Path to the camaera')
    parser.add_argument('-model', type = str, default = 'testModel', help = 'Path to the saved tf model')
    parser.add_argument('-image', type=str, default='test.jpg', help='Name of the image that will be written out')

    return parser

if __name__ == "__main__":
    parser = createParser()
    args = parser.parse_args()
    execute(args)
