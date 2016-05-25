/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   errmagr.c
 * @author Vicent Ferrer
 * @date   March, 2013
 * @brief  Error managment functions.
 *
 * Based on Syslog functions.(2003, 2004 Mondru AB.)
 */

#include <stdarg.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <libgen.h>

#include "logmgr.h"

/** Structure to store the logger variables.
 *  Currently it is storing only the priority.
 */
struct log_entity{
  int priority;
};

/** msgentity instance*/
static struct log_entity logger;

/** Log Level dictionary to convert the priority to a string*/
static char* logLevelStr[] = {"EMER", "ALER", "CRIT", "ERRO", "WARN", "NOTI", "INFO", "DEBG"};

void init_logger(const char * app, int priority){
  /*openlog(app, (LOG_PID | LOG_PERROR), LOG_USER);*/
  logger.priority = priority;
  setlogmask (LOG_UPTO (priority));
  log_msg(LOG_INFO, 0, "Log initialized with level %s", logLevelStr[priority]);
}

void close_logger(){
  log_msg(LOG_INFO, 0, "Closing Log");
  /*closelog();*/
}


void change_logger_lvl(int priority){
  if(priority>=0 && priority<8){
    setlogmask (LOG_UPTO (priority));
    logger.priority=priority;
    log_msg(LOG_INFO, 0, "Changed log level: %s", logLevelStr[priority]);
  }else{
    log_msg(LOG_ERR, 0, "Couldn't Change Log level, value incorrect %d", priority);
  }
}


void log_msg_s(int pri, char *fn, const char *func, int ln, int en, const char *msg) {
  char timeStr[128];
  struct timeval tv;
  struct tm *pTm;
  gettimeofday( &tv, NULL );
  pTm = localtime ( (const time_t*)&tv.tv_sec );
  strftime ((char*) timeStr, 128, "%d/%m/%Y %T", pTm );
  if(logger.priority >= LOG_DEBUG){
    sprintf((char*)(timeStr + strlen((char*)timeStr)),
            ".%06u",
            (unsigned int)tv.tv_usec);
  }
 /* if(logger.priority >= LOG_DEBUG){ */
  if(0){
	if (en)
      syslog(pri, "[%s] %s - %d (%s) %s <%s(), %s:%d>",
             timeStr, logLevelStr[pri], en, strerror(en), msg,
             func, basename(fn), ln);
    else
	  syslog(pri, "[%s] %s - %s <%s(), %s:%d>",
	         timeStr, logLevelStr[pri], msg,
             func, basename(fn), ln);
  }else if(0){
	if (en)
      syslog(pri, "[%s] %s - %d (%s) %s",
             timeStr, logLevelStr[pri], en, strerror(en), msg);
    else
      syslog(pri, "[%s] %s - %s", timeStr, logLevelStr[pri], msg);
  }else{
    if(pri <= logger.priority){
      if (en)
        printf("%s - %d (%s) %s\n",
               logLevelStr[pri], en, strerror(en), msg);
      else
        printf("%s - %s\n", logLevelStr[pri], msg);
    }
    fflush(stdout);
  }
}


void log_msg_(int pri, char *fn, const char *func, int ln, int en, char *fmt, ...) {
  va_list args;
  char buf[SYSERR_MSGSIZE];

  va_start(args, fmt);
  vsnprintf(buf, SYSERR_MSGSIZE, fmt, args);
  buf[SYSERR_MSGSIZE-1] = 0; /* Make sure it is null terminated */
  log_msg_s(pri, fn, func, ln, en, buf);
  va_end(args);
}


void log_errpack_(int pri, char *fn, const char *func, int ln, int en, struct sockaddr_in *peer,
         void *pack, unsigned len, char *fmt, ...) {

  va_list args;
  char buf[SYSERR_MSGSIZE];
  char buf2[SYSERR_MSGSIZE];
  unsigned int n;
  int pos;

  va_start(args, fmt);
  vsnprintf(buf, SYSERR_MSGSIZE, fmt, args);
  va_end(args);
  buf[SYSERR_MSGSIZE-1] = 0;

  snprintf(buf2, SYSERR_MSGSIZE, "Packet from/to %s:%u, length: %d, content:",
       inet_ntoa(peer->sin_addr),
       ntohs(peer->sin_port),
       len);
  buf2[SYSERR_MSGSIZE-1] = 0;
  pos = strlen(buf2);
  for(n=0; n<len; n++) {
    if ((pos+4)<SYSERR_MSGSIZE) {
      sprintf((buf2+pos), " %02hhx", ((unsigned char*)pack)[n]);
      pos += 3;
    }
  }
  buf2[pos] = 0;

  if (en)
    syslog(pri, "%s - %d (%s) %s. %s <%s, %s(),%d> ", logLevelStr[pri], en, strerror(en), buf, buf2, fn, func, ln);
  else
    syslog(pri, "%s - %s. %s <%s, %s(),%d>", logLevelStr[pri], buf, buf2, fn, func, ln);

}


void printfbuffer(uint8_t *buf, uint32_t size){
  uint32_t i;
  printf("The buffer looks like: n %u\n", size);
  for( i = 0; i < size; i=i+2) {
    printf("%.2x%.2x ", buf[i], buf[i+1]);
    if(i%16==14){
      printf("\n");
    }
  }
  printf("\n");
}
