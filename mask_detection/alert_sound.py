import socket, threading
import sys 	
from playsound import playsound


# Recieve data from client and play the sound(alert)
def binder(client_socket, addr):
  time_checker=0
  flag = 0	

  print('Connected by', addr);	
  try:	

    while True:	

	
      data = client_socket.recv(4);	

      length = int.from_bytes(data, "little");	

      data = client_socket.recv(length);
      
      # decode received data to str type
      msg = data.decode();	
      datalist = msg.split(',')
      for idx in range(int(len(datalist)/5)):
        label=datalist[idx*5]

      time_checker+=1
      # Wait 3 seconds till the previous alert ends
      if "No_Mask" in datalist:   
        if time_checker % 3 == 0:
          playsound("Please.mp3")
          time_checker=0
        flag = 1
      elif "Useless_Mask" in datalist:
        if time_checker % 3 == 0:
          playsound("PleaseProp.mp3")
          time_checker=0
        flag = 1
      elif "Mask" in datalist:
        if flag == 1:
          playsound("thankyou.mp3")
          flag = 0
  except:	
	
    print("except : " , addr);	
  finally:	

    client_socket.close();	
 	

server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM);	

server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1);	

server_socket.bind(('', 9999));	

server_socket.listen();	
 	
try:	

  while True:	

    client_socket, addr = server_socket.accept();	

    th = threading.Thread(target=binder, args = (client_socket,addr));	
    th.start();
    
except:	
  print("server");
  
finally:
  server_socket.close();
