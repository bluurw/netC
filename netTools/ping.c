#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
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

int main() {
    int sock;
    struct sockaddr_in addr;
    char packet[64];
    char target[12];

    strcpy(target, "8.8.8.8");

    // Socket cru
    sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0) {
        perror("Fail Create Socket");
        return 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(target);

    // Datagrama 
    struct icmphdr *icmp = (struct icmphdr *)packet;
    memset(packet, 0, sizeof(packet));
    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->un.echo.id = getpid();
    icmp->un.echo.sequence = 1;
    icmp->checksum = checksum(icmp, sizeof(packet));

    if (sendto(sock, packet, sizeof(packet), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Erro ao enviar pacote");
        close(sock);
        return 1;
    }

    printf("Send ping %s \n", target);

    char response[1024];
    if (recv(sock, response, sizeof(response), 0) > 0) {
        printf("Recived response \n");
    } else {
        perror("Fail response \n");
    }

    close(sock);
    return 0;
}