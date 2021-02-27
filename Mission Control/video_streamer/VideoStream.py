from threading import Thread
import cv2

class VideoStream:
    def __init__(self, src, name="VideoStream"):
        self.stream = cv2.VideoCapture(src)
        (self.grabbed, self.frame) = self.read()

        self.stopped = False
    
    def start(self):
        t = Thread(target=self.update, args=())
        t.daemon = True
        t.start()
        return self
    
    def update(self):
        while True:
            if self.stopped:
                return
            (self.grabbed, self.frame) = self.stream.read()
    
    def read(self):
        return self.frame

    def stop(self):
        self.stopped = True

    def set(self, property, value):
        self.stream.set(property, value)
    
    def get(self, property):
        return self.stream.get(property)
    