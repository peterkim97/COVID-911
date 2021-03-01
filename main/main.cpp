#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/LaserScan.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <cstring>
extern "C"{
#include<i2c/smbus.h>
#include <aarch64-linux-gnu/sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
}
#include <time.h>
#include <motor_final/motor_driver.h>

// for pipe communication
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
//socket communication


#define PORT 8888
char            data[7] ="Hello!";
int             serversock, clientsock;
void            quit(char* msg,int retval);
struct sockaddr_in      server, client;
    int                     accp_sock;
    int                     addrlen =sizeof(client);
    int                     bytes;
    int                     dataSize =sizeof(data);
    int                     from_client;
    int                     i;

// Debug mode
 #define DEBUG 1

void socket_init(){

    printf("Data Size is : %d\n", dataSize);
 
    /* open socket */
    if ((serversock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        quit("socket() failed", 1);
    }
 
    /* setup server's IP and port */
    memset(&server, 0,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;
 
    /* bind the socket */
    if (bind(serversock, (struct sockaddr *)&server,sizeof(server)) == -1) {
        quit("bind() failed", 1);
    }
 
    printf("start listen....\n");
    // wait for connection
    if(listen(serversock, 10) == -1){
        quit("listen() failed.", 1);
    }
    printf("client wait....\n");
 
    accp_sock = accept(serversock, (struct sockaddr *)&client, (socklen_t*)&addrlen);
    if(accp_sock < 0){
        quit("accept() failed", 1);
    }
 
    bytes = send(accp_sock, &data, dataSize, 0);
 
    if(bytes != dataSize){
        fprintf(stderr,"Connection closed. bytes->[%d], dataSize->[%d]\n",bytes, dataSize);
        close(accp_sock);
 
        if ((accp_sock = accept(serversock, NULL, NULL)) == -1) {
            quit("accept() failed", 1);
        }
    }
}

void socket_server(){
    bytes = 0;
    for(i=0; i<sizeof(from_client); i+= bytes){
    bytes = recv(accp_sock, &from_client + i,sizeof(from_client) - i, 0);
        if (bytes == -1) {
            quit("recv failed", 1);
        }
    }

}

void quit(char* msg,int retval)
{
    if (retval == 0) {
        fprintf(stdout, (msg == NULL ?"" : msg));
        fprintf(stdout,"\n");
    }else {
        fprintf(stderr, (msg == NULL ?"" : msg));
        fprintf(stderr,"\n");
    }
 
    if (clientsock) close(clientsock);
    if (serversock) close(serversock);
 
    exit(retval);
}


// Calibrated for main BLDC motor

#define THROTTLE_FULL_FORWARD 2400//Lock max speed
#define THROTTLE_FULL_REVERSE 1850//Lock max backward speed

//RND Parameters
#define THROTTLE_FORWARD 2250
#define THROTTLE_NEUTRAL 2250
#define THROTTLE_REVERSE 2070

//Direction Parameters
#define DIR_NEUTRAL 2120

#define SERVO_CHANNEL 10
#define ESC_CHANNEL 8

int getkey() {
    int character;
    struct termios orig_term_attr;
    struct termios new_term_attr;

    /* set the terminal to raw mode */
    tcgetattr(fileno(stdin), &orig_term_attr);
    memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
    new_term_attr.c_lflag &= ~(ECHO|ICANON);
    new_term_attr.c_cc[VTIME] = 0;
    new_term_attr.c_cc[VMIN] = 0;
    tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);

    /* read a character from the stdin stream without blocking */
    /*   returns EOF (-1) if no character is available */
    character = fgetc(stdin);

    /* restore the original terminal attributes */
    tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);

    return character;
}

char mode;
int chk;
int restart = 0;


float LDS01Distance_F[30]; // 2D LiDAR Foward 30 degree's distance(165~195)
float LDS01Distance_L[30]; // 2D LiDAR Left-Forward 30 degree's distance(220~250)
float LDS01Distance_R[30]; // 2D LiDAR Right-Forward 30 degree's distance(110~140)

