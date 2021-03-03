from threading import Thread
import cv2

class VideoStream:
    # initialize self with defined source and fps
    def __init__(self, src=0, fps=15,  name="VideoStream"):
        self.stream = cv2.VideoCapture(src)
        self.src = src
        self.fps = fps
        # set the capture codec to MJPG
        self.stream.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('M','J','P','G'))

        # set fps to argument
        self.stream.set(cv2.CAP_PROP_FPS, fps)

        # set grabbed to false
        self.grabbed = False

        # set the name of the object
        self.name = name

        # define a variable to indicate whether the thread should be stopped
        self.stopped = False
    
    # create a new thread to continuously call update() and read frames
    def start(self):
        print('starting ' + self.name)
        self.stopped = False
        t = Thread(target=self.update, name=self.name, args=())
        t.daemon = True
        t.start()
        # print('thread started')
        return self
    
    # read a frame from the camera unless stopped is true
    def update(self):
        while True:
            if self.stopped:
                self.grabbed = False
                return
            (self.grabbed, self.frame) = self.stream.read()
    
    # return the most recent frame grabbed from the camera
    def read(self):
        return self.frame

    # set stopped to True which will halt the thread
    def stop(self):
        print('stopping ' + self.name)
        self.stopped = True

    # sets the property of the VideoCapture object
    def set(self, property, value):
        self.stream.set(property, value)
    
    # returns the property of the VideoCapture object
    def get(self, property):
        return self.stream.get(property)

    # returns whether a frame can be read
    def isreadable(self):
        return self.grabbed

    # returns index of camera
    def getsrc(self):
        return self.src
    
    # releases video capture object, doesn't work lol
    def release(self):
        self.stop()
        self.stream.release()

    # also doesn't work, just don't touch this for now
    def relaunch(self):
        print('relaunch has been called')
        self.release()
        self.__init__(self.src, self.fps)

