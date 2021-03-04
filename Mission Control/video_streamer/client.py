from flask import Response, Flask, render_template, request
import argparse
from videostream import VideoStream
import imutils
import cv2
import time

# global variables to keep track of encoding quality and the targeted time between frames
encodingquality = 50 
targetfps = 30	 # 30 FPS as a default
targetwait = 1/targetfps

# initialize a flask object
app = Flask(__name__)

# find the number of streams actively providing frames
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
		global encodingquality, targetwait, targetfps

		# add button handler for the quality selector
		if 'qualitysubmit' in request.form:
			encodingquality = int(request.form['qualityslider'])
		# add button handler for the FPS buttons
		for i in [30,20,15,12,10,8,5,4,3,2,1]:
			if 'fps_{}'.format(i) in request.form:
				targetwait = 1000/i
				targetfps = i

		# find active streams for limiting new streams
		activestreams = findactivestreams(vslist)
		camerasconnected = vslist.__len__()
		handlecamops(activestreams, camerasconnected)
		
		# find number of active video streams for rendering
		activestreams = findactivestreams(vslist)
		recordingstreams = findrecording(vslist)
		print(recordingstreams)
		# return the rendered template
		return render_template("index.html", 
				camerasconnected=camerasconnected,
				activestreams=activestreams,
				encodingquality=encodingquality,
				responsewait=targetwait,
				fps=targetfps,
				recordingstreams=recordingstreams)

# handlers for the currently connected cameras
def handlecamops(activestreams, camerasconnected):
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

# find number of video streams actively recording
def findrecording(vslist):
	recording = []
	for vs in vslist:
		recording.append(vs.recording)
	return recording

def generate():
		# grab global references to the list of video streams
		# and create list for frames to be stored in
		global vslist
		framelist = []

		# declare variables for calculating time differences
		lastwait = 0
		lasttime = time.time()

		# loop over frames from the output stream
		while True:
				# find difference in times between iterations
				currenttime=time.time() # in seconds
				difference = (currenttime-lasttime)*1000 # in milliseconds
				lasttime = currenttime

				# calculate the projected time this function will take to execute
				calculatedwait = VideoStream.calcwait(difference, targetwait, lastwait) # in milliseconds
				lastwait = calculatedwait

				# debugging stuff, dont delete yet lol
				# print('difference is ' + str(difference) + 'ms, trying to wait ' + str(targetwait) + 'ms, waiting ' + str(calculatedwait) + 'ms')

				time.sleep(calculatedwait/1000) # sleep takes an argument in seconds

				# read a frame from each vs in vslist and add it
				# to framelist if it is readable
				for vs in vslist:
						if vs.isreadable():
							framelist.append(vs.read())

				# continue if there are no frames to read
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
				encoding_parameters = [int(cv2.IMWRITE_JPEG_QUALITY), encodingquality]
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
		ap.add_argument("-f", "--fps", type=int, default=30,
				help="fps of stream")

		# parse arguments
		args = vars(ap.parse_args())
		fps = args["fps"]

		# start a thread that will grab frames from camera
		vs = VideoStream(args["source1"], fps, 'vs1').start()

		# create a list of video streams to reference in generate()
		vslist = [vs]

		# if up to 5 other video sources have been defined, start that many streams 
		for i in range(2,6):
			if args["source{}".format(i)] != -1:
				newvs = VideoStream(args["source{}".format(i)], fps, 'vs{}'.format(i))
				vslist.append(newvs)

		# start the flask app
		app.run(host=args["ip"], port=args["port"], debug=True,
				threaded=True, use_reloader=False)

# release the video stream pointer
for vs in vslist:
		vs.release()

	