float LDS_R_90[10];	   // 2D LiDAR Right 10 degree's distance (90~100)
float LDS_L_270[10];	   // 2D LiDAR Left 10 degree's distance (260~270)

//Filter some error(too close or beyond range) distances from LiDAR
float MinDist(float list[],int cnt,int num){
	int div=num/cnt;
	float min=10000.0;
	float Avg;
	float temp;
	for(int i=0;i<cnt;i++){
		
		for(int j=0;j<div;j++){
            //if too close or beyond range -> set to 2 meter
			if(list[div*i+j] == 0 || list[div*i+j] > 2)
			list[div*i+j] = 2;

			temp=list[div*i+j];
			if(min>temp) min=temp;
		}
	}
	
	return min;
}

//Compute average distance for given distances
float AvgDist(float list[],int cnt){
	float Avg;
	float sum=0.0;
	for(int i=0;i<cnt;i++){
		if(list[i] == 0 || list[i] > 2)
			list[i] = 2;
		sum+=list[i];		
	}
	Avg=sum/cnt;
	return Avg;
}

//Callback function for 2D LiDAR datas
void LDS01Callback(const sensor_msgs::LaserScan::ConstPtr& msg){
	for(int i=0;i<10;i++)
	{
		float* L = (float*)(&msg->ranges[259+i]);
		LDS_L_270[i]=*L;
		float* R = (float*)(&msg->ranges[89+i]);
		LDS_R_90[i]=*R;
	}
	for(int i=0;i<30;i++)
	{
			float* ranges_F = (float*)(&msg->ranges[164+i]);
			LDS01Distance_F[i] = *ranges_F;
  
		float* ranges_L = (float*)(&msg->ranges[219+i]);
		LDS01Distance_L[i] = *ranges_L;
		float* ranges_R = (float*)(&msg->ranges[109+i]);
		LDS01Distance_R[29-i] = *ranges_R;	
	}
		
}
int DIR = DIR_NEUTRAL, ACC = THROTTLE_FORWARD,BACK = THROTTLE_REVERSE;

