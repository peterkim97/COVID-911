import datetime
import cv2

capture = cv2.VideoCapture("/dev/video0")
capture.set(cv2.CAP_PROP_FRAME_WIDTH, 1080)
capture.set(cv2.CAP_PROP_FRAME_HEIGHT, 720)

fourcc = cv2.VideoWriter_fourcc(*'XVID')
record = False
capture_on=False
count=0
while True:

    ret, frame = capture.read()
    cv2.imshow("VideoFrame", frame)
   
    now = datetime.datetime.now().strftime("%d_%H-%M-%S")
    key = cv2.waitKey(1) & 0xFF
    if capture_on==True:
        count+=1
    if key == ord("q"):
        break
    elif key == ord("c"):
        capture_on=True
    elif key == ord("x"):
        print("capture pause");
        capture_on=False
    
    elif count == 50: #captures every 5 second
        count=0
        print("capture")
        cv2.imwrite(str(now) + ".png", frame)
    elif key == ord("r"):
        print("record start")
        record = True
        video = cv2.VideoWriter(str(now) + ".mp4", fourcc, 20.0, (frame.shape[1], frame.shape[0]))
    elif key == ord("s"):
        print("record pause")
        record = False
        video.release()
        
    if record == True:
        print("녹화 중..")
        video.write(frame)

capture.release()
cv2.destroyAllWindows()
