#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <unistd.h>
#include <time.h>


unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;
    result = sum >> 16;
    sum += result;
    return ~sum;
}

int ping(const char *target) {
    int sock;
    struct sockaddr_in addr, local_addr, r_addr;
    socklen_t addr_len = sizeof(r_addr);
    socklen_t local_addr_len = sizeof(local_addr);
    char packet[64];
    char response[1024];

    sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0) {
        perror("Failed on create socket");
        return 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(target);

    // socket raw
    struct icmphdr *icmp = (struct icmphdr *)packet;
    memset(packet, 0, sizeof(packet));
    icmp->type = ICMP_ECHO; // tipo do protocolo
    icmp->code = 0;
    icmp->un.echo.id = getpid();
    icmp->un.echo.sequence = 1;
    icmp->checksum = checksum(icmp, sizeof(packet));

    if (sendto(sock, packet, sizeof(packet), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Fail send package");
        close(sock);
        return 1;
    }

    if (getsockname(sock, (struct sockaddr *)&local_addr, &local_addr_len) == 0 ) {
        printf("Source IP: %s\n", inet_ntoa(local_addr.sin_addr));
    } else {
        perror("Fail get source IP");
    }

    printf("Destination IP: %s\n", inet_ntoa(addr.sin_addr));

    if (recvfrom(sock, response, sizeof(response), 0, (struct sockaddr *)&r_addr, &addr_len) > 0) {
        printf("Recived response: %s\n", inet_ntoa(r_addr.sin_addr));
    } else {
        perror("Fail obtain response");
    }

    close(sock);
    return 0;
}

int main() {
    const char target[16] = "192.168.0.10";
    int result;

    result = ping(target);

    printf("%s", result);

    return 0;
}
