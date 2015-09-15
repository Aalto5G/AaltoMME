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
static char* logLevelStr[] = {"EMERG", "ALERT", "CRIT", "ERROR", "WARN", "NOTICE", "INFO", "DEBUG"};

/** @brief Function used to initialize the log_entity
 *  @param [in] app Application name to be shown on logs
 *  @param [in] priority priority level acording to syslog levels
 *
 *  level : This determines the importance of the message. The levels are, in order of decreasing importance:
 *  LOG_EMERG   system is unusable
 *  LOG_ALERT   action must be taken immediately
 *  LOG_CRIT    critical conditions
 *  LOG_ERR     error conditions
 *  LOG_WARNING warning conditions
 *  LOG_NOTICE  normal, but significant, condition
 *  LOG_INFO    informational message
 *  LOG_DEBUG   debug-level message
 */
void init_logger(const char * app, int priority){
  openlog(app, (LOG_PID | LOG_PERROR), LOG_USER);
  logger.priority = priority;
  setlogmask (LOG_UPTO (priority));
  log_msg(LOG_INFO, 0, "Log initialized with level %s", logLevelStr[priority]);
}

/** @brief Function to close the log_entity
 *  Currently is an optional function, but is recommended to use it to avoid
 *  compatibility issues in future versions*/
void close_logger(){
    log_msg(LOG_INFO, 0, "Closing Log");
    closelog();
}

/* @brief Change the level of the messages
 * @param [in] priority priority level acording to syslog levels*/
void change_logger_lvl(int priority){
    if(priority>=0 && priority<8){
        setlogmask (LOG_UPTO (priority));
        logger.priority=priority;
        log_msg(LOG_INFO, 0, "Changed log level: %s", logLevelStr[priority]);
    }else{
        log_msg(LOG_ERR, 0, "Couldn't Change Log level, value incorrect %d", priority);
    }
}

/*/@brief send log message
 * @param [in] priority priority level acording to syslog levels
 * @param [in] filename
 * @param [in] line
 * @param [in] errno error number
 * @param [in] fmt format, printf equivalent*/
void log_msg_(int pri, char *fn, const char *func, int ln, int en, char *fmt, ...) {
  va_list args;
  char buf[SYSERR_MSGSIZE];

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

  va_start(args, fmt);
  vsnprintf(buf, SYSERR_MSGSIZE, fmt, args);
  va_end(args);
  buf[SYSERR_MSGSIZE-1] = 0; /* Make sure it is null terminated */

  if(logger.priority >= LOG_DEBUG){
	  if (en)
		  syslog(pri, "[%s] %s - %d (%s) %s <%s, %s(), %d>",
		         timeStr, logLevelStr[pri], en, strerror(en), buf,
		         fn, func, ln);
	  else
		  syslog(pri, "[%s] %s - %s <%s, %s(), %d>",
		         timeStr, logLevelStr[pri], buf,
		         fn, func, ln);
  }else{
	  if (en)
		  syslog(pri, "[%s] %s - %d (%s) %s",
		         timeStr, logLevelStr[pri], en, strerror(en), buf);
	  else
		  syslog(pri, "[%s] %s - %s", timeStr, logLevelStr[pri], buf);
  }
}

/*/@brief send log message
 * @param [in] priority priority level acording to syslog levels
 * @param [in] filename
 * @param [in] line
 * @param [in] errno error number, use 0 by default.
 * @param [in] peer source address
 * @param [in] pack packet buffer
 * @param [in] len packet length
 * @param [in] fmt format, printf equivalent*/
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

/**@ simple function to print a buffer
 * @param [in] buf buffer pointer
 * @param [in] size size of the buffer
 * */
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
