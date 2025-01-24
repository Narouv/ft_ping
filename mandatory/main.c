/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rnauke <rnauke@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/15 00:55:57 by rnauke            #+#    #+#             */
/*   Updated: 2024/12/15 00:55:57 by rnauke           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ping.h"

void	resolve_target(const char *hostname, struct sockaddr_in *target)
{
	struct addrinfo		hints;
	struct addrinfo		*res;
	struct sockaddr_in	*addr;

	ft_bzero(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	if (getaddrinfo(hostname, NULL, &hints, &res) != 0)
	{
		printf("Error resolving hostname: %s\n", hostname);
		exit(1);
	}
	addr = (struct sockaddr_in *)res->ai_addr;
	ft_memcpy(target, addr, sizeof(struct sockaddr_in));
	freeaddrinfo(res);
}

unsigned short	calculate_checksum(void *b, int len)
{
	unsigned short	*buf;
	unsigned int	sum;

	buf = (unsigned short *)b;
	sum = 0;
	while (len > 1)
	{
		sum += *buf++;
		len -= 2;
	}
	if (len == 1)
		sum += *(unsigned char *)buf;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	return (~sum);
}

void	handle_interrupt(int signum, siginfo_t *info, void *context)
{
	// somehow close fds idk how :(
	long tmdev;

	tsum /= nreceived + nrepeats;
	tsum2 /= nreceived + nrepeats;
	tmdev = llsqrt(tsum2 - tsum * tsum);

	printf("rtt min/avg/max/mdev = %ld.%03ld/%lu.%03ld/%ld.%03ld/%ld.%03ld ms",
			(long)tmin/1000, (long)tmin%1000,
			(unsigned long)(tsum/1000), (long)(tsum%1000),
			(long)tmax/1000, (long)tmax%1000,
			(long)tmdev/1000, (long)tmdev%1000
			);
	exit(1);
	return ;
}

int	main(int argc, char **argv)
{
	t_ping	context;
	char	resolved[16];

	if (argc < 2)
		print_usage_error();
	ft_bzero(&context, sizeof(t_ping));
	context.hostname = parse_args(argv, &context);
	resolve_target(context.hostname, &context.target_address);
	if (0 > (context.sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)))
	{
		perror("socket");
		exit(1);
	}
	signal(SIGINT, handle_interrupt);
	prepare_packet(&context);
	resolved = malloc(16);
	// context.host_ip = resolved;
	inet_ntop(AF_INET, &context.target_address.sin_addr, resolved, 16);
	printf("PING %s (%s): %d data bytes\n", context.hostname, resolved, 56);
	initialize_ping(&context);
	close(context.sockfd);
	return (0);
}






#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>

#define MAX_PACKETS 1000

struct packet_stats {
    struct timeval send_time;
    struct timeval recv_time;
    int received; // 0 = not received, 1 = received
};

struct packet_stats packets[MAX_PACKETS];
int sent_packets = 0;
int received_packets = 0;

// Calculate the RTT in milliseconds
double calc_rtt(struct timeval *start, struct timeval *end) {
    double rtt = (end->tv_sec - start->tv_sec) * 1000.0; // sec to ms
    rtt += (end->tv_usec - start->tv_usec) / 1000.0;     // us to ms
    return rtt;
}

int main() {
    int sock;
    struct sockaddr_in dest_addr;
    char send_buf[64];
    struct icmphdr *icmp = (struct icmphdr *)send_buf;
    int ttl = 64;

    // Create raw socket
    sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Set TTL
    if (setsockopt(sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0) {
        perror("setsockopt failed");
        close(sock);
        return 1;
    }

    // Set up destination address
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr("8.8.8.8"); // Google DNS

    // Loop to send packets
    for (int i = 0; i < MAX_PACKETS; i++) {
        memset(send_buf, 0, sizeof(send_buf));

        // Create ICMP header
        icmp->type = 8;  // Echo request
        icmp->code = 0;
        icmp->checksum = 0;
        icmp->un.echo.sequence = htons(i);
        icmp->checksum = ~((0x0800 + i) & 0xFFFF); // Simplified checksum

        // Record send time
        gettimeofday(&packets[i].send_time, NULL);
        packets[i].received = 0;

        // Send packet
        if (sendto(sock, send_buf, sizeof(send_buf), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
            perror("sendto failed");
        } else {
            sent_packets++;
        }

        // Simulate waiting for a response (you should implement recvmsg for real data)
        usleep(50000); // Wait 50ms for demonstration

        // Simulate a reply (for demonstration purposes)
        gettimeofday(&packets[i].recv_time, NULL);
        packets[i].received = 1;
        received_packets++;
    }

    // Calculate final statistics
    double total_rtt = 0, min_rtt = 1e9, max_rtt = 0;
    for (int i = 0; i < sent_packets; i++) {
        if (packets[i].received) {
            double rtt = calc_rtt(&packets[i].send_time, &packets[i].recv_time);
            total_rtt += rtt;
            if (rtt < min_rtt) min_rtt = rtt;
            if (rtt > max_rtt) max_rtt = rtt;
        }
    }

    // Print statistics
    printf("Packets sent: %d, received: %d, lost: %d\n",
           sent_packets, received_packets, sent_packets - received_packets);
    if (received_packets > 0) {
        printf("RTT: min = %.2f ms, max = %.2f ms, avg = %.2f ms\n",
               min_rtt, max_rtt, total_rtt / received_packets);
    }

    close(sock);
    return 0;
}
