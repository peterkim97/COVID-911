#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/LaserScan.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
#include <termios.h>
#include "std_msgs/String.h"
#include <string>

extern "C"{
#include<i2c/smbus.h>
#include <aarch64-linux-gnu/sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
}
#include <time.h>
#include <motor_final/SUNPWMPCA9685.h>

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
    //printf(" from_client size : %ld, Contents : %d\n",sizeof(from_client), from_client);
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
#define THROTTLE_FULL_REVERSE 1850//204 20ms 1241 // 1ms / 3.3ms * 4096
//#define PWM_NEUTRAL 307 //2048 // 1.5ms / 3.3ms * 4096
#define THROTTLE_FULL_FORWARD 2300 //2482 // 2ms / 3.3ms * 4096
#define DIR_NEUTRAL 1960

#define THROTTLE_NEUTRAL 2085 //2050//2048 // 1.5ms / 3.3ms * 4096
#define THROTTLE_FORWARD 2175 //2160
#define THROTTLE_REVERSE 1945//1940
//+- 500정도 움직임 2210부터 전진

#define SERVO_CHANNEL 2
#define ESC_CHANNEL 4

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
//float ZED2Distance;
float LDS01Distance_F[30]; // 165~195
float LDS01Distance_L[30]; // 220~250
float LDS01Distance_R[30]; // 110~140
float LDS_R_90[10];	   //90~100
float LDS_L_270[10];	   //260~270
float MinDist(float list[],int cnt,int num){
	int div=num/cnt;
	float min=10000.0;
	float Avg;
	float temp;
	for(int i=0;i<cnt;i++){
		//float sum=0.0;
		for(int j=0;j<div;j++){
			if(list[div*i+j] == 0 || list[div*i+j] > 2)
			list[div*i+j] = 2;

			temp=list[div*i+j];
			if(min>temp) min=temp;
			//sum+=list[div*i+j];	
		}
		//Avg=sum/div; 
		//if(min>Avg) min=Avg;
	}
	
	return min;
}

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
		//if(i<30){
			float* ranges_F = (float*)(&msg->ranges[164+i]);
			LDS01Distance_F[i] = *ranges_F;
		//}	  
		float* ranges_L = (float*)(&msg->ranges[219+i]);
		LDS01Distance_L[i] = *ranges_L;
		float* ranges_R = (float*)(&msg->ranges[109+i]);
		LDS01Distance_R[29-i] = *ranges_R;	
	}
	//printf("callback1\n");	
	//for(int p=0;p<30;p++)
	//	ROS_INFO("range_F[%d] :  %g", p, LDS01Distance_F[p]); 
}
int DIR = DIR_NEUTRAL, ACC = THROTTLE_FORWARD,BACK = THROTTLE_REVERSE;
double aangle,vvelocity;
void senderCallback(const std_msgs::String::ConstPtr& msg)
{
	//printf("callback2\n");
	//std::string str = "";
	//str=msg->angle.c_str();
	const char *temp;
	temp = msg->data.c_str();
	char angle[7];
	char velocity[7];

	for(int i=0;i<7;i++)
	angle[i]=temp[i];
	aangle=atof(angle);

	for(int i=8;i<15;i++)
	velocity[i-8]=temp[i];
	vvelocity=atof(velocity);

	//angle=(char*)(&msg->data);
	//aangle=angle;
	//double* velocity = (double*)(&msg->velocity);
	//vvelocity=velocity;	
	ROS_INFO("Receive Data: %g, %g",aangle,vvelocity);//msg->data.c_str());
}

