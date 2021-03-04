from flask import Response, Flask, render_template, request
import argparse
from videostream import VideoStream
import imutils
import cv2

# initialize a flask object
app = Flask(__name__)

def findactivestreams(vslist):
	activestreams = 0
	for vs in vslist:
		if vs.stopped == False:
			activestreams += 1
	return activestreams

# define the root url to be index.html
# in other words, when the user accesses http://[ip]:[port]/
# they will see index.html
@app.route("/", methods=['GET','POST'])
def index():
		# find active streams for limiting new streams
		activestreams = findactivestreams(vslist)

		# add button handlers depending on number of cameras
		camerasconnected = vslist.__len__()
		for i in range(0, camerasconnected):
			if 'stopvs{}'.format(i+1) in request.form:
				vslist[i].stop()
			if 'stoprecordvs{}'.format(i+1) in request.form:
				vslist[i].recordstop()
			if 'startrecordvs{}'.format(i+1) in request.form:
				vslist[i].recordstart()				
			if 'relaunchvs{}'.format(i+1) in request.form:
				vslist[i].relaunch()
			if 'startvs{}'.format(i+1) in request.form and activestreams <= 3:
				vslist[i].start()
		
		# find number of active video streams
		activestreams = findactivestreams(vslist)

		# return the rendered template
		return render_template("index.html", 
				camerasconnected=camerasconnected,
				activestreams=activestreams)

def generate():
		# grab global references to the list of video streams
		# and create list for frames to be stored in
		global vslist
		framelist = []
		# loop over frames from the output stream
		while True:
				# read a frame from each vs in vslist and add it
				# to framelist if it is readable
				for vs in vslist:
						if vs.isreadable():
							framelist.append(vs.read())

				if framelist.__len__() == 0:
					continue
				
				# resize each frame to 600
				for frame in framelist:
						frame = imutils.resize(frame, width=600)
					
				# merge each of the three frames into one image
				mergedim = cv2.hconcat(framelist)

				# add the timestamp to the merged image
				mergedim = VideoStream.timestampframe(mergedim)

				# clear the framelist for the next iteration
				framelist.clear()

				# encode the frame in JPEG format
				encoding_parameters = [int(cv2.IMWRITE_JPEG_QUALITY), 50]
				(flag, encodedimage) = cv2.imencode(".jpg", mergedim, encoding_parameters)

				# ensure the frame was successfully encoded
				if not flag:
						continue

				# yield the output frame in the byte format
				yield(b'--frame\r\n' b'Content-Type: image/jpeg\r\n\r\n' + 
						bytearray(encodedimage) + b'\r\n')

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

		# allow up to 6 video streams to be created
		ap.add_argument("-s1", "--source1", type=int, default=0,
				help="index of first camera to use")
		ap.add_argument("-s2", "--source2", type=int, default=-1,
				help="index of second camera to use")
		ap.add_argument("-s3", "--source3", type=int, default=-1,
				help="index of third camera to use")
		ap.add_argument("-s4", "--source4", type=int, default=-1,
				help="index of fourth camera to use")
		ap.add_argument("-s5", "--source5", type=int, default=-1,
				help="index of fifth camera to use")
		ap.add_argument("-s6", "--source6", type=int, default=-1,
				help="index of sixth camera to use")

		# choose fps
		ap.add_argument("-f", "--fps", type=int, default=15,
				help="fps of stream")

		# parse arguments
		args = vars(ap.parse_args())
		fps = args["fps"]
		# start a thread that will grab frames from camera
		vs = VideoStream(args["source1"], fps, 'vs1').start()

		# create a list of video streams to reference in generate()
		vslist = [vs]

		# if a second source has been defined, start a new video stream 
		if args["source2"] != -1:
				vs2 = VideoStream(args["source2"], fps, 'vs2')
				vslist.append(vs2)

		# if a third source has been defined, start a new video stream 
		if args["source3"] != -1:
				vs3 = VideoStream(args["source3"], fps, 'vs3')
				vslist.append(vs3)

		# and so on...
		if args["source4"] != -1:
				vs4 = VideoStream(args["source4"], fps, 'vs4')
				vslist.append(vs4)
		if args["source5"] != -1:
				vs5 = VideoStream(args["source5"], fps, 'vs5')
				vslist.append(vs5)
		if args["source6"] != -1:
				vs6 = VideoStream(args["source6"], fps, 'vs6')
				vslist.append(vs6)

		# start the flask app
		app.run(host=args["ip"], port=args["port"], debug=True,
				threaded=True, use_reloader=False)

# release the video stream pointer
for vs in vslist:
		vs.release()

	
