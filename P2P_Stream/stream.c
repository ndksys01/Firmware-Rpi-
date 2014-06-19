#include "stream.h"

void* _check_connection()
{
	char buffer[181] = {0};
	char receiver[181] = {0};
	char *tmp, *tmp2;
	int rc = 0;

	while (text_gathering_done == FALSE) usleep(100);

	int ret = recv(global_socket, buffer, 181, NULL);

	if( ret != -1)// Good
	{
		rc = Base64Decode(buffer, receiver, BUFFFERLEN);
		printf("Android's username exit[Full message] = %s\n", receiver);
		tmp = strtok (receiver,"$");
		tmp = strtok (NULL,"$");
		tmp = strtok (NULL,"$");
		tmp = strtok (NULL,"$");
		tmp2 = strtok (tmp," ");
		printf("Android's username exit = %s\n", tmp2);

		if (!strcmp(tmp2,originBuf))
		{
			printf("Android exit\n");
			g_main_loop_quit (gloop);
		}
	}
}

int stream ()
{
	GThread *video_send, *text_receive, *check_connection, *motion_detect;

	/* Init Glib */
	g_type_init();
	gloop = g_main_loop_new(NULL, FALSE);
	io_stdin = g_io_channel_unix_new(fileno(stdin));

	/* Allocate data */
	RpiData_SendVideo = (Rpi_Data*)malloc(sizeof(Rpi_Data));
	//RpiData_SendAudio = (Rpi_Data*)malloc(sizeof(Rpi_Data));
	//RpiData_ReceiveAudio = (Rpi_Data*)malloc(sizeof(Rpi_Data));
	RpiData_Text = (Rpi_Data*)malloc(sizeof(Rpi_Data));


	video_send_gathering_done = FALSE;
	//send_audio_gathering_done = FALSE;
	//receive_audio_gathering_done = FALSE;
	text_gathering_done = FALSE;
	detect_done = FALSE;

	/* Init video streaming */
	motion_detect = g_thread_new("Motion detection", &_detect_main, NULL);
	video_send = g_thread_new("send video", &_video_send_main, NULL);
	//send_audio = g_thread_new("send audio", &_send_audio_main, NULL);
	//audio_receive = g_thread_new("receive audio", &_audio_receive_main, NULL);
	text_receive = g_thread_new("text send+receive", &_text_receive_main, NULL);
	check_connection = g_thread_new("Check if android exit or not", &_check_connection, NULL);
	

	/* Unalocate all object */
	g_main_loop_run (gloop);

	printf("\n[stream]g_main_loop_quit (gloop);\n");

	/* Free libnice agent & gstreamer pipeline */

	/* Free send video */
	printf("[stream] Agent = %d!\n", RpiData_SendVideo->agent);
	g_object_unref(RpiData_SendVideo->agent);
	gst_object_unref (RpiData_SendVideo->bus);
	gst_element_set_state (RpiData_SendVideo->pipeline, GST_STATE_NULL);
	gst_object_unref (RpiData_SendVideo->pipeline);

	/* Free text */
	g_object_unref(RpiData_Text->agent);


	free(RpiData_SendVideo);
	free(RpiData_Text);

	/* Free threads */
	g_thread_join (motion_detect);
	g_thread_join (video_send);
	//g_thread_join (send_audio);
	//g_thread_join (audio_receive);
	g_thread_join (text_receive);
	g_thread_join (check_connection);
	g_main_loop_unref (gloop);

	return 0;
}
