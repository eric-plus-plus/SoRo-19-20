from flask import Response
from flask import Flask
from flask import render_template
import threading
import argparse
import datetime
from videostream import VideoStream
from threading import Thread
import imutils
import time
import cv2


# initialize the output frame and a lock used to ensure thread-safe
# exchanges of the output frames (useful for multiple browsers/tabs
# are viewing tthe stream)
# lock = threading.Lock()

# initialize a flask object
app = Flask(__name__)


# initialize the video stream and allow the camera sensor to
# warmup
def startStream(src):
	print('stream started')
	return VideoStream(src).start()

@app.route("/")
def index():
	# return the rendered template
	return render_template("index.html")

def get_frame(vs):
	
	while vs is not None:
		frame = vs.read()
		# frame = imutils.resize(frame, width=600)

		# print("frame has been read")

		# grab the current timestamp and draw it on the frame
		# may not be useful and can be disabled
		timestamp = datetime.datetime.now()
		cv2.putText(frame, timestamp.strftime(
			"%A %d %B %Y %I:%M:%S%p"), (10, frame.shape[0] - 10),
			cv2.FONT_HERSHEY_SIMPLEX, 0.35, (0, 0, 255), 1)

		# acquire the lock, set the output frame, and release the
		# lock
		# with lock:
		# print('frame returned')
		return frame.copy()
			
def generate():
	# grab global references to the output frame and lock variables
	# global vs, vs2
	global vslist
	framelist = []

	# loop over frames from the output stream
	while True:

		for vs in vslist:
			framelist.append(vs.read())

		# wait until the lock is acquired
		# with lock:
			# check if the output frame is available, otherwise skip
			# the iteration of the loop
		
		mergedim = cv2.hconcat(framelist)
		framelist.clear()
			# encode the frame in JPEG format
		encoding_parameters = [int(cv2.IMWRITE_JPEG_QUALITY), 50]
		(flag, encodedImage) = cv2.imencode(".jpg", mergedim, encoding_parameters)

			# ensure the frame was successfully encoded
		if not flag:
			continue

		# send = Falsehreading.Thread(target=get_frame, args=(vs))
		# t.daemon = True
		# t.star
		# yield the output frame in the byte format
		yield(b'--frame\r\n' b'Content-Type: image/jpeg\r\n\r\n' + 
			bytearray(encodedImage) + b'\r\n')
			# print('image sent')

@app.route("/video_feed")
def video_feed():
	# return the response generated along with the specific media
	# type (mime type)
	return Response(generate(),
		mimetype = "multipart/x-mixed-replace; boundary=frame")

# check to see if this is the main thread of execution
if __name__ == '__main__':

	# construct the argument parser and parse command line arguments
	ap = argparse.ArgumentParser()
	ap.add_argument("-i", "--ip", type=str, required=True,
		help="ip address of the device")
	ap.add_argument("-o", "--port", type=int, required=True,
		help="ephemeral port number of the server (1024 to 65535)")
	ap.add_argument("-s1", "--source1", type=int, default=0,
		help="index of camera to use")
	ap.add_argument("-s2", "--source2", type=int, default=-1,
		help="index of camera to use")
	ap.add_argument("-s3", "--source3", type=int, default=-1,
		help="index of camera to use")

	args = vars(ap.parse_args())
	# start a thread that will grab frames from camera
	vs = startStream(args["source1"])
	vslist = [vs]
	if args["source2"] != -1:
		vs2 = startStream(args["source2"])
		vslist.append(vs2)
	if args["source3"] != -1:
		vs3 = startStream(args["source3"])
		vslist.append(vs3)

	# vs.stream.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('M','J','P','G'))

	print(vs.get(cv2.CAP_PROP_FOURCC))
	print(vs.get(cv2.CAP_PROP_FPS))

	print(vs.stream.get(cv2.CAP_PROP_FRAME_HEIGHT))
	print(vs.stream.get(cv2.CAP_PROP_FRAME_WIDTH))

	for vs in vslist:
		pass
		# vs.set(cv2.CAP_PROP_FPS, 5)
		# t = threading.Thread(target=get_frame, args=(vs))
		# t.daemon = True
		# t.start()

	# start the flask app
	app.run(host=args["ip"], port=args["port"], debug=True,
		threaded=True, use_reloader=False)

# release the video stream pointer
for vs in vslist:
	vs.stop()

    