int main(int argc, char** argv) {
    //socket setting
    socket_init();
    ros::init(argc, argv, "motor_distance");

    ros::NodeHandle n;
    ros::init(argc, argv, "rx");

    ros::NodeHandle a;
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
        // 27 is the ESC key
        printf("Hit 1 key to exit\n");
	ros::Subscriber subLDS01Distance    = n.subscribe("/scan", 100, LDS01Callback);
	
	ros::Subscriber sub = a.subscribe("sender", 1000, senderCallback);

	float middle_temp;
	int middle_flag=0;
	int stop_flag=0;
        while(pca9685->error >= 0){
	 socket_server();
	if(ACC>=(THROTTLE_FULL_FORWARD-150))		//안전장치 설정
		ACC=(THROTTLE_FULL_FORWARD-150);
	else if(BACK<=THROTTLE_FULL_REVERSE)
		BACK=THROTTLE_FULL_REVERSE;


	if(DIR>2650)
	DIR=2650;
	else if(DIR<1450)
	DIR=1450;
	//printf("%d\n",from_client);
    //mode=getkey();

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
	
	//서보 중립 2050, 좌우 최대 500씩
	//ROS_INFO("R : %g, L : %g F : %g ",Min_R,Min_L,Min_F);		
	float sub=Min_L-Min_R;		//old값이랑 비교 필요
	float middle=Avg_L_270-Avg_R_90;
	//ROS_INFO("R_90 : %g, L_270 : %g , middle : %g ",Avg_R_90,Avg_L_270, middle);
	//ROS_INFO("!!!!SUB : %g !!!!! ",sub);
	
int motor_speed=2185;

	    if(from_client==2)goto exit;

		pca9685->setPWM(ESC_CHANNEL,0,motor_speed);
		pca9685->setPWM(SERVO_CHANNEL,0, (int)((DIR_NEUTRAL+50) + aangle*16.7*3));
		printf("%d\n",(int)((DIR_NEUTRAL+50) + aangle*16.7*2));
		
	/*
		if(Min_F >0.5 && Min_F<1.5){//평균  LDS-MIN_R : 0, LDS-MIN_L : 0 애매하게 가까울때
			//모서리 돌기
			if(Avg_L_270==2)//왼쪽으로 돌 때
				{
				pca9685->setPWM(SERVO_CHANNEL,0, DIR_NEUTRAL-500);//좌회전
				pca9685->setPWM(ESC_CHANNEL,0,motor_speed);
				}
			else if(Avg_R_90==2)//오른쪽으로 돌 때
				{
				pca9685->setPWM(SERVO_CHANNEL,0, DIR_NEUTRAL+500);//우회전
				pca9685->setPWM(ESC_CHANNEL,0,motor_speed);
				}

			else//장애물 피하기 및 사람 감지
			{
				stop_flag = 1;
				servo=(int)(400*(sub));
				if(abs(servo) > 500){
					if(servo > 0)	
						servo = 500;
					else
						servo = -500;
				}
				esc=abs((int)(2*(sub)));
				if(from_client==1){
					DIR = DIR_NEUTRAL;
				 	pca9685->setPWM(SERVO_CHANNEL,0,DIR_NEUTRAL);
					pca9685->setPWM(ESC_CHANNEL,0,THROTTLE_NEUTRAL);	
					ROS_INFO("peopleeee");
				}
				
				else{
					pca9685->setPWM(SERVO_CHANNEL,0, DIR_NEUTRAL-servo);
					pca9685->setPWM(ESC_CHANNEL,0,motor_speed-esc);
				    }
			}
		}

		else if(Min_F > 0 && Min_F < 0.5){		//코닿을 거리일 때
				DIR = DIR_NEUTRAL;
			 	pca9685->setPWM(SERVO_CHANNEL,0,DIR_NEUTRAL);
				pca9685->setPWM(ESC_CHANNEL,0,THROTTLE_NEUTRAL);

				//후진 기어 바꾸기
				if(stop_flag){
					for(i=0;i<15;i++)
						pca9685->setPWM(ESC_CHANNEL,0,THROTTLE_NEUTRAL);
					for(i=0;i<15;i++)
						pca9685->setPWM(ESC_CHANNEL,0, BACK);
					for(i=0;i<15;i++)
						pca9685->setPWM(ESC_CHANNEL,0,THROTTLE_NEUTRAL);
					ROS_INFO("START STOPPPPPPPPPPP!");
				}
				stop_flag=0;
				for(int i=0;i<20;i++)
				{
				pca9685->setPWM(ESC_CHANNEL,0,THROTTLE_REVERSE);
				}
				
				pca9685->setPWM(ESC_CHANNEL,0,THROTTLE_NEUTRAL);
				ROS_INFO("Crashhhhhhhhhhhhhhh!");
		}

		else{//평상시 주행
				//ROS_INFO("!!   Normal   !!");
				stop_flag = 1;
				if((abs(middle_temp)-abs(middle))<0)
				{
					middle_flag=1;
				}		
				else
				{
					middle_flag=0;
				}


				if(middle>0){//왼쪽 정렬
					if(middle_flag){
						servo_drive=(int)((middle)*180);
						pca9685->setPWM(SERVO_CHANNEL,0, DIR_NEUTRAL-servo_drive);
					}
					else{
					pca9685->setPWM(SERVO_CHANNEL,0, DIR_NEUTRAL);
					}
					
				}
				else if(middle<0)//오른쪽 정렬
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
		  }*/

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
