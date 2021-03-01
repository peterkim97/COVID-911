# COVID-911
  

Project Name:COVID-911   
Team:EOEO   
Members: 박태양, 김윤환, 전현정, 김승수   
Term:08/2020 - 10/2020   
Project Video Link : https://youtu.be/5f1vN644pEo   

If you have any questions, feel free to ask seungpeter@gmail.com   

Our Members   
<img width="409" alt="스크린샷 2020-11-06 오후 4 51 09" src="https://user-images.githubusercontent.com/66055313/98340209-57f3cc80-2050-11eb-85cb-211f9488efd9.png">   


<img width="527" alt="스크린샷 2020-11-06 오후 4 50 09" src="https://user-images.githubusercontent.com/66055313/98340102-2b3fb500-2050-11eb-910d-6c02d1f01a56.png">

  
As the COVID-19 pandemic is getting worse and people are being exposed to the virus, wearing mask in public is considered mandatory, as well as checking people's temperature.    
Therefore, we made a self-driving robot that patrols inside the building and detects people with high temperature, or not wearing masks and diagnose the possibility of COVID-19 in advance. If they found someone who's not wearing, they will warn them to wear mask until they wear it properly and then it will say 'thank you'.   


This is H/W blcok diagram of our Robot.
![H:W 다이어그램 001](https://user-images.githubusercontent.com/66055313/97284762-c54f7280-1884-11eb-9304-952abccadbb0.jpeg)





Main H/Ws are   

* NVIDIA Xavier AGX   
* VLP-16 3D LiDAR   
* LDS-01 2D LiDAR
* 2 Logitech C920 Cameras 
* FLIR ONE Gen3 Thermal Camera   
* Raspberry Monitor


-NVIDIA Xavier AGX will be used as a main board with Ubuntu 18.04 Linux environment.   
-VLP-16 LiDAR will be used for SLAM.   
-Upper Camera will be used for mask detection.   
-Lower Camera and 2D LiDAR will be used for collision avoidance and to detect people.   
-Theremal Camera will be used to detect people's temperature.   
-Monitor will display if people are wearing mask or not and sound an alarm when at least one person is not wearing a mask.   
-CUDA, Jetpack4.3, PyTorch, Tensorflow used     

#Upper Camera   
It detects whether people have wore a mask properly or not.   

#Lower Camera and 2D LiDAR
It stops when the camera detects people's foot, otherwise if it's just an obstacle or a wall, it executes collision-avoidance self driving.   

#SLAM with VLP16 LiDAR   
We used BLAM(Berkley Localization and Mapping) for slam which has a loop closure.   
<img width="752" alt="스크린샷 2021-01-05 오후 3 32 41" src="https://user-images.githubusercontent.com/66055313/103614195-6173b500-4f6b-11eb-9c5f-11b3cda6808d.png">   

#We also used Autoware for Autnomous Driving with 3D LiDAR  
Used pcd map file created with BLAM.   
Ndt Matching for Localization.   
Test Video Link : https://youtu.be/gTM9PTE23JQ 

For mask detection, we forked codes from pyimagesearch's face detection and modified it.
For foot detection, we also forked codes from jetbot and modified it.
