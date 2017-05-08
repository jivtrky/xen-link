#ifndef IFNETLINK_H
#define IFNETLINK_H

#include <linux/if.h>
#include <linux/rtnetlink.h>

#define ENTRY(x) {x, #x}

#define IF_NAMESIZE 32

struct ifi_flag_s {
    unsigned flag;
    const char *name;
};

struct nlmrt_type_s {
    unsigned type;
    const char *name;
};

struct interface {
    int     index;
    int     flags;      /* IFF_UP etc. */
    long    speed;      /* Mbps; -1 is unknown */
    int     duplex;     /* DUPLEX_FULL, DUPLEX_HALF, or unknown */
    char    name[IF_NAMESIZE + 1];
};

#endif
