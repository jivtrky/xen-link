#include <regex.h>
#include <stdlib.h>
#include <stdio.h>

int is_match(const char* name, const char* pattern)
{
    regex_t regex;
    size_t num = snprintf(NULL, 0, "^%s", pattern) + 1;
    char* fullpattern = malloc(num);
    snprintf(fullpattern, num, "^%s", pattern);
    
    int ret;
    int value = 0;
    
    ret = regcomp(&regex, fullpattern, 0);
    if (ret)
        value = -1;
    
    ret = regexec(&regex, name, 0, NULL, 0);
    if(!ret) {
        value = 1;
    }
    else if (ret == REG_NOMATCH) {
        value = 0;
    }
    else {
        value = -1;
    }
    
    free(fullpattern);
    regfree(&regex);
    return value;
}
