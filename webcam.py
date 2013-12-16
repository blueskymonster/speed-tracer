import cv2
import sys
import numpy as np

capture = cv2.VideoCapture(-1)

if capture is None:
  print "No camera detected. Quitting..."
  sys.exit(1)

cv2.namedWindow("stream")

lastFrame = None
alphaColor = None

def onMouse(event, x, y, skip1, skip2):
  if event != cv2.EVENT_LBUTTONDOWN:
    return
  if lastFrame is not None:
    print "x: %d, y: %d, val: %s" % (x, y, lastFrame[y, x])
    global alphaColor
    alphaColor = lastFrame[y, x]
cv2.setMouseCallback("stream", onMouse)

def transformFrame(preFrame):
  print "alphaColor: %s" % alphaColor
  print "last frame is none: %s" % (lastFrame is None)
  postFrame = np.fliplr(preFrame)
  if alphaColor is not None and lastFrame is not None:
    return np.where(postFrame == alphaColor, lastFrame, postFrame)
  else:
    print postFrame
  return postFrame

print "Streaming video..."
while True:
  status, frame = capture.read()
  if status:
    transformedFrame = transformFrame(frame)
    cv2.imshow("stream", transformedFrame)
    lastFrame = transformedFrame
  if cv2.waitKey(10) >= 0:
    break
