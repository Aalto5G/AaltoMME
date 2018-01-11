/* AaltoMME - Mobility Management Entity for LTE networks
 * Copyright (C) 2013 Vicent Ferrer Guash & Jesus Llorente Santos
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file   eNBemulator.c
 * @Author Vicent Ferrer
 * @date   May, 2013
 * @brief  simple eNB emulator
 *
 * Tool to test S1 capabilities during development.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <netinet/sctp.h>
#include <event2/event.h>

#include "logmgr.h"
#include "S1AP.h"

#define FIRSTMESSAGE "/home/vicent/S1APmsg/S1AP_id-S1Setup.s1"
#define SECONDMESSAGE "/home/vicent/S1APmsg/S1AP_id-initialUEMessageReq.s1"
#define S1AP_CONTROL_STREAM0 0
#define S1AP_CONTROL_STREAM1 1

char *msgpath[] = {"/home/vicent/S1APmsg/S1AP_id-S1Setup.s1",
                   "/home/vicent/S1APmsg/S1AP_id-initialUEMessageReq.s1",
                   "/home/vicent/S1APmsg/S1AP_id-uplinkNASTransport.s1",
                   "/home/vicent/S1APmsg/S1AP_id-uplinkNASTransport-SecurityModeComplete.s1",
                   "/home/vicent/S1APmsg/S1AP_id-InitialContextSetup_Rsp.s1",
                   "/home/vicent/S1APmsg/S1AP_id-uplinkNASTransport_AttachComplete.s1"
};

void send_testMsg(int sock, int sid, int i){
	FILE *f;
	uint8_t buffer[500];
	int  n, ret;
	f = fopen(msgpath[i], "rb");
	if (f){
		n = fread(buffer, 1, 500, f);
	}
	else{
		printf("error opening file\n");
	}
	fclose(f);

	 ret = sctp_sendmsg( sock, (void *)buffer, (size_t)n, NULL, 0,
	                     SCTP_S1AP_PPID, 0, sid, 0, 0 );
	 if (ret<0){
		 log_msg(LOG_DEBUG, errno, "sctp_sendmsg error");
		 close(sock);
		 exit(1);
	 }
}


void s1_accept(evutil_socket_t listener, short event, void *arg){
    int in=0, flags, n, ret;
    uint8_t buffer[500];
    struct sctp_sndrcvinfo sndrcvinfo;
    struct event *ev = *(struct event **)arg;
    FILE *f;
    static int i=1;


    in = sctp_recvmsg( listener, (void *)buffer, sizeof(buffer),
                        (struct sockaddr *)NULL, 0, &sndrcvinfo, &flags );

    if (in > 0) {
        buffer[in] = 0;
        if (sndrcvinfo.sinfo_stream == 0) {
            printf("Recv from Stream0 \n");
            printfbuffer(buffer, in);
        } else if (sndrcvinfo.sinfo_stream == 1) {
            printf("Recv from Stream1 \n");
            printfbuffer(buffer, in);
        }
    }else{
	    printf("Nothing received, closing \n");
        /*Close socket*/
        close(listener);
        /*Delete event*/
        event_free(ev);
    }

    if(i<6){
	    send_testMsg(listener, sndrcvinfo.sinfo_stream, i);
	    i++;
	    if(i==5){
		    send_testMsg(listener, sndrcvinfo.sinfo_stream, i);
		    i++;
	    }
    }else{
        exit (0);
    }
}


int main(int argc, char **argv)
{
    int connSock, in, ret, n;
    struct sockaddr_in servaddr;
    struct sctp_status status;
    struct sctp_event_subscribe events;
    struct sctp_initmsg initmsg;
    char buffer[500];
    FILE *f;

    /*libevent*/
    struct event_base *base;
    struct event *listener_s1;
    struct event **cb_arg;

    /*Init syslog*/
    init_logger("eNBemulator", LOG_DEBUG);

    if(argc!=1){
        log_msg(LOG_DEBUG, 0, "Parameter number not correct. Usage:\n\eNBemulator\n");
        exit(1);
    }

    base = event_base_new();
    if (!base)
    {
        log_msg(LOG_DEBUG, 0, "Failed to create libevent event-base");
        exit(1);
    }


    /* Create an SCTP TCP-Style Socket */
    connSock = socket( AF_INET, SOCK_STREAM, IPPROTO_SCTP );

    /* Specify that a maximum of 5 streams will be available per socket */
    memset( &initmsg, 0, sizeof(initmsg) );
    initmsg.sinit_num_ostreams = 5;
    initmsg.sinit_max_instreams = 5;
    initmsg.sinit_max_attempts = 4;
    ret = setsockopt( connSock, IPPROTO_SCTP, SCTP_INITMSG,
                     &initmsg, sizeof(initmsg) );

    /* Specify the peer endpoint to which we'll connect */
    bzero( (void *)&servaddr, sizeof(servaddr) );
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(S1AP_PORT);
    servaddr.sin_addr.s_addr = inet_addr( "127.0.0.1" );

    /* Connect to the server */
    ret = connect( connSock, (struct sockaddr *)&servaddr, sizeof(servaddr) );

    /* Enable receipt of SCTP Snd/Rcv Data via sctp_recvmsg */
    memset( (void *)&events, 0, sizeof(events) );
    events.sctp_data_io_event = 1;
    ret = setsockopt( connSock, SOL_SCTP, SCTP_EVENTS,
                     (const void *)&events, sizeof(events) );

    /* Read and emit the status of the Socket (optional step) */
    in = sizeof(status);
    ret = getsockopt( connSock, SOL_SCTP, SCTP_STATUS,
                     (void *)&status, (socklen_t *)&in );

    printf("assoc id  = %d\n", status.sstat_assoc_id );
    printf("state     = %d\n", status.sstat_state );
    printf("instrms   = %d\n", status.sstat_instrms );
    printf("outstrms  = %d\n", status.sstat_outstrms );

    /* Expect two messages from the peer */

    /*for (i = 0 ; i < 2 ; i++) {

    in = sctp_recvmsg( connSock, (void *)buffer, sizeof(buffer),
                        (struct sockaddr *)NULL, 0, &sndrcvinfo, &flags );

    if (in > 0) {
      buffer[in] = 0;
      if (sndrcvinfo.sinfo_stream == 0) {
        printf("(Local) %s\n", buffer);
      } else if (sndrcvinfo.sinfo_stream == 1) {
        printf("(GMT  ) %s\n", buffer);
      }
    }

    }*/


    /********** Send first message **********/
    //snprintf( buffer, 500, "Test %d", i);
    f = fopen(msgpath[0], "rb");
    if (f){
        n = fread(buffer, 1, 500, f);
    }
    else{
        printf("error opening file\n");
        close(connSock);
        return 1;
    }
    fclose(f);

    ret = sctp_sendmsg( connSock, (void *)buffer, (size_t)n, NULL, 0, SCTP_S1AP_PPID, 0, S1AP_CONTROL_STREAM0, 0, 0 );

    if (ret<0){
      log_msg(LOG_DEBUG, errno, "sctp_sendmsg error");
      close(connSock);
      return 1;
    }

    printf("send %d bytes\n", ret);

    cb_arg = &listener_s1;
    listener_s1 = event_new(base, connSock, EV_READ|EV_PERSIST, s1_accept, cb_arg);

    event_add(listener_s1, NULL);
    evutil_make_socket_nonblocking(connSock);
    event_base_dispatch(base);
    log_msg(LOG_DEBUG, 0, "End of loop.\n");

    return 0;
}
