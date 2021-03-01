import torch
import torchvision

model = torchvision.models.alexnet(pretrained=False)
model.classifier[6] = torch.nn.Linear(model.classifier[6].in_features, 2)


model.load_state_dict(torch.load('best_model.pth'))


device = torch.device('cuda')
model = model.to(device)


import cv2
import numpy as np
from socket import *

class SocketInfo():
    HOST=""
    PORT=8888
    BUFSIZE=7
    ADDR=(HOST, PORT)
 
class socketInfo(SocketInfo):
    HOST= "127.0.0.1"

#socket init
csock= socket(AF_INET, SOCK_STREAM)
csock.connect(socketInfo.ADDR)
print("connect is success")
 
commend= csock.recv(socketInfo.BUFSIZE, MSG_WAITALL)
data= commend.decode("UTF-8")
 
print("type : {}, data len : {}, data : {}, Contents : {}".format(type(commend),len(commend), commend, data))
print()


mean = 255.0 * np.array([0.485, 0.456, 0.406])
stdev = 255.0 * np.array([0.229, 0.224, 0.225])

normalize = torchvision.transforms.Normalize(mean, stdev)

def preprocess(camera_value):
    global device, normalize
    x = camera_value
    x = cv2.cvtColor(x, cv2.COLOR_BGR2RGB)
    x = x.transpose((2, 0, 1))
    x = torch.from_numpy(x).float()
    x = normalize(x)
    x = x.to(device)
    x = x[None, ...]
    return x
import torch.nn.functional as F
import time
total_cnt=0
block_cnt=0

def update(change):
#    global blocked_slider
    global block_cnt
    global total_cnt
    x = change['new'] 
    x = preprocess(x)
    y = model(x)
    
    # we apply the `softmax` function to normalize the output vector so it sums to 1 (which makes it a probability distribution)
    y = F.softmax(y, dim=1)
    
    prob_blocked = float(y.flatten()[0])

#    blocked_slider.value = prob_blocked
    if prob_blocked < 0.5:
        Data=0
    else:
        Data=1
        
    block_cnt+=Data
    total_cnt+=1
    
    # Collect 11 datas of foot(people_)detection infos and choose
    # the most counted result
    # should be odd number since there could be a draw when using even number
    
    if total_cnt == 11:
        if block_cnt >= 6:
            Data=1	
        else:
            Data=0
        
        block_cnt=0
        total_cnt=0
        to_server= int(Data)
        right_method= to_server.to_bytes(4, byteorder='little')
        
        if Data is 1:
            print("Send Data : foot ")
        elif Data is 0:
            print("Send Data : pass ")
        sent= csock.send(right_method)
        time.sleep(0.001)


cap = cv2.VideoCapture("/dev/video0")
cap.set(cv2.CAP_PROP_FRAME_WIDTH,1080)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT,720)
while True:
    ret, img = cap.read()
    if not ret:
        break
    h,w=img.shape[:2]
    result_img = img.copy()
    cv2.imshow("Frame", result_img)
    key = cv2.waitKey(1) & 0xFF
    if key == ord("q"):
        to_server1= int(2)
        right_method1= to_server1.to_bytes(4, byteorder='little')
        #print("Send Data : {}".format(to_server))
        sent1= csock.send(right_method1)
        break
    update({'new':img}) #camera.value})  # we call the function once to intialize

cap.release()
cv2.destroyAllWindows()


