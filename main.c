/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rnauke <rnauke@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/14 07:22:23 by rnauke            #+#    #+#             */
/*   Updated: 2024/12/14 07:22:23 by rnauke           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ping.h"

volatile sig_atomic_t	g_exit = 0;

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

void	initialize_ping(t_ping *context)
{
	struct timeval start;

	while (!g_exit)
	{
		gettimeofday(&start, NULL);
		send_icmp_request(context);
		receive_icmp_reply(context, &start);
		context->sequence++;
		sleep(1);
	}
	t_statistics s = context->statistics;
	if (s.p_recv)
	{
		long tmdev;
		s.t_total /= s.p_recv;
		s.t_total2 /= s.p_recv;
		tmdev = llsqrt(s.t_total2 - s.t_total * s.t_total);
		int p_loss = 100 * (s.p_sent - s.p_recv) / s.p_sent;
		printf("--- %s ping statistics ---\n", context->hostname);
		printf("%d packets transmitted, %d packets recieved, %d%% packet loss\n", s.p_sent, s.p_recv, p_loss);
		printf("rtt min/avg/max/mdev = %ld.%03ld/%lu.%03ld/%ld.%03ld/%ld.%03ld ms\n",
				(long)s.t_min/1000, (long)s.t_min%1000,
				(unsigned long)(s.t_total/1000), (long)(s.t_total%1000),
				(long)s.t_max/1000, (long)s.t_max%1000,
				(long)tmdev/1000, (long)tmdev%1000
				);
	}
}

void	handle_interrupt()
{
	g_exit = 1;
	return ;
}

// setup context struct with default values or with whatever options were provided
void	init(t_ping *context)
{
	struct linger	linger;

	context->statistics.t_max = LLONG_MIN;
	context->statistics.t_min = LLONG_MAX;
	if (!(context->time_to_live))
		context->time_to_live = 63;
	// context->time_to_live = 1;
	if (0 > setsockopt(context->sockfd, IPPROTO_IP, IP_TTL, &context->time_to_live, sizeof(context->time_to_live)))
		perror("setsockopt ttl failed");
	// if (context->linger_for)
	// {
	// 	linger.l_onoff = 1;
	// 	linger.l_linger = context->linger_for;
	// 	setsockopt(context->sockfd, SOL_SOCKET, SO_LINGER, &linger, (socklen_t)sizeof(linger));
	// }
	// if (context->b_noroute)
	// 	setsockopt(context->sockfd, SOL_SOCKET, SO_DONTROUTE, &(context->b_noroute), (socklen_t)sizeof(int));
	return;
}

char	*parse_args(char **argv, t_ping *context)
{
	char	*option;

	argv++;
	while (*argv && **argv == '-')
	{
		option = *argv;
		if (option[1] == '-')
			handle_expanded(option, context, &argv);
		else
			handle_abbrev(option, context);
		argv++;
	}
	return (*argv);
}

int	main(int argc, char **argv)
{
	t_ping	context;
	char	resolved[16];
	char	packet[64];

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
	init(&context);
	signal(SIGINT, handle_interrupt);
	prepare_packet(&context, packet);
	inet_ntop(AF_INET, &context.target_address.sin_addr, resolved, 16);
	printf("PING %s (%s): %d data bytes", context.hostname, resolved, 56);
	if (context.b_verbose)
		printf(", %x = %d", getpid(), getpid());
	printf("\n");
	initialize_ping(&context);
	close(context.sockfd);
	
	return (0);
}
