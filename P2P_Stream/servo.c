#include "stream.h"
//#include <wiringPi.h> 
//#include <softPwm.h>
//#include <wiringSerial.h>
int posHor=3; //default value
int posVer=11; //default value
int icount=0;
#define	MAX_PINS	1024

// The PWM Frequency is derived from the "pulse time" below. Essentially,
//	the frequency is a function of the range and this pulse time.
//	The total period will be range * pulse time in uS, so a pulse time
//	of 100 and a range of 100 gives a period of 100 * 100 = 10,000 uS
//	which is a frequency of 100Hz.
//
//	It's possible to get a higher frequency by lowering the pulse time,
//	however CPU uage will skyrocket as wiringPi uses a hard-loop to time
//	periods under 100uS - this is because the Linux timer calls are just
//	accurate at all, and have an overhead.
//
//	Another way to increase the frequency is to reduce the range - however
//	that reduces the overall output accuracy...

#define	PULSE_TIME	100

static int marks [MAX_PINS] ;
static int range [MAX_PINS] ;

int flag_f = 0;
int newPin = -1 ;
static int i_count =0;

/*
 * softPwmThread:
 *	Thread to do the actual PWM output
 *********************************************************************************
 */

void decode_send(char serial_input[181]){
	char temp1[181] = {0};
        char sender[181] = {0};
        int rc = 0;
        printf("[DECODING: %s\n",serial_input);
        switch(serial_input[1])
        {
                case '3': //disable warning sercurity
			serial_input[1] = '2';
			sprintf(temp1, "005$%s$%s$%s", "rpi001", "ceslab", serial_input);
                        break;
		case '4': //get status autolight 1 2
			serial_input[1] = '3';
			sprintf(temp1, "005$%s$%s$%s", "rpi001", "ceslab", serial_input);
			break;
		case '6': //turn led 1 off
			serial_input[1] = '5';
			sprintf(temp1, "005$%s$%s$%s", "rpi001", "ceslab", serial_input);
			break;
		case '7': //turn led 2 on
			serial_input[1] = '6';
			sprintf(temp1, "005$%s$%s$%s", "rpi001", "ceslab", serial_input);
			break;
		case '8': //turn led 2 off
                        serial_input[1] = '6';
                        sprintf(temp1, "005$%s$%s$%s", "rpi001", "ceslab", serial_input);
                        break;
		case '9': //turn led 3 on
                        serial_input[1] = '7';
                        sprintf(temp1, "005$%s$%s$%s", "rpi001", "ceslab", serial_input);
                        break;
		case 'a': //turn led 3 off
                        serial_input[1] = '7';
                        sprintf(temp1, "005$%s$%s$%s", "rpi001", "ceslab", serial_input);
                        break;
		case 'b': //turn led 4 on
                        serial_input[1] = '8';
                        sprintf(temp1, "005$%s$%s$%s", "rpi001", "ceslab", serial_input);
                        break;
		case 'c': //turn led 4 off
                        serial_input[1] = '8';
                        sprintf(temp1, "005$%s$%s$%s", "rpi001", "ceslab", serial_input);
                        break;
		case 'e': //turn Autoled 1 off
                        serial_input[1] = 'd';
                        sprintf(temp1, "005$%s$%s$%s", "rpi001", "ceslab", serial_input);
                        break;
		case 'k': // detected opened door
			sprintf(temp1,"006$rpi001$ceslab$Door");
			break;
		default:
			sprintf(temp1, "005$%s$%s$%s", "rpi001", "ceslab", serial_input);
			break;
        }
        printf("[Detected] SEND data %s \n", temp1);
        rc = Base64Encode(temp1, sender, BUFFFERLEN);
        send(global_socket,sender,181,NULL);
}

PI_THREAD (myThread)
{
int iSerial = 0;
while(1){
    char temp[181] = {0};
        while (serialDataAvail(fd)){
                temp[iSerial] = serialGetchar(fd);
              	printf("SERIAL get: %c", temp[iSerial]);
		 fflush(stdout);
		 if(temp[0] == '\0' || temp[0] == ' ') continue;
                printf("SERIAL get: %c", temp[iSerial]);
                        //serial_flag = ' ';
                        //if(temp[iSerial] == ' ') continue;
                printf("WAITING %d serial: %s\n",iSerial,temp);
                iSerial++;
                if(temp[iSerial-1] == '*'){
                        iSerial=0;
			decode_send(temp);
			memset(temp, '\0', 181);
                }
        }
}
}

static PI_THREAD (softPwmThread)
{
  int pin, mark, space ;

  pin    = newPin ;
  newPin = -1 ;

  piHiPri (50) ;

  while(1)
  {
  //if(i<30)

    i_count++;
    mark  = marks [pin] ;
    space = range [pin] - mark ;

    if (mark != 0)
      {
      if(i_count<25)
      {
      	digitalWrite (pin, HIGH) ;
      	}
      else
      	{
      		digitalWrite (pin, LOW) ;
      	}
      }
      delayMicroseconds (mark * 100) ;
	
    if (space != 0)
      digitalWrite (pin, LOW) ;
    delayMicroseconds (space * 100) ;
  }
   digitalWrite (pin, LOW) ;

  return NULL ;
}

