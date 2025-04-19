

int utils_IfNameToIndex(const char *ifName)
{
    struct ifreq *request = (struct ifreq *) malloc(sizeof(struct ifreq));
    strcpy(request->ifr_name, "ppp0");
    
    if (ioctl(handler, SIOCGIFMTU, request) < 0) {
        log_error("ioctl error, can't get mtu: %s", strerror(errno));
    }

}

struct in6_addr utils_GetINet6Addr(const char *ifName)
{
}