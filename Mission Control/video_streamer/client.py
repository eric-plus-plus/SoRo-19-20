from flask import Response
from flask import Flask
from flask import render_template
import argparse
import datetime
from videostream import VideoStream
import imutils
import cv2


# initialize the output frame and a lock used to ensure thread-safe
# exchanges of the output frames (useful for multiple browsers/tabs
# are viewing tthe stream)
# benton note: maybe this might be important to put back in at some point
# but it seems to work fine without it so i'll leave it as a comment for now
# lock = threading.Lock()

# initialize a flask object
app = Flask(__name__)

# initialize the video stream with the desired fps
def startStream(src, fps):
		print('stream started')
		return VideoStream(src, fps).start()

# define the root url to be index.html
# in other words, when the user accesses http://[ip]:[port]/
# they will see index.html
@app.route("/")
def index():
		# return the rendered template
		return render_template("index.html")

# grab the current timestamp and draw it on the frame
def timestampframe(frame):
		timestamp = datetime.datetime.now()
		cv2.putText(frame, timestamp.strftime(
			  "%A %d %B %Y %I:%M:%S%p"), (10, frame.shape[0] - 10),
			  cv2.FONT_HERSHEY_SIMPLEX, 0.35, (0, 0, 255), 1)

		return frame.copy()
						
def generate():
		# grab global references to the list of video streams
		# and create list for frames to be stored in
		global vslist
		framelist = []

		# loop over frames from the output stream
		while True:
				# read a frame from each vs in vslist and add it
				# to framelist
				for vs in vslist:
						framelist.append(vs.read())

				# resize each frame to 600
				for frame in framelist:
						frame = imutils.resize(frame, width=600)
					
				# wait until the lock is acquired
				# with lock:

				# merge each of the three frames into one image
				mergedim = cv2.hconcat(framelist)

				# add the timestamp to the merged image
				timestampframe(mergedim)

				# clear the framelist for the next iteration
				framelist.clear()

				# encode the frame in JPEG format
				encoding_parameters = [int(cv2.IMWRITE_JPEG_QUALITY), 50]
				(flag, encodedImage) = cv2.imencode(".jpg", mergedim, encoding_parameters)

				# ensure the frame was successfully encoded
				if not flag:
						continue

				# yield the output frame in the byte format
				yield(b'--frame\r\n' b'Content-Type: image/jpeg\r\n\r\n' + 
						bytearray(encodedImage) + b'\r\n')

# return the generated image to the 'video_feed' element of index.html
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
				help="port number of the server (1024 to 65535)")
		ap.add_argument("-s1", "--source1", type=int, default=0,
				help="index of first camera to use")
		ap.add_argument("-s2", "--source2", type=int, default=-1,
				help="index of second camera to use")
		ap.add_argument("-s3", "--source3", type=int, default=-1,
				help="index of third camera to use")
		ap.add_argument("-f", "--fps", type=int, default=15,
				help="fps of stream")

		# parse arguments
		args = vars(ap.parse_args())

		# start a thread that will grab frames from camera
		vs = startStream(args["source1"], args["fps"])

		# create a list of video streams to reference in generate()
		vslist = [vs]

		# if a second source has been defined, start a new video stream 
		if args["source2"] != -1:
				vs2 = startStream(args["source2"], args["fps"])
				vslist.append(vs2)

		# if a third source has been defined, start a new video stream 
		if args["source3"] != -1:
				vs3 = startStream(args["source3"], args["fps"])
				vslist.append(vs3)

		# start the flask app
		app.run(host=args["ip"], port=args["port"], debug=True,
				threaded=True, use_reloader=False)

# release the video stream pointer
for vs in vslist:
		vs.stop()

	