/*
 * softPwmWrite:
 *	Write a PWM value to the given pin
 *********************************************************************************
 */

void softPwmWrite (int pin, int value)
{
  pin &= (MAX_PINS - 1) ;

  /**/ if (value < 0)
    value = 0 ;
  else if (value > range [pin])
    value = range [pin] ;

  marks [pin] = value ;
}


/*
 * softPwmCreate:
 *	Create a new PWM thread.
 *********************************************************************************
 */

int softPwmCreate (int pin, int initialValue, int pwmRange)
{
  int res ;

  pinMode      (pin, OUTPUT) ;
  digitalWrite (pin, LOW) ;

  marks [pin] = initialValue ;
  range [pin] = pwmRange ;

  newPin = pin ;
  res = piThreadCreate (softPwmThread) ;

  while (newPin != -1)
    delay (1) ;

  return res ;
}
void init_servo()
{
   if ((fd = serialOpen ("/dev/ttyAMA0", 9600)) < 0)
	  {
	    fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
	    return 1 ;
	  }
	printf("init fd: %d",fd);
	//inital Thread
	int x = piThreadCreate(myThread);
	if(x!=0)
	{
		printf("No ko the bat dau\n");
	}
   if (wiringPiSetup () == -1)
   {
        fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno)) ;
    	return 1 ;
   }
    // initial 0 & 1 pin PWM are output 
    pinMode (1, OUTPUT) ; // camera go up/down
    pinMode(0,OUTPUT); //camera turn lerf/right

    // pull 2 pin PWM to GND
    digitalWrite (1, LOW) ;
    digitalWrite(0, LOW);
    
    //setup 2 pin is PWM
    softPwmCreate(0,0,200);
    softPwmCreate(1,0,200);
    
    //setup the location defaults 
    i_count = 0;
    softPwmWrite(1,175+posHor);
    //delay(100);
    softPwmWrite(0,175+posVer);

    //delay(1000); 
}
void servo_rotate(char posChar)
{
	printf("GET IN SERVO: %c",posChar);
	switch(posChar)
	{
		case 'u': //camera go up
			i_count=0; 
			posHor++;
			if(posHor>20)posHor=20;
			softPwmWrite(1,175+posHor);
			printf("(^_^) Camera is going up.\n");
			break;
		case 'g': //camera go down
			i_count=0;
			posHor--;
			if(posHor<0)posHor=0;
			softPwmWrite(1,175+posHor);
			printf("(^_^) Camera is going down.\n");
			break;
		case 'l': //camera turn the lerf
			i_count=0;
			posVer--;
			if(posVer<0)posVer=0;
			softPwmWrite(0,175+posVer);
			printf("(^_^) Camera is turning left.\n");
			break;
		case 'r': //camera turn the right
			i_count=0;
			posVer++;
			if(posVer>20)posVer=20;
			softPwmWrite(0,175+posVer);
			printf("(^_^) Camera is turning right.\n");
			break;
		case '1':
			//get temperature data
			printf("(^_^) Send temperature data to server.\n");
			serialPuts (fd, "#1") ;
			break;
		case '2':
			// enable warning
			printf("(^_^) Enable security warning.\n");
			serialPuts (fd, "#2") ;
			break;
		case '3':
			// disable warning
			printf("(^_^) Disable security warning.\n");
			serialPuts (fd, "#3") ;
			break;
		case '4':	
			// get status light 1 2 ...
			printf("(^_^) Get status all Lights in your house.\n");
			serialPuts (fd, "#4") ;
			break;
		case '5':
			// turn on/off light 1
			serialPuts (fd, "#5") ;
			printf("\nTurn on light (1) : %d\n",fd);
			break;
		case '6':
			// turn on/off light 2
			serialPuts (fd, "#6") ;
			printf("\nTurn off light (1) : %d\n",fd);
			break;
		case '7':
			// turn on/off light 2
			serialPuts (fd, "#7") ;
			printf("\nTurn on light (2) : %d\n",fd);
			break;
		case '8':
			// turn on/off light 2
			serialPuts (fd, "#8") ;
			printf("\nTurn off light (2) : %d\n",fd);
			break;
		case '9':
			// turn on/off light 2
			serialPuts (fd, "#9") ;
			printf("\nTurn on light (3) : %d\n",fd);
			break;
		case 'a':
			// turn on/off light 2
			serialPuts (fd, "#a") ;
			printf("\nTurn off light (3) : %d\n",fd);
			break;
		case 'b':
			// turn on/off light 2
			serialPuts (fd, "#b") ;
			printf("\nTurn on light (4) : %d\n",fd);
			break;
		case 'c':
			// turn on/off light 2
			serialPuts (fd, "#c") ;
			printf("\nTurn off light (4) : %d\n",fd);
			break;
		case 'd':
			// turn on/off light 2
			serialPuts (fd, "#d") ;
			break;
		case 'e':
			// turn on/off light 2
			serialPuts (fd, "#e") ;
			break;
		case 'f':
			// Update all data
			serialPuts (fd, "#f") ;
			printf("\nUpdate data : %d\n",fd);
			break;
	}
}
