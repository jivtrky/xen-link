#include <asm/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include "ifnetlink.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>

FILE* FILEPTR = NULL;
char* FILE_PATH = "/var/run/linkstate";

extern int is_match(const char* name, const char* pattern);
extern void set_if_state(const char* name, unsigned state);
extern unsigned and_states();

struct cmd_params_s {
    const char *iface;
    const char *watch;
    int daemon;
} cmd_params = {
    NULL,
    NULL,
    0
};

struct ifi_flag_s ifi_flag_map[] = {
    ENTRY(IFF_UP),
    ENTRY(IFF_BROADCAST),
    ENTRY(IFF_DEBUG),
    ENTRY(IFF_LOOPBACK),
    ENTRY(IFF_POINTOPOINT),
    ENTRY(IFF_NOTRAILERS),
    ENTRY(IFF_RUNNING),
    ENTRY(IFF_NOARP),
    ENTRY(IFF_PROMISC),
    ENTRY(IFF_ALLMULTI),
    ENTRY(IFF_MASTER),
    ENTRY(IFF_SLAVE),
    ENTRY(IFF_MULTICAST),
    ENTRY(IFF_PORTSEL),
    ENTRY(IFF_AUTOMEDIA),
    ENTRY(IFF_DYNAMIC),
    ENTRY(IFF_LOWER_UP),
    ENTRY(IFF_DORMANT),
    ENTRY(IFF_ECHO),
};

struct nlmrt_type_s nlmrt_type_map[] = {
    ENTRY(RTM_NEWLINK),
    ENTRY(RTM_DELLINK),
    ENTRY(RTM_GETLINK),
    ENTRY(RTM_SETLINK),
    ENTRY(RTM_NEWADDR ),
    ENTRY(RTM_DELADDR),
    ENTRY(RTM_GETADDR),
    ENTRY(RTM_NEWROUTE),
    ENTRY(RTM_DELROUTE),
    ENTRY(RTM_GETROUTE),
    ENTRY(RTM_NEWNEIGH),
    ENTRY(RTM_DELNEIGH),
    ENTRY(RTM_GETNEIGH),
    ENTRY(RTM_NEWRULE),
    ENTRY(RTM_DELRULE),
    ENTRY(RTM_GETRULE),
    ENTRY(RTM_NEWQDISC),
    ENTRY(RTM_DELQDISC),
    ENTRY(RTM_GETQDISC),
    ENTRY(RTM_NEWTCLASS),
    ENTRY(RTM_DELTCLASS),
    ENTRY(RTM_GETTCLASS),
    ENTRY(RTM_NEWTFILTER),
    ENTRY(RTM_DELTFILTER),
    ENTRY(RTM_NEWACTION),
    ENTRY(RTM_DELACTION),
    ENTRY(RTM_GETACTION),
    ENTRY(RTM_NEWPREFIX),
    ENTRY(RTM_GETMULTICAST),
    ENTRY(RTM_GETANYCAST),
    ENTRY(RTM_NEWNEIGHTBL ),
    ENTRY(RTM_GETNEIGHTBL ),
    ENTRY(RTM_SETNEIGHTBL),
    ENTRY(RTM_NEWNDUSEROPT ),
    ENTRY(RTM_NEWADDRLABEL ),
    ENTRY(RTM_DELADDRLABEL),
    ENTRY(RTM_GETADDRLABEL),
    ENTRY(RTM_GETDCB ),
    ENTRY(RTM_SETDCB),
    ENTRY(RTM_NEWNETCONF ),
    ENTRY(RTM_GETNETCONF ),
    ENTRY(RTM_NEWMDB ),
    ENTRY(RTM_DELMDB ),
    ENTRY(RTM_GETMDB ),
    ENTRY(RTM_NEWNSID ),
    ENTRY(RTM_DELNSID ),
    ENTRY(RTM_GETNSID ),
    ENTRY(RTM_NEWSTATS ),
    ENTRY(RTM_GETSTATS ),
};

void write_data(unsigned flags)
{
    fseek(FILEPTR, 0, SEEK_SET);
    int err = fprintf(FILEPTR, "%u\n", flags);
    fflush(FILEPTR);
    if (err < 0) {
        perror("Error writing flags");
    }
}

void print_flags_and_exit()
{
    int i;
    for (i = 0; i < sizeof ifi_flag_map/sizeof ifi_flag_map[0]; i++) 
    {
        printf("FLAG value: 0x%x\t\tname: %s\n",
               ifi_flag_map[i].flag, ifi_flag_map[i].name);
    }
    exit(0);
}

void stop()
{
    fclose(FILEPTR);
    unlink(FILE_PATH);
}

