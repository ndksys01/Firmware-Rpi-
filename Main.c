#include "Rpi_Setup/Check_Hardware.h"
#include "Connect_Server/connect_to_server.h"
#include "P2P_Stream/stream.h"
#include "stdio.h"




int main()
{	
	int tes;
	Rpi_setup();
	init_servo();
	connect_to_server();
	//tes = detect_motion();
	//while(1){usleep(1000);}
	int ret = 0;
	while (1)
	{
		/* Wait android request */
		init_waiting_done = FALSE;
		//video_send_gathering_done = FALSE;
		//text_gathering_done = FALSE;
		//detect_done = FALSE;
		printf ("Waiting android request ...\n");
		do{
			ret = wait_android_request();	
		}while(!ret);
		init_waiting_done = TRUE;
		printf("Accept android's request!\n");
		printf("Start streaming!\n");

		stream();
	}

}
