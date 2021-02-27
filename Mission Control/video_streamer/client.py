from imutils.video import WebcamVideoStream
from flask import Response
from flask import Flask
from flask import render_template
import threading
import argparse
import datetime
import imutils
import time
import cv2

# initialize the output frame and a lock used to ensure thread-safe
# exchanges of the output frames (useful for multiple browsers/tabs
# are viewing tthe stream)
outputFrame = None
lock = threading.Lock()

# initialize a flask object
app = Flask(__name__)


# initialize the video stream and allow the camera sensor to
# warmup
def startSteam(src):
	return WebcamVideoStream(src).start()

@app.route("/")
def index():
	# return the rendered template
	return render_template("index.html")

def get_frame(frameCount):
    global vs, outputFrame, lock

    while vs is not None:
	    frame = vs.read()
	    frame = imutils.resize(frame, width=800)

		# grab the current timestamp and draw it on the frame
		# may not be useful and can be disabled
	    timestamp = datetime.datetime.now()
	    cv2.putText(frame, timestamp.strftime(
			"%A %d %B %Y %I:%M:%S%p"), (10, frame.shape[0] - 10),
			cv2.FONT_HERSHEY_SIMPLEX, 0.35, (0, 0, 255), 1)

		# acquire the lock, set the output frame, and release the
		# lock
	    with lock:
		    outputFrame = frame.copy()

def generate():
	# grab global references to the output frame and lock variables
	global outputFrame, lock

	# loop over frames from the output stream
	while True:
		# wait until the lock is acquired
		with lock:
			# check if the output frame is available, otherwise skip
			# the iteration of the loop
			if outputFrame is None:
				continue

			# encode the frame in JPEG format
			encoding_parameters = [int(cv2.IMWRITE_JPEG_QUALITY), 50]
			(flag, encodedImage) = cv2.imencode(".jpg", outputFrame, encoding_parameters)

			# ensure the frame was successfully encoded
			if not flag:
				continue

		# send = False
		# yield the output frame in the byte format
		if (True):
			yield(b'--frame\r\n' b'Content-Type: image/jpeg\r\n\r\n' + 
				bytearray(encodedImage) + b'\r\n')
			# print('image sent')
		# send = not send

@app.route("/video_feed")
def video_feed():
	# return the response generated along with the specific media
	# type (mime type)
	return Response(generate(),
		mimetype = "multipart/x-mixed-replace; boundary=frame")

# check to see if this is the main thread of execution
if __name__ == '__main__':
	global vs
	# construct the argument parser and parse command line arguments
	ap = argparse.ArgumentParser()
	ap.add_argument("-i", "--ip", type=str, required=True,
		help="ip address of the device")
	ap.add_argument("-o", "--port", type=int, required=True,
		help="ephemeral port number of the server (1024 to 65535)")
	ap.add_argument("-f", "--frame-count", type=int, default=32,
		help="# of frames used to construct the background model")
	ap.add_argument("-s", "--source", type=int, default=2,
		help="index of camera to use")

	args = vars(ap.parse_args())
	# start a thread that will grab frames from camera
	vs = startSteam(args["source"])
	t = threading.Thread(target=get_frame, args=(
		args["frame_count"],))
	t.daemon = True
	t.start()

	# start the flask app
	app.run(host=args["ip"], port=args["port"], debug=True,
		threaded=True, use_reloader=False)

# release the video stream pointer
vs.stop()