void handle_signals(int signal)
{
    exit(0);
}

int setup_signals()
{
    struct sigaction sa;

    sa.sa_handler = &handle_signals;
    sa.sa_flags = SA_RESTART;
    sigfillset(&sa.sa_mask);
    if (sigaction(SIGHUP, &sa, NULL) == -1) {
        return -1;
    }
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        return -1;
    }
    if (sigaction(SIGQUIT, &sa, NULL) == -1) {
        return -1;
    }

    return 0;
}

void read_msg(int len, struct nlmsghdr *buf, int write)
{
    struct rtattr *retrta;
	unsigned state = 1;

    // Loop through the msg hdr
    for (struct nlmsghdr *nh = buf; NLMSG_OK (nh, len);
        nh = NLMSG_NEXT (nh, len)) {
        struct ifinfomsg *ifimsg;
        /* The end of multipart message. */

        if (nh->nlmsg_type == NLMSG_DONE) {
            return;
        }

        if (nh->nlmsg_type == NLMSG_ERROR) {
            continue;
        }

        ifimsg = NLMSG_DATA(nh);
        // if we notice a change or a forced write is requested...
        if (ifimsg->ifi_change || write) {
            retrta = (struct rtattr*)IFLA_RTA(ifimsg);
            int attlen = IFLA_PAYLOAD(nh);
            // check the return attribute - a name?
            if (RTA_OK(retrta, attlen) &&
                    retrta->rta_type == IFLA_IFNAME)
            {
		// is it one we're looking for?
		if (is_match((const char*)RTA_DATA(retrta),
			 cmd_params.iface))
		{
                    set_if_state((const char*)RTA_DATA(retrta),
			(ifimsg->ifi_flags & IFF_UP));
							
                    state = and_states();
                    write_data(state);
                }
            }
        }
    }
}

void recv_msg(int fd)
{
    int len;
    char buf[8192];
    struct iovec iov = { buf, sizeof(buf) };
    struct sockaddr_nl sa;
    struct msghdr msg = { (void *)&sa, sizeof(sa), &iov, 1, NULL, 0, 0 };

    len = recvmsg(fd, &msg, 0);
    if(len == -1) {
        return;
    }

    read_msg(len, (struct nlmsghdr *) buf, 0);
}

void get_link_state()
{
	struct {
	    struct nlmsghdr nlhdr;
	    struct ifinfomsg infomsg;
	} msg;

	char buf[8192];
	
	// Set up the netlink socket
	int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);

	memset(&msg, 0, sizeof(msg));
	msg.nlhdr.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
	msg.nlhdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_ROOT;
	msg.nlhdr.nlmsg_type = RTM_GETLINK;
	msg.infomsg.ifi_family = AF_UNSPEC;
	
	// Send the first netlink message & receive response
	send(sock, &msg, msg.nlhdr.nlmsg_len, 0);
	int len = recv(sock, buf, sizeof(buf), 0);
	    
	read_msg(len,(struct nlmsghdr *)buf, 1);
}

void handle_params(int argc, char* argv[])
{
    int c, req = 0;

    cmd_params.daemon = 1;
    while((c = getopt(argc, argv, "nfi:")) != -1) {
        switch (c) {
        case 'f':
            print_flags_and_exit();
            break;

        case 'i':
            req = 1;
            cmd_params.iface = optarg;
            break;
        case 'n':
            cmd_params.daemon = 0;
            break;
        }
    }

    if (req == 0) {
        printf("missing -i option. Monitored interface type name.\n");
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    struct sockaddr_nl sa;
    int fd;

    handle_params(argc, argv);

    if (cmd_params.daemon)
    {
        int err = daemon(1, 1);
        if (err < 0) {
            perror("Failed to daemonize");
            return err;
        }
    }

    FILEPTR = fopen(FILE_PATH, "w+");
    if (FILEPTR == NULL)
    {
        perror("Failed to open file");
        return 1;
    }

    atexit(stop);

    if (setup_signals() < 0)
    {
        perror("Failed to setup Signal Handler");
        return 1;
    }

    memset(&sa, 0, sizeof(sa));
    sa.nl_family = AF_NETLINK;
    sa.nl_groups = RTMGRP_LINK;

    fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if(fd == -1)
    {
        perror("Failed to Open Socket");
        return 1;
    }

    if(bind(fd, (struct sockaddr *) &sa, sizeof(sa)) == -1)
    {
        perror("Failed to bind to socket");
        return 1;
    }

	// get the initial state of the interfaces we're interested in
    get_link_state();

    while(1) {
        recv_msg(fd);
    }

    return 0;
}
