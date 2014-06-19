#include "stream.h"

static int count_det = 0;
GstElement *pipeline, *source, *sink, *filter, *queue, *decoder, *convert, *motions;
GstBus *bus;
GstMessage *msg;
GstStateChangeReturn ret;
GThread* connectThread;

static int _detected_ClientThread();
static void
bus_call (GstBus     *bus,
          GstMessage *msg,
          gpointer    data);

void *_detect_main(){
	while(init_waiting_done == FALSE)
		usleep(100);
	count_det = 0;
	connectThread = g_thread_create(_detected_ClientThread, NULL, FALSE, NULL);
	detect_motion();
}

void detect_motion(){
 
  //GMainLoop *loop;
  //guint bus_watch_id;

  /* Initialize GStreamer */
  gst_init (NULL, NULL);
  //loop = g_main_loop_new (NULL, FALSE);
   
  /* Create the elements */
  source = gst_element_factory_make ("rpicamsrc", "source");
  filter = gst_element_factory_make ("capsfilter", "filter");
  queue = gst_element_factory_make ("queue", "queue");
  decoder = gst_element_factory_make ("omxh264dec", "decoder");
  convert = gst_element_factory_make ("videoconvert", "convert");
  motions = gst_element_factory_make ("motioncells", "motions");
  sink = gst_element_factory_make ("autovideosink", "sink");

  //rpicamsrc
  g_object_set (source, "exposure-mode", 2, NULL);
  g_object_set (source, "fullscreen", FALSE, NULL);
  g_object_set (filter, "caps", gst_caps_from_string("video/x-h264,width=320,height=240,framerate=5/1"), NULL);
  g_object_set (queue, "max_size_buffers", 4096, NULL);
  g_object_set (motions, "postallmotion", TRUE, NULL);
 // g_object_set(motions,"threshold",0.05,NULL);
  /* Create the empty pipeline */
  pipeline = gst_pipeline_new ("test-pipeline");
   
  if (!pipeline || !source || !sink || !filter || !queue || !decoder || !convert || !motions ) {
    g_printerr ("Not all elements could be created.\n");
    return -1;
  }
  
  /* Build the pipeline */
  gst_bin_add_many (GST_BIN (pipeline), source, filter, queue, decoder, convert, motions, sink, NULL);
  if (gst_element_link_many (source, filter, queue, decoder, convert, motions, sink, NULL) != TRUE) {
    g_printerr ("Elements could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }
  bus = gst_element_get_bus (pipeline);
  gst_bus_enable_sync_message_emission (bus);
  gst_bus_add_signal_watch (bus);
  g_signal_connect (bus, "message::element",
      		(GCallback) bus_call, NULL);
  //bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
  //gst_object_unref (bus);

  /* Start playing */
  ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    gst_object_unref (pipeline);
    return -1;
  }
  //g_print ("Running...\n");
  //g_main_loop_run (loop);

  /* Free resources */
  //gst_object_unref (bus);
  //gst_element_set_state (pipeline, GST_STATE_NULL);
  //gst_object_unref (pipeline);
  //g_source_remove (bus_watch_id);
  //g_main_loop_unref (loop);
}
static void
bus_call (GstBus     *bus,
          GstMessage *msg,
          gpointer    data)
{
  GMainLoop *loop = (GMainLoop *) data;
  char sender[181] = {0};
  int rc = 0;
  if (!strcmp("motions",GST_MESSAGE_SRC_NAME(msg)))
{
	count_det++;
	//rc = Base64Encode("006$rpi001$ceslab$Detected", sender, BUFFFERLEN);
	//send(global_socket,sender,181,NULL);
	printf("Detection: %d\n",count_det);
	//gst_element_set_state (pipeline, GST_STATE_NULL);
  	//gst_object_unref (pipeline);
  //gst_element_set_state (pipeline, GST_STATE_NULL);
  //gst_object_unref (pipeline);
}
  if (count_det == 5)
{
	rc = Base64Encode("006$rpi001$ceslab$Detected", sender, BUFFFERLEN);
        send(global_socket,sender,181,NULL);
        //printf("%d\n",count_det);
	gst_object_unref (bus);
        gst_element_set_state (pipeline, GST_STATE_NULL);
        gst_object_unref (pipeline);
	//count_det=0;
	//printf("video_send_gathering_done: %d\n", video_send_gathering_done);
	//printf("text_gathering_done: %d\n", text_gathering_done);
	//printf("detect_done: %d\n", detect_done);
}
  //return TRUE;
}
static int _detected_ClientThread()
{
	char *header, *init , *dest, *data;
	char buffer[181] = {0};
	char temp[181]={0};
	char temp1[181]={0};
	char combine[181]={0};

	char receiver[181] = {0};
	char sender[181] = {0};
	int rc = 0;

	//memset(originBuf,'\0', sizeof(originBuf));
	//memset(destBuf,'\0', sizeof(destBuf));

	//Receive ice info from android
	while(1)
{
	memset(buffer,'\0', 181);
	memset(receiver,'\0', 181);
	memset(sender,'\0', 181);
	if(recv(global_socket, buffer, 181, NULL))
	{
		rc = Base64Decode(buffer, receiver, BUFFFERLEN);
		printf("[Detected] receive[Decode] = %s\n", receiver);

		header = strtok (receiver,"$");
		init = strtok (NULL,"$");
		dest = strtok (NULL,"$");
		data = strtok (NULL,"$");

		if(!strcmp(header,"100"))
		{
			//strncpy(originBuf,init,strlen(init));
			//strncpy(destBuf,dest, strlen(dest));
			printf("Change!!");
			if (count_det != 5)
			{
			gst_object_unref (bus);
			gst_element_set_state (pipeline, GST_STATE_NULL);
			//gst_object_unref (bus);
  			gst_object_unref (pipeline);
			}	
			detect_done= TRUE;
			return 0;
		}
		if(!strcmp(header,"005"))
		{
			if(!strcmp(data,"#51"))
			{
			servo_rotate ('5');
			//rc = Base64Encode("005$rpi001$ceslab$#51", sender, BUFFFERLEN);
			//send(global_socket,sender,181,NULL);
			continue;
			}
			if(!strcmp(data,"#50"))
			{
			servo_rotate ('6');
			//rc = Base64Encode("005$rpi001$ceslab$#50", sender, BUFFFERLEN);
			//send(global_socket,sender,181,NULL);
			continue;
			}
			if(!strcmp(data,"#61"))
			{
			servo_rotate ('7');
			//rc = Base64Encode("005$rpi001$ceslab$#61", sender, BUFFFERLEN);
			//send(global_socket,sender,181,NULL);
			continue;
			}
			if(!strcmp(data,"#60"))
			{
			servo_rotate ('8');
			//rc = Base64Encode("005$rpi001$ceslab$#60", sender, BUFFFERLEN);
			//send(global_socket,sender,181,NULL);
			continue;
			}
			if(!strcmp(data,"#71"))
			{
			servo_rotate ('9');
			//rc = Base64Encode("005$rpi001$ceslab$#71", sender, BUFFFERLEN);
			//send(global_socket,sender,181,NULL);
			continue;
			}
			if(!strcmp(data,"#70"))
			{
			servo_rotate ('a');
			//rc = Base64Encode("005$rpi001$ceslab$#70", sender, BUFFFERLEN);
			//send(global_socket,sender,181,NULL);
			continue;
			}
			if(!strcmp(data,"#81"))
			{
			servo_rotate ('b');
			//rc = Base64Encode("005$rpi001$ceslab$#81", sender, BUFFFERLEN);
			//send(global_socket,sender,181,NULL);
			continue;
			}
			if(!strcmp(data,"#80"))
			{
			servo_rotate ('c');
			//rc = Base64Encode("005$rpi001$ceslab$#80", sender, BUFFFERLEN);
			//send(global_socket,sender,181,NULL);
			continue;
			}
			if(!strcmp(data,"#d1"))
                        {
                        servo_rotate ('d');
                        //rc = Base64Encode("005$rpi001$ceslab$#80", sender, BUFFFERLEN);
                        //send(global_socket,sender,181,NULL);
                        continue;
                        }
			if(!strcmp(data,"#e0"))
                        {
                        servo_rotate ('3');
                        //rc = Base64Encode("005$rpi001$ceslab$#80", sender, BUFFFERLEN);
                        //send(global_socket,sender,181,NULL);
                        continue;
                        }
			 if(!strcmp(data,"#e1"))
                        {
                        servo_rotate ('2');
                        //rc = Base64Encode("005$rpi001$ceslab$#80", sender, BU$
                        //send(global_socket,sender,181,NULL);
                        continue;
                        }

			 if(!strcmp(data,"#d0"))
                        {
                        servo_rotate ('e');
                        //rc = Base64Encode("005$rpi001$ceslab$#80", sender, BU$
                        //send(global_socket,sender,181,NULL);
                        continue;
                        }

			if(!strcmp(data,"#f"))
			{
			printf("DETECTED #F");
			servo_rotate ('f');
			continue;
			}
			if(!strcmp(data,"#3")){
				printf("Update 2 auto leds");
				servo_rotate('4');
				continue;
			}
		}

	}
}
}
