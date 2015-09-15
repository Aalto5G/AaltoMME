/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**@file   commands.c
 * @author Vicent Ferrer
 * @date   March, 2013
 * @brief  Implementation of some server functions to recv commands
 * */

#include <stdio.h>
#include <stdlib.h>

#include <errno.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <stdarg.h>

#include "MME.h"
#include "S1Assoc.h"
#include "commands.h"
#include "logmgr.h"

/*-****************************************************************/
typedef struct{
    gpointer mme;
    struct evconnlistener *listener;
}CommandSrv_t;

typedef struct{
    gpointer           mme;
    CommandSrv_t       *srv;
    struct evbuffer    *output;
    struct evbuffer    *input;
    struct bufferevent *bev;
}CommandConn_t;

const char banner[] = ""
    "**************************************************\n"
    "*          Welcome to the MME cmd shell          *\n"
    "**************************************************\n";

const char prompt[] = "mme > ";

const char *menu_msg(){
    return "Menu:\n"
        "\tl level \tsend log level\n"
        "\th option \tshow option possible arguments\n"
        "\tm \t\tshow this menu\n"
        "\ts \t\tprint stats\n"
        "\tq \t\tquit console\n";
}

/* #define conn_print(self, fmt, ...)\ */
/*  evbuffer_add_printf( (* self ).output, fmt,  __VA_ARGS__) */

static int conn_vprint(CommandConn_t *self, const char* fmt, va_list args){
    return evbuffer_add_vprintf(self->output, fmt, args);
}

static int conn_print(CommandConn_t *self, const char* fmt, ...){
    va_list args;
    int r;
    va_start(args, fmt);
    r = conn_vprint(self, fmt, args);
    va_end(args);
    return r;
}

static void conn_stop(CommandConn_t *self){
    conn_print(self, "");
    bufferevent_free(self->bev);
    g_free(self);
}

void help_log_menu(CommandConn_t *self){
    conn_print(self,
               "Debug levels:\n"
               "This determines the importance of the message."
               " The levels are, in order of decreasing importance:\n"
               "\t%d \tLOG_EMERG   system is unusable\n"
               "\t%d \tLOG_ALERT   action must be taken immediately\n"
               "\t%d \tLOG_CRIT    critical conditions\n"
               "\t%d \tLOG_ERR     error conditions\n"
               "\t%d \tLOG_WARNING warning conditions\n"
               "\t%d \tLOG_NOTICE  normal, but significant, condition\n"
               "\t%d \tLOG_INFO    informational message\n"
               "\t%d \tLOG_DEBUG   debug-level message\n",
               0,1,2,3,4,5,6,7);
    /* LOG_EMERG, LOG_ALERT, LOG_CRIT, LOG_ERR, */
    /*  LOG_WARNING, LOG_NOTICE, LOG_INFO, LOG_DEBUG); */
}

static printAssoc(gpointer assoc, CommandConn_t *self){
    mme_GlobaleNBid gid;
    s1Assoc_getID(assoc, &gid);
    conn_print(self, "eNB: \t%u\t%u\t%.6x\t%s\n",
               globaleNB_getMCC(&gid),
               globaleNB_getMNC(&gid),
               globaleNB_getCI(&gid),
               s1Assoc_getName(assoc));
}

static void conn_printStats(CommandConn_t *self){
    GList *assocs = mme_getS1Assocs(self->mme);
    conn_print(self, "\t\t== Statistics==\n\n"
               "\tMCC\tMNC\teNB ID\teNB name\n");
    g_list_foreach(assocs, (GFunc)printAssoc, self);
    g_list_free(assocs);
}

static void process_line(CommandConn_t* self, char * line, size_t len){
    uint32_t args;
    char help_arg, option;
    /* gchar ** tockens = g_strsplit(line, " ", 3); */
    switch(line[0]){
    case 'l':
        sscanf(line, "%c %d\n", &option, &args);
        log_msg(LOG_DEBUG, 0, "Recv debug lvl change req: %d", args);
        change_logger_lvl(args);
        break;
    case 'm':
        conn_print(self, menu_msg());
        break;
    case 'h':
        sscanf(line, "%c %c\n", &option, &help_arg);
        switch(help_arg){
        case 'l':
            help_log_menu(self);
            break;
        default:
            conn_print(self, "Option not available\n");
            break;
        }
        break;
    case 's':
        conn_printStats(self);
        break;
    case 'q':
        conn_stop(self);
        return;
    default:
        conn_print(self, "Option not available: %c\n", line[0]);
        break;
    }
    conn_print(self, prompt);
}

static void cmd_read_cb(struct bufferevent *bev, void *ctx){
    char *line;
    size_t len;
    CommandConn_t* self = (CommandConn_t* ) ctx;

    /* This callback is invoked when there is data to read on bev. */
    struct evbuffer *input = bufferevent_get_input(bev);
    line = evbuffer_readln(input, &len, EVBUFFER_EOL_LF);

    process_line(self, line, len);
}

static void cmd_event_cb(struct bufferevent *bev, short events, void *ctx){
    CommandConn_t* self = (CommandConn_t* ) ctx;
    if (events & BEV_EVENT_ERROR)
        perror("Error from bufferevent");
    if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        bufferevent_free(bev);
    }
}

static void accept_conn_cb(struct evconnlistener *listener,
                           evutil_socket_t fd,
                           struct sockaddr *address,
                           int socklen,
                           void *ctx){
    CommandSrv_t* srv = (CommandSrv_t*)ctx;
    CommandConn_t* self = g_new0(CommandConn_t, 1);
    self->srv = srv;
    self->mme = srv->mme;
    /* We got a new connection! Set up a bufferevent for it. */
    struct event_base *base = evconnlistener_get_base(listener);

    self->bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    self->output = bufferevent_get_output(self->bev);
    self->input = bufferevent_get_input(self->bev);

    bufferevent_setcb(self->bev, cmd_read_cb, NULL, cmd_event_cb, self);

    bufferevent_enable(self->bev, EV_READ|EV_WRITE);

    conn_print(self, banner);
    conn_print(self, menu_msg());
    conn_print(self, prompt);
}

gpointer servcommand_init(gpointer mme, const int servPort){

    CommandSrv_t* self = g_new0(CommandSrv_t, 1);
    struct sockaddr_in sin;
    int servSock;
    struct event_base * base = mme_getEventBase(mme);

    self->mme=mme;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(servPort);

    self->listener = evconnlistener_new_bind(base, accept_conn_cb, self,
                                             LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE,
                                             -1,
                                             (struct sockaddr*)&sin,
                                             sizeof(sin));
    if (!self->listener) {
        err(1,"Couldn't create listener");
        return NULL;
    }

    log_msg(LOG_INFO, 0, "Open command server on port %d", servPort);
    return self;
}

extern gpointer servcommand_stop(gpointer serv_h){
    CommandSrv_t* self = serv_h;
    evconnlistener_free(self->listener);
    g_free(self);
}


void cmd_accept(evutil_socket_t listener, short event, void *arg){
    struct mme_t *mme = (struct mme_t *)arg;
}
