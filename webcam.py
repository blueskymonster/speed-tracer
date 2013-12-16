import cv2
import sys

capture = cv2.VideoCapture(-1)

if capture is None:
  print "No camera detected. Quitting..."
  sys.exit(1)

cv2.namedWindow("stream")

print "Streaming video..."
while True:
  status, frame = capture.read()
  if status:
    cv2.imshow("stream", frame)
  if cv2.waitKey(10) >= 0:
    break
cv2.DestroyAllWindows()
