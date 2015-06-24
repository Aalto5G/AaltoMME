/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**@file   MMEcmd.c
 * @author Vicent Ferrer
 * @date   March, 2013
 * @brief  Client to send command to the MME
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

#include "logmgr.h"
#include "commands_msg.h"

void menu_msg(){
  printf("Menu:\n");
  printf("\ts signal\tsend signal\n");
  printf("\td level \tsend debug level\n");
  printf("\th option \tshow option possible arguments\n");
  printf("\tm \t\tshow this menu\n");
  printf("\tq \t\tquit application\n");
}

void debug_menu(){

  printf("Debug levels:\n");
  printf("This determines the importance of the message. The levels are, in order of decreasing importance:\n");
  printf("\t%d \tLOG_EMERG   system is unusable\n", LOG_EMERG);
  printf("\t%d \tLOG_ALERT   action must be taken immediately\n", LOG_ALERT);
  printf("\t%d \tLOG_CRIT    critical conditions\n", LOG_CRIT);
  printf("\t%d \tLOG_ERR     error conditions\n", LOG_ERR);
  printf("\t%d \tLOG_WARNING warning conditions\n", LOG_WARNING);
  printf("\t%d \tLOG_NOTICE  normal, but significant, condition\n", LOG_NOTICE);
  printf("\t%d \tLOG_INFO    informational message\n", LOG_INFO);
  printf("\t%d \tLOG_DEBUG   debug-level message\n", LOG_DEBUG );
}

int main(int argc, char**argv)
{
   int sockfd,n;
   struct sockaddr_in servaddr,cliaddr;
   char option, help_arg, line[25];
   int arg;
   uint64_t arg2;
   char recvline[1000];
   struct comm_tlv msg;

   if (argc != 2)
   {
      printf("usage:  MMEcmd <IP address>\n");
      return 1;
   }

   /*Init syslog*/
   init_logger("MMEcmd", LOG_DEBUG);

   sockfd=socket(AF_INET, SOCK_DGRAM,0);

   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr = inet_addr(argv[1]);
   servaddr.sin_port = htons(COMMAND_PORT);
   printf("Welcome to MMEcmd!\n");
   menu_msg();

   do{
     printf("> ");
     fgets(line, 25, stdin);
     switch(line[0]){
     case 'd':
       sscanf(line, "%c %d\n", &option, &arg);
       msg.t=(uint8_t)debug_level;
       msg.l=3;
       msg.v[0]=arg;
       if (sendto(sockfd, &msg, msg.l ,0, (struct sockaddr *)&servaddr, sizeof(servaddr))<1){
         log_msg(LOG_ERR, errno, "Error sending the command.");
       };
       break;
     case 'q':
       break;
     case 'm':
       menu_msg();
       break;
     case 'h':
       sscanf(line, "%c %c\n", &option, &help_arg);
       switch(help_arg){
       case 'd':
         debug_menu();
         break;
       case 's':
         printf("Info not available yet\n");
         break;
       default:
         printf("Option not available\n");
         break;
       }
       break;
     default:
       printf("Option not available: %c\n", line[0]);
       break;
     }

   }while (line[0]!='q');

   closelog(); /*Optional*/
}
