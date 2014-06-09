/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <sys/queue.h>

#include "gtp.h"
#include "logmgr.h"
#include "storagesys.h"

/* Libevent. */
//#include <event.h>
#include <event2/event.h>

struct argcb{
  struct gsn_t *gsn;
  int cnt;
  struct event_base *base;
  char *dst;
};

int
check_for_old_version(void)
{
    const char *v = event_get_version();
    /* This is a dumb way to do it, but it is the only thing that works
       before Libevent 2.0. */
    if (!strncmp(v, "0.", 2) ||
        !strncmp(v, "1.1", 3) ||
        !strncmp(v, "1.2", 3) ||
        !strncmp(v, "1.3", 3)) {

        printf("Your version of Libevent is very old.  If you run into bugs,"
               " consider upgrading.\n");
        return -1;
    } else {
        printf("Running with Libevent version %s\n", v);
        return 0;
    }
}

void
do_accept(evutil_socket_t listener, short event, void *arg)
{/*
  struct gsn_t *gsn = arg;
  union gtp_packet packet;
  size_t len;
  struct sockaddr_in src_addr;
  socklen_t addrlen;


  if (listener == gsn->fd1c)
  {
    //gtp_decaps1u(gsn);
    gtp2_recv(gsn->fd1c, &packet, &len, &src_addr, &addrlen);
    printf("\nReceived packet from %s:%d\n", inet_ntoa(src_addr.sin_addr), src_addr.sin_port);
    printf("Packet type: %d\n*****Packet Dump:*****\n", packet.gtp2s.h.type);
    print_packet(&packet, len);
  }
  else
    return;*/
}

void do_send_echo_req(evutil_socket_t listener, short event, void *arg)
{
  struct argcb *cbargs = (struct argcb*)arg;
  //struct gsn_t *gsn = cbargs->gsn;
  struct in_addr destaddr;

  static int n=0;

  inet_aton(cbargs->dst, &destaddr);
  //struct timeval second_tv = {1,0};
  //gtp_echo_req(gsn, 2, NULL, &destaddr);
  printf("***** Send echo req #%d",n);
  if(cbargs->cnt == -1)
    printf("\n");
  else
  printf(" of %d\n", cbargs->cnt);
  n++;
  if (cbargs->cnt == n)
    event_base_loopbreak(cbargs->base);
}

int main(int argc, char **argv)
{

  struct event_base *base;
  //struct event *listener_event1c, *send_echo_req;//, *listener_event1u;
  //struct timeval second_tv = {1,0};
  int server=0;
  int count, i;
  char *dst;

  //struct gsn_t *gsn;              /* GSN instance            */
  /*int listen_fd;
  struct sockaddr_in listen_addr;*/
  //struct in_addr listen_;//, destaddr;
  //listen_.s_addr=INADDR_ANY;

  /*
  init_storage_system();
  free_storage_system();

  init_logger(LOG_DEBUG);*/
  //struct argcb cbargs;
  /*Param parser*/
  if(argc<2 || argc > 5)
  {
    printf("Incorrect number of parameters: eping [-s][-c count] destination.\n\ts\tServer Mode\n\tc\techo count\n");
    exit(1);
  }
  count = -1;
  for(i=1;i<argc;i++){
    if(argv[i][0] == '-'){
      if(argv[i][1] == 's'){
        server=1;
        printf("Mode server detected\n");
      }else if( argv[i][1] == 'c'){
        if(argv[i][2] == '\0'){
          i++;
          count = atoi(argv[i]);
        }
        else{
          count = atoi(argv[i]+2);
        }
        printf("count detected %d\n",count);
      }else{
        printf("Option not available: %s\n", argv[i]);
        exit(1);
      }
    }
  }
  if(!server){
    dst = argv[argc-1];
    printf("dest : %s\n", dst);
  }
  printf("eGTP-PING 0.1.\n");
  printf("gtplib version: %s\n", gtp_version());
  check_for_old_version();
  base = event_base_new();
  if (!base)
  {
      //log_msg(LOG_ERR, 0, "Failed to create libevent event-base");
      error(1, errno, "Failed to create libevent event-base");
      exit(1);
  }



  //inet_aton(dst, &destaddr);

  if (server){
    /*if (gtp_new(&gsn, ".",  &listen_, GTP_MODE_GGSN)) {
        error(1, errno,  "Failed to create gtp");
        exit(1);
    }
    listener_event1c = event_new(base, gsn->fd1c, EV_READ|EV_PERSIST, do_accept, (void*)gsn);
  //listener_event1u = event_new(base, gsn->fd1u, EV_READ|EV_PERSIST, gtp_decaps1u, (void*)base);*/
    //event_add(listener_event0, NULL);
    //event_add(listener_event1c, NULL);
  }
  else{/*
    if (gtp_new(&gsn, ".",  &listen_, GTP_MODE_NO_BIND)) {
        //log_msg(LOG_ERR, 0, "Failed to create gtp");
        error(1, errno, "Failed to create gtp");
        exit(1);
    }*/
    //cbargs.gsn=gsn;
    //cbargs.cnt=count;
    //cbargs.base=base;
    //cbargs.dst = dst;
    //send_echo_req = event_new(base, -1, EV_PERSIST, (event_callback_fn)do_send_echo_req, (void*)&cbargs);
    //listener_event1c = event_new(base, gsn->fd1c, EV_READ|EV_PERSIST, do_accept, (void*)gsn);
    //event_add(listener_event1c, NULL);
    //evtimer_add(send_echo_req, &second_tv);
  }

  //evutil_make_socket_nonblocking(gsn->fd0);
  //evutil_make_socket_nonblocking(gsn->fd1c);
  //evutil_make_socket_nonblocking(gsn->fd1u);

  event_base_dispatch(base);
  printf("End of loop.\n");
  //check_version_match();

  //gtp_free(gsn);
  /*close_logger();*/

  return 0;

}
