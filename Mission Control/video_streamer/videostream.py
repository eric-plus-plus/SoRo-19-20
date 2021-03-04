from threading import Thread
import cv2
import datetime
import time
import os

class VideoStream:
	# initialize self with defined source and fps
	def __init__(self, src=0, fps=30,  name="VideoStream"):
		# set grabbed to false
		self.grabbed = False

		# define a variable to indeicate whether recording should happen
		self.recording = False

		# set the name of the object
		self.name = name

		# define a variable to indicate whether the thread should be stopped
		self.stopped = True

		# initialize the VideoCapture object with the source
		self.stream = cv2.VideoCapture(src)
		# set the capture codec to MJPG
		self.stream.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('M','J','P','G'))

		# tries to set fps to argument
		self.stream.set(cv2.CAP_PROP_FPS, fps)

		# gets actual width and height from camera
		self.width = int(self.stream.get(cv2.CAP_PROP_FRAME_WIDTH))
		self.height = int(self.stream.get(cv2.CAP_PROP_FRAME_HEIGHT))

		# gets actual fps value from camera
		self.fps = self.stream.get(cv2.CAP_PROP_FPS)

		# creates a new VideoWriter to save the recording
		currenttime = datetime.datetime.now()
		self.filename = './recordings/{}_{}.avi'.format(currenttime.strftime("%Y%m%d_%H-%M-%S"),self.name)
		self.writer = cv2.VideoWriter(self.filename,
			cv2.VideoWriter_fourcc('M','J','P','G'),self.fps,(self.width,self.height))

		# saves the index of the webcam for relaunching
		self.src = src
		self.totaldifference = 0
		self.recordedframes = 0
		# time in milliseconds between each frame
		if self.fps != 0:
			self.waittime = 1000/self.fps


	
	# create a new thread to continuously call update() and read frames
	def start(self):
		if self.stopped is True:
			print('starting ' + self.name)
			self.stopped = False
			t = Thread(target=self.update, name=self.name, args=())
			t.daemon = True
			t.start()
			# print('thread started')
			return self
		else:
			print(self.name + " is already running")

	
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
		self.recordstop()
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
	
	# releases video capture object
	def release(self):
		self.stop()
		self.writer.release()
		self.stream.release()
		time.sleep(.1)
		if os.path.exists(self.filename) and os.stat(self.filename).st_size < 10000:
			os.remove(self.filename)
		

	# releases old VideoCapture object and creates a new one
	# use this if there's a hardware error with the camera such
	# as it being unplugged while the program is running
	def relaunch(self):
		print('relaunch has been called')
		self.release()
		self.__init__(self.src, self.fps, self.name)


	# starts the thread that handles recording
	def recordstart(self):
		if self.recording == False and self.stopped == False:
			self.recording = True
			print('recording started')
			t = Thread(target=self.record, name=(self.name + 'record'), args=())
			t.daemon = True
			t.start()
		else:
			print('recording has already started')

	# target function to grab a frame from the camera after a set amount of time
	def record(self):
		lasttime = time.time()
		lastwait = 0
		while True:
			if self.recording == False:
				return
			if self.stopped == False:
				self.writer.write(VideoStream.timestampframe(self.read()))
				currenttime = time.time()
				difference = (currenttime-lasttime)*1000
				lasttime = currenttime
				# debugging stuff
				# self.totaldifference += difference
				# self.recordedframes += 1

				timetosleep = VideoStream.calcwait(difference, self.waittime, lastwait)
				lastwait = timetosleep
				# print('difference: ' + str(difference) + ' going to wait ' + str(timetosleep) + 'ms')
				time.sleep(timetosleep/1000)

	# stops the thread that handles recording
	def recordstop(self):
		if self.recording == True:
			print('recording stopped')
			# print('recorded frames: ' + str(self.recordedframes))
			# print('average difference: ' + str(self.totaldifference/self.recordedframes))
			self.recording = False
		else:
			print('recording is already stopped')

	# grab the current timestamp and draw it on the frame
	@staticmethod
	def timestampframe(frame):
		timestamp = datetime.datetime.now()
		copiedframe = frame.copy()
		cv2.putText(copiedframe, timestamp.strftime(
			  "%A %d %B %Y %I:%M:%S%p"), (10, frame.shape[0] - 10),
			  cv2.FONT_HERSHEY_SIMPLEX, 0.35, (0, 0, 255), 1)
		return copiedframe
	
	# essentially all this does is calculate the amount of time to wait
	# before grabbing the next frame
	@staticmethod
	def calcwait(difference, targettime, lastwait):
		timetosleep = lastwait - (difference-targettime)
		if timetosleep < 0:
			timetosleep = 0
		if timetosleep > targettime:
			timetosleep = targettime
		return timetosleep