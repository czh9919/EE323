/*
 * transport.c 
 *
 * CS244a HW#3 (Reliable Transport)
 *
 * This file implements the STCP layer that sits between the
 * mysocket and network layers. You are required to fill in the STCP
 * functionality in this file. 
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "mysock.h"
#include "stcp_api.h"
#include "transport.h"

enum
{
    CSTATE_ESTABLISHED
}; /* obviously you should have more states */

/* this structure is global to a mysocket descriptor */
#define win_size 3072
typedef struct
{
    bool_t done; /* TRUE once connection is closed */

    int connection_state; /* state of the connection (established, etc.) */
    tcp_seq initial_sequence_num;

    tcp_seq send_base, nextseqnum, rcv_base;
    tcp_seq cwnd, rwnd, swnd;
    tcp_seq send_buf[win_size], recv_buf[win_size];
    /* any other connection-wide global variables go here */
} context_t;

static void generate_initial_seq_num(context_t *ctx);
static void control_loop(mysocket_t sd, context_t *ctx);

/* initialise the transport layer, and start the main loop, handling
 * any data from the peer or the application.  this function should not
 * return until the connection is closed.
 */

struct timespec time_out()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct timespec ts;
    ts.tv_sec = tv.tv_sec + 10;
    ts.tv_nsec = tv.tv_usec * 1000;
    return ts;
}
void print_log_send(struct tcphdr *pck, context_t *ctx)
{
    printf("Send:\t %d \t %d \t %d", ctx->swnd, ctx->rwnd, pck->th_seq);
}
void transport_init(mysocket_t sd, bool_t is_active)
{
    context_t *ctx;

    ctx = (context_t *)calloc(1, sizeof(context_t));
    assert(ctx);

    generate_initial_seq_num(ctx);

    ctx->send_base = ctx->initial_sequence_num;
    ctx->rcv_base = ctx->initial_sequence_num;
    ctx->nextseqnum = ctx->initial_sequence_num;
    /*
    * The sender window is the minimum of the other side's advertised receiver window, 
    * and the congestion window. 
    * The congestion window has dynamic size to support slow start.
    */

    /*
     * First SYN
     */

    struct tcphdr *syn;
    syn = (struct tcphdr *)calloc(1, sizeof(struct tcphdr));
    if (is_active)
    {
        syn->th_seq = ctx->initial_sequence_num;
        syn->th_off = 5;
        syn->th_flags = TH_SYN;
        syn->th_win = win_size;

        stcp_network_send(sd, syn, sizeof(struct tcphdr));

        struct tcphdr *syn_ack;
        syn_ack = (struct tcphdr *)calloc(1, sizeof(struct tcphdr));
        unsigned int event;
        struct timespec ts_timeout;
        ts_timeout = time_out();
        event = stcp_wait_for_event(sd, NETWORK_DATA, &ts_timeout);
        stcp_network_recv(sd, syn_ack, sizeof(struct tcphdr));
        assert(syn_ack->th_flags == TH_SYN | TH_ACK);
        ctx->initial_sequence_num = ctx->initial_sequence_num + 1;
        ctx->nextseqnum = ctx->nextseqnum + 1;
        ctx->rcv_base = syn_ack->th_seq + 1;
        ctx->send_base = ctx->send_base + 1;

        /* ACK */
        struct tcphdr *ack;
        ack = (struct tcphdr *)calloc(1, sizeof(struct tcphdr));
        ack->th_seq = ctx->initial_sequence_num;
        ack->th_ack = syn_ack->th_seq + 1;
        ack->th_flags = TH_ACK;
        ack->th_off = 5;
        stcp_network_send(sd, ack, sizeof(struct tcphdr));
        /* code */
    }
    else
    {
        struct timespec ts_timeout;
        unsigned int event;
        ts_timeout = time_out();
        event = stcp_wait_for_event(sd, NETWORK_DATA, &ts_timeout);
        struct tcphdr *syn;
        struct tcphdr *syn_ack;
        assert(event == NETWORK_DATA);
        stcp_network_recv(sd, syn, sizeof(struct tcphdr));
        assert(syn->th_flags==TH_SYN);
         /* 
         * todo check syn
         */

        ctx->initial_sequence_num = ctx->initial_sequence_num + 1;
        ctx->nextseqnum = ctx->nextseqnum + 1;
        ctx->rcv_base = syn_ack->th_seq + 1;
        ctx->send_base = ctx->send_base + 1;//!wrong ,should slow start

        syn_ack=(struct tcphdr *)calloc(1, sizeof(struct tcphdr));
        syn_ack->th_flags=TH_SYN|TH_ACK;
        syn_ack->th_ack=syn->th_seq;
        syn_ack->th_seq=ctx->initial_sequence_num;
        syn->th_off = 5;

        stcp_network_send(sd,syn,sizeof(struct tcphdr));
        /* code */
    }

    /* XXX: you should send a SYN packet here if is_active, or wait for one
     * to arrive if !is_active.  after the handshake completes, unblock the
     * application with stcp_unblock_application(sd).  you may also use
     * this to communicate an error condition back to the application, e.g.
     * if connection fails; to do so, just set errno appropriately (e.g. to
     * ECONNREFUSED, etc.) before calling the function.
     */
    ctx->connection_state = CSTATE_ESTABLISHED;
    stcp_unblock_application(sd);

    control_loop(sd, ctx);

    /* do any cleanup here */
    free(ctx);
}

/* generate initial sequence number for an STCP connection */
static void generate_initial_seq_num(context_t *ctx)
{
    assert(ctx);
    ctx->initial_sequence_num = 1;
}

/* control_loop() is the main STCP loop; it repeatedly waits for one of the
 * following to happen:
 *   - incoming data from the peer
 *   - new data from the application (via mywrite())
 *   - the socket to be closed (via myclose())
 *   - a timeout
 */
static void control_loop(mysocket_t sd, context_t *ctx)
{
    assert(ctx);

    while (!ctx->done)
    {
        unsigned int event;

        /* see stcp_api.h or stcp_api.c for details of this function */
        /* XXX: you will need to change some of these arguments! */
        struct timespec ts_timeout;
        ts_timeout = time_out();
        event = stcp_wait_for_event(sd, ANY_EVENT, &ts_timeout);

        /* check whether it was the network, app, or a close request */
        if (event & APP_DATA)
        {
            /* the application has requested that data be sent */
            /* see stcp_app_recv() */
            void *dst;
            stcp_network_recv(sd,dst,ctx->rwnd);

        }
        if (event & NETWORK_DATA)
        {
            //todo
        }
        if (event & APP_CLOSE_REQUESTED)
        {
            //todo
        }
        /* etc. */
    }
}

/**********************************************************************/
/* our_dprintf
 *
 * Send a formatted message to stdout.
 * 
 * format               A printf-style format string.
 *
 * This function is equivalent to a printf, but may be
 * changed to log errors to a file if desired.
 *
 * Calls to this function are generated by the dprintf amd
 * dperror macros in transport.h
 */
void our_dprintf(const char *format, ...)
{
    va_list argptr;
    char buffer[1024];

    assert(format);
    va_start(argptr, format);
    vsnprintf(buffer, sizeof(buffer), format, argptr);
    va_end(argptr);
    fputs(buffer, stdout);
    fflush(stdout);
}
