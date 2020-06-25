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
#define WIN_SIZE 3072
typedef struct
{
    bool_t done; /* TRUE once connection is closed */

    int connection_state; /* state of the connection (established, etc.) */
    tcp_seq initial_sequence_num;

    tcp_seq send_base, nextseqnum, rcv_base;
    tcp_seq cwnd, rwnd, swnd;
    tcp_seq send_buf[WIN_SIZE], recv_buf[WIN_SIZE];
    FILE *fp_s;
    FILE *fp_r;
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
    ts.tv_sec = tv.tv_sec + 100;
    ts.tv_nsec = tv.tv_usec * 1000;
    return ts;
}
#define SEND 0
#define RECV 1
void print_log(FILE *fp, struct tcphdr *pck, context_t *ctx, int flag)
{
    if (fp == NULL)
    {
        if (flag == SEND)
        {
            fp = fopen("client_log.txt", "w");
        }
        if (flag == RECV)
        {
            fp = fopen("server_log.txt", "w");
        }
    }
/*     if (flag == SEND)
        fprintf(stdout, "Send:\t %d \t %d \t %d\n", ctx->swnd, (ctx->swnd - ((ctx->nextseqnum) - (ctx->send_base))), pck->th_seq);
    if (flag == RECV)
        fprintf(stdout, "Recv:\t %d \t %d \t %d\n", ctx->swnd, (ctx->swnd - ((ctx->nextseqnum) - (ctx->send_base))), pck->th_seq);
 */}
