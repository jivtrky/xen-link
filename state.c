#include <stdlib.h>
#include <stdio.h>

#define MULTIPLIER 97
#define AGE_THRESHOLD 5

#ifdef DEBUG
#define debug_print(...) do { fprintf(stdout, __VA_ARGS__); } while(0)
#else
#define debug_print(...) do {} while (0)
#endif

struct ifstate_s {
    unsigned ifhash;
    unsigned state;
    short age;
    struct ifstate_s* next;
};

struct ifstate_s* ifstate;

static unsigned hash(const char* str)
{
    unsigned const char* us;
    unsigned h = 0;
    
    for (us = (unsigned const char*) str; *us; ++us)
    {
        h = h * MULTIPLIER + *us;
    }
    return h;
}

static struct ifstate_s* get_if(unsigned ifh)
{
    struct ifstate_s *ifs;
    debug_print("Get State\n");
    for (ifs = ifstate; ifs != NULL; ifs = ifs->next)
    {
        if (ifs->ifhash == ifh)
        {
            return ifs;
        }
    }
    return NULL;
}

static void remove_if(unsigned ifh)
{
    struct ifstate_s *ifp = NULL;
    struct ifstate_s *ifs = NULL;
    debug_print("Removing hash %u\n", ifh);
    for (ifs = ifstate; ifs != NULL; ifs = ifs->next) 
    {
        if (ifs->ifhash == ifh)
        {
            debug_print("Found hash %u\n", ifs->ifhash);
            if (ifs == ifstate)
            {
                ifstate = ifstate->next;
            }
            
            if (ifp) {
                ifp->next = ifs->next;
            }
            ifs->next = NULL;
            free(ifs);
            break;
        }
        ifp = ifs;
    }
}

static struct ifstate_s* add_if_state(unsigned ifh)
{
    struct ifstate_s *ifp = NULL;
    debug_print("Adding hash %u\n", ifh);
    struct ifstate_s *ifs = ifstate;

    while (ifs != NULL && ifs->ifhash != 0) 
    {
        debug_print("Not NULL, Not 0\n");
        ifp = ifs;
        ifs = ifs->next;
    }

    ifs = (struct ifstate_s*)malloc(sizeof(struct ifstate_s));
    if (ifstate == NULL)
    {
        ifstate = ifs;
    }
    
    debug_print("ifs is %p, ifstate is %p\n", ifp, ifstate);
    ifs->ifhash = ifh;
    ifs->next = NULL;
    if (ifp)
    {
        debug_print("Assigning next\n");
        ifp->next = ifs;
    }
    return ifs;
}

void set_if_state(const char* name, unsigned state)
{
    struct ifstate_s *ifs;
    unsigned ifh = hash(name);
    ifs = get_if(ifh);
    if (ifs == NULL)
    {
        debug_print("Did not find state\n");
        ifs = add_if_state(ifh);
    }
    else
        debug_print("Found State (%u); setting to: %u\n", ifs->ifhash, state);
    ifs->state = state;
    ifs->age = 0;
}

unsigned and_states()
{
    unsigned state = 1;
    struct ifstate_s *ifs;
    for (ifs = ifstate; ifs != NULL; ifs = ifs->next)
    {
        debug_print("hash %u has state %u and age %u\n", 
            ifs->ifhash, ifs->state, ifs->age);
        if (ifs->age < AGE_THRESHOLD) {
            state &= ifs->state;
            ifs->age += 1;
        }
        else 
        {
            debug_print("hash TOO OLD, age %d\n", ifs->age);
            remove_if(ifs->ifhash);
        }
    }
    debug_print("Returning AND state %u\n", state);
    return state;
}

unsigned or_states()
{
    unsigned state = 0;
    struct ifstate_s *ifs;
    for (ifs = ifstate; ifs != NULL; ifs = ifs->next)
    {
        if (ifs->age < AGE_THRESHOLD) {
            state |= ifs->state;
            ifs->age += 1;
        }
        else 
        {
            debug_print("hash TOO OLD, age %d\n", ifs->age);
            remove_if(ifs->ifhash);
        }
    }
    return state;
}