int main(int argc, char** argv) {

    socket_init();
    ros::init(argc, argv, "motor_distance");

    ros::NodeHandle n;
	 PCA9685 *pca9685 = new PCA9685();
    
    int err = pca9685->openPCA9685();
    if (err < 0){
        printf("Error: %d", pca9685->error);
    } else {
        printf("PCA9685 Device Address: 0x%02X\n",pca9685->kI2CAddress) ;
        pca9685->setAllPWM(0,0) ;
        pca9685->reset() ;
        pca9685->setPWMFrequency(300);
        
	sleep(1);
	pca9685->setPWM(ESC_CHANNEL, 0, THROTTLE_NEUTRAL);
	pca9685->setPWM(SERVO_CHANNEL, 0, DIR_NEUTRAL);
#ifdef DEBUG

        printf("Hit q key to exit\n");
        
	ros::Subscriber subLDS01Distance    = n.subscribe("/scan", 100, LDS01Callback);
        
	float middle_temp;
	int middle_flag=0;
	int stop_flag=0;
        while(pca9685->error >= 0){
	 socket_server();
            
	if(ACC>=(THROTTLE_FULL_FORWARD-150))		//Lock Speed
		ACC=(THROTTLE_FULL_FORWARD-150);
            
	else if(BACK<=THROTTLE_FULL_REVERSE)
		BACK=THROTTLE_FULL_REVERSE;


	if(DIR>2650)   //Lock Steering Angle
	DIR=2650;
            
	else if(DIR<1450)
	DIR=1450;

	int i=0;
            
	float Min_F=MinDist(LDS01Distance_F, 10,30);
	float Min_L=MinDist(LDS01Distance_L, 3,30);
	float Min_R=MinDist(LDS01Distance_R, 3,30);
            
	float Avg_L_270=AvgDist(LDS_L_270,10);
	float Avg_R_90=AvgDist(LDS_R_90,10);
            
	int servo;
	int esc;
	int servo_drive;
	int esc_drive;
	
	float sub=Min_L-Min_R; //for collision avoidance
	float middle=Avg_L_270-Avg_R_90; //to make it move in the middle

    int motor_speed=2345;

	    if(from_client==2)goto exit;
		
        //obstacle detection and collision avoidance
		if(Min_F >0.5 && Min_F<1.5){
			stop_flag = 1;
			servo=(int)(400*(sub));//Steering
			if(abs(servo) > 500){
				if(servo > 0)	
					servo = 500;
				else
					servo = -500;
			}
			esc=abs((int)(2*(sub)));
            
            //If detect people(foot) stop immediately
			if(from_client==1){
				DIR = DIR_NEUTRAL;
			 	pca9685->setPWM(SERVO_CHANNEL,0,DIR_NEUTRAL);
				pca9685->setPWM(ESC_CHANNEL,0,THROTTLE_NEUTRAL);	
				ROS_INFO("peopleeee");
			}
			
            //If not, drive
			else{
				pca9685->setPWM(SERVO_CHANNEL,0, DIR_NEUTRAL-servo);
				pca9685->setPWM(ESC_CHANNEL,0,motor_speed-esc);
			}
		}
        
        //Emergency Stop(Too Close)
		else if(Min_F > 0 && Min_F < 0.5){
				DIR = DIR_NEUTRAL;
			 	pca9685->setPWM(SERVO_CHANNEL,0,DIR_NEUTRAL);
				pca9685->setPWM(ESC_CHANNEL,0,THROTTLE_NEUTRAL);

				if(stop_flag){
					for(i=0;i<15;i++)
						pca9685->setPWM(ESC_CHANNEL,0,THROTTLE_NEUTRAL);
					for(i=0;i<15;i++)
						pca9685->setPWM(ESC_CHANNEL,0, BACK);
					for(i=0;i<15;i++)
						pca9685->setPWM(ESC_CHANNEL,0,THROTTLE_NEUTRAL);
				}
				stop_flag=0;
				for(int i=0;i<20;i++)
				{
				pca9685->setPWM(ESC_CHANNEL,0,THROTTLE_REVERSE);
				}
				
				pca9685->setPWM(ESC_CHANNEL,0,THROTTLE_NEUTRAL);
		}

        //Autonomous Drive
		else{
				ROS_INFO("!!   Normal   !!");
				stop_flag = 1;
				if((abs(middle_temp)-abs(middle))<0)
				{
					middle_flag=1;
				}		
				else
				{
					middle_flag=0;
				}

                //To drive at the middle
				if(middle>0){
					if(middle_flag){
						servo_drive=(int)((middle)*180);
						pca9685->setPWM(SERVO_CHANNEL,0, DIR_NEUTRAL-servo_drive);
					}
					else{
					pca9685->setPWM(SERVO_CHANNEL,0, DIR_NEUTRAL);
					}
					
				}
            
				else if(middle<0)
				{
					if(middle_flag){
						servo_drive=(int)((middle)*180);
						pca9685->setPWM(SERVO_CHANNEL,0, DIR_NEUTRAL-servo_drive);
					}
					else{
					pca9685->setPWM(SERVO_CHANNEL,0, DIR_NEUTRAL);
					}
				
				}
            
				else
				{
					pca9685->setPWM(SERVO_CHANNEL,0,DIR_NEUTRAL);
				}
            
			pca9685->setPWM(ESC_CHANNEL,0,motor_speed);
			middle_temp=middle;
		  }

	ros::spinOnce();
        }
	exit:
#endif

    sleep(1);
    pca9685->setPWM(SERVO_CHANNEL, 0, DIR_NEUTRAL);
    pca9685->setPWM(ESC_CHANNEL, 0, THROTTLE_NEUTRAL);
    pca9685->closePCA9685();
        
   }


    return 0;
    
}