void slw_start(context_t *ctx)
{
    if (ctx->cwnd < (4 * STCP_MSS))
    {
        ctx->cwnd = ctx->cwnd + STCP_MSS;
    }
    else
    {
        ctx->cwnd = ctx->cwnd + STCP_MSS * STCP_MSS / ctx->cwnd;
    }
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
    ctx->rwnd = WIN_SIZE;
    ctx->cwnd = STCP_MSS;
    ctx->swnd = STCP_MSS;
    /*
    * The sender window is the minimum of the other side's advertised receiver window, 
    * and the congestion window. 
    * The congestion window has dynamic size to support slow start.
    */

    /*
     * First SYN
     */

    struct tcphdr *syn;

    if (is_active)
    {
        syn = (struct tcphdr *)calloc(1, STCP_MSS);
        syn->th_seq = ctx->initial_sequence_num;
        syn->th_off = 5;
        syn->th_flags = TH_SYN;
        syn->th_win = STCP_MSS;

        print_log(ctx->fp_s, syn, ctx, SEND);
        stcp_network_send(sd, syn, STCP_MSS, NULL);

        struct tcphdr *syn_ack;
        syn_ack = (struct tcphdr *)calloc(1, STCP_MSS);
        unsigned int event;
        struct timespec ts_timeout;
        ts_timeout = time_out();
        event = stcp_wait_for_event(sd, NETWORK_DATA, &ts_timeout);

        stcp_network_recv(sd, syn_ack, STCP_MSS);
        print_log(ctx->fp_r, syn_ack, ctx, RECV);

        assert(syn_ack->th_flags == (TH_SYN | TH_ACK));
        ctx->swnd = MIN(syn_ack->th_win, ctx->rwnd);
        ctx->initial_sequence_num = ctx->initial_sequence_num + 1;
        ctx->nextseqnum = ctx->nextseqnum + 1;
        ctx->rcv_base = syn_ack->th_seq + 1;
        ctx->send_base = ctx->send_base + 1;
        slw_start(ctx);
        /* ACK */
        struct tcphdr *ack;
        ack = (struct tcphdr *)calloc(1, STCP_MSS);
        ack->th_seq = ctx->initial_sequence_num;
        ack->th_ack = syn_ack->th_seq + 1;
        ack->th_flags = TH_ACK;
        ack->th_off = 5;

        print_log(ctx->fp_s, ack, ctx, SEND);
        stcp_network_send(sd, ack, STCP_MSS, NULL);

        /* code */
        free(syn);
        free(ack);
        free(syn_ack);
    }
    else
    {
        struct timespec ts_timeout;
        unsigned int event;
        ts_timeout = time_out();
        event = stcp_wait_for_event(sd, NETWORK_DATA, &ts_timeout);
        struct tcphdr *syn;
        struct tcphdr *syn_ack;
        syn = (struct tcphdr *)calloc(1, STCP_MSS);
        assert(event == NETWORK_DATA);

        stcp_network_recv(sd, syn, STCP_MSS);
        print_log(ctx->fp_r, syn, ctx, RECV);

        assert(syn->th_flags == TH_SYN);
        /* 
         * todo check syn
         */

        ctx->initial_sequence_num = ctx->initial_sequence_num + 1;
        ctx->nextseqnum = ctx->nextseqnum + 1;
        ctx->rcv_base++;
        ctx->swnd = MIN(syn->th_win, ctx->rwnd);

        syn_ack = (struct tcphdr *)calloc(1, ctx->swnd);
        syn_ack->th_flags = TH_SYN | TH_ACK;
        syn_ack->th_ack = syn->th_seq;
        syn_ack->th_seq = ctx->initial_sequence_num;
        syn_ack->th_off = 5;
        syn_ack->th_win = STCP_MSS;

        print_log(ctx->fp_s, syn_ack, ctx, SEND);
        stcp_network_send(sd, syn_ack, STCP_MSS, NULL);

        /*
         *ACK received
         */
        event = stcp_wait_for_event(sd, NETWORK_DATA, &ts_timeout);
        struct tcphdr *ack;
        ack = (struct tcphdr *)calloc(1, STCP_MSS);
        assert(event == NETWORK_DATA);
        stcp_network_recv(sd, ack, STCP_MSS);
        print_log(ctx->fp_r, ack, ctx, RECV);

        assert(ack->th_flags == TH_ACK);
        ctx->initial_sequence_num++;
        ctx->nextseqnum++;
        ctx->send_base++;
        slw_start(ctx);
        /* code */
        free(syn);
        free(syn_ack);
        free(ack);
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
            ctx->swnd = MIN(ctx->cwnd, ctx->rwnd);
            dst = (void *)calloc(1, ctx->swnd);
            tcp_seq len;
            len = stcp_app_recv(sd, dst, ctx->swnd);
            assert(len > 0);
            int t;
            t = MIN(ctx->swnd, len);
            printf("len:%d \t ctx->swnd:%d \t t:%d\n", len, ctx->swnd, t);
            struct tcphdr *hdr;
            while (t > 0)
            {
                printf("SEND message\n");
                printf("len:%d \t ctx->swnd:%d \t t:%d\n", len, ctx->swnd, t);
                printf("%d\n", t);

                hdr = (struct tcphdr *)calloc(1, STCP_MSS);
                hdr->th_seq = ctx->initial_sequence_num;
                hdr->th_off = 5;
                hdr->th_win = ctx->swnd;
                void *send_buf;
                send_buf = dst;
                if (*((char *)send_buf) == '\0')
                {
                    printf("PASS\n");
                    break;
                }
                /* void *buf;
                buf = calloc(1, STCP_MSS);
                memcpy(buf, hdr, sizeof(struct tcphdr)); */
                printf("send_buf_len:%d\n", MIN(STCP_MSS - sizeof(struct tcphdr), t));
                /* int len_s = MIN(STCP_MSS - sizeof(struct tcphdr), t);
                memset(hdr + sizeof(struct tcphdr), 0, STCP_MSS - sizeof(struct tcphdr));*/

                memcpy(hdr + sizeof(struct tcphdr), send_buf, MIN(STCP_MSS - sizeof(struct tcphdr), t));
                /* char N[STCP_MSS] = {0};
                if (t < STCP_MSS - sizeof(struct tcphdr))
                {
                    memcpy(hdr + sizeof(struct tcphdr) + t, N, STCP_MSS - sizeof(struct tcphdr) - t);
                } */
                printf("%s\n", (((char *)(hdr + sizeof(struct tcphdr)))));
                dst = dst + MIN(STCP_MSS - sizeof(struct tcphdr), t);
                t = t - MIN(STCP_MSS - sizeof(struct tcphdr), t);
                /* send_buf = (struct tcphdr *)calloc(1, STCP_MSS - sizeof(struct tcphdr)); */

                /* len = stcp_app_recv(sd, send_buf, STCP_MSS - sizeof(struct tcphdr));
                if (len <= 0)
                    break; */
                /* stcp_network_send(sd, hdr, sizeof(struct tcphdr), send_buf, STCP_MSS - sizeof(struct tcphdr), NULL); */
                printf("hdr size:%d\n",sizeof(struct tcphdr));
                stcp_network_send(sd, hdr, STCP_MSS /* len_s+sizeof(struct tcphdr) */, NULL);
                free(hdr);
                printf("send message finish\n");
                print_log(ctx->fp_s, hdr, ctx, SEND);

                /*
                else
                {
                    continue;
                } */
            }
            /* printf("recv ACK\n");
            hdr = (struct tcphdr *)calloc(1, STCP_MSS);
            stcp_network_recv(sd, hdr, STCP_MSS);
            print_log(ctx->fp_r, hdr, ctx, RECV);
            assert((hdr)->th_flags == TH_ACK);
            ctx->initial_sequence_num++;
            ctx->nextseqnum++;
            ctx->send_base++;
            slw_start(ctx); */
        }
        if (event & NETWORK_DATA)
        {
            void *dst;
            struct tcphdr *hdr;
            dst = calloc(1, ctx->swnd);
            hdr = (struct tcphdr *)calloc(1, STCP_MSS);
            stcp_network_recv(sd, hdr, STCP_MSS);

            print_log(ctx->fp_r, hdr, ctx, RECV);
            if ((hdr)->th_flags == TH_ACK)
            {
                printf("recv ACK\n");
                ctx->initial_sequence_num++;
                ctx->nextseqnum++;
                ctx->send_base++;
                slw_start(ctx);
            }
            if ((hdr)->th_flags == TH_FIN)
            {
                printf("close\n");
                struct tcphdr *fin_ack;

                fin_ack = (struct tcphdr *)calloc(1, STCP_MSS);
                fin_ack->th_seq = ctx->initial_sequence_num;
                fin_ack->th_ack = (hdr)->th_seq + 1;
                fin_ack->th_flags = TH_ACK | TH_FIN;
                fin_ack->th_off = 5;

                print_log(ctx->fp_s, fin_ack, ctx, SEND);

                stcp_network_send(sd, fin_ack, STCP_MSS, NULL);

                struct tcphdr *ack;
                ack = (struct tcphdr *)calloc(1, STCP_MSS);
                stcp_network_recv(sd, ack, STCP_MSS);
                printf("close\n");
                print_log(ctx->fp_r, ack, ctx, RECV);
                free(ack);

                return;
                /* code */
            }
            if (hdr->th_flags == 0)
            {
                printf("recv message\n");
                ctx->rcv_base = hdr->th_seq + 1;
                ctx->swnd = MIN(hdr->th_win, ctx->rwnd);
                dst = hdr + sizeof(struct tcphdr);
                printf("%s\n", (((char *)dst)));

                print_log(ctx->fp_r, hdr, ctx, RECV);
                stcp_app_send(sd, dst, STCP_MSS - sizeof(struct tcphdr));
                struct tcphdr *ack;
                ack = (struct tcphdr *)calloc(1, STCP_MSS);
                ack->th_seq = ctx->initial_sequence_num;
                ack->th_ack = hdr->th_seq + 1;
                ack->th_flags = TH_ACK;
                ack->th_off = 5;
                stcp_network_send(sd, ack, STCP_MSS, NULL);
                print_log(ctx->fp_s, ack, ctx, SEND);
                printf("send ACK\n");
            }
            /* free(hdr); */
            //todo
        }
        if (event & APP_CLOSE_REQUESTED)
        {
            printf("close\n");
            struct tcphdr *FIN;
            FIN = (struct tcphdr *)calloc(1, STCP_MSS);
            FIN->th_flags = TH_FIN;
            FIN->th_seq = ctx->initial_sequence_num;
            FIN->th_off = 5;
            stcp_network_send(sd, FIN, STCP_MSS, NULL);

            struct tcphdr *FIN_ACK;
            FIN_ACK = (struct tcphdr *)calloc(1, STCP_MSS);
            stcp_network_recv(sd, FIN_ACK, STCP_MSS);
            print_log(ctx->fp_r, FIN_ACK, ctx, RECV);

            assert(FIN_ACK->th_flags == (TH_FIN | TH_ACK));
            struct tcphdr *ack;
            ack = (struct tcphdr *)calloc(1, STCP_MSS);
            ack->th_seq = ctx->initial_sequence_num;
            ack->th_ack = FIN_ACK->th_seq + 1;
            ack->th_flags = TH_ACK;
            ack->th_off = 5;
            stcp_network_send(sd, ack, STCP_MSS, NULL);
            print_log(ctx->fp_s, ack, ctx, SEND);
            break;
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
