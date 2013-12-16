import cv2
import sys
import numpy as np

capture = cv2.VideoCapture(-1)

if capture is None:
  print "No camera detected. Quitting..."
  sys.exit(1)

cv2.namedWindow("stream")

lastFrame = None

def onMouse(event, x, y, skip1, skip2):
  if event != cv2.EVENT_LBUTTONDOWN:
    return
  if lastFrame is not None:
	  print "x: %d, y: %d, val: %s" % (x, y, lastFrame[y, x])
cv2.setMouseCallback("stream", onMouse)

print "Streaming video..."
while True:
  status, frame = capture.read()
  if status:
    lastFrame = frame
    cv2.imshow("stream", np.fliplr(frame))
  if cv2.waitKey(10) >= 0:
    break
