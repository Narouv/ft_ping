/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rnauke <rnauke@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/14 08:35:52 by rnauke            #+#    #+#             */
/*   Updated: 2024/12/14 08:35:52 by rnauke           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ping.h"

void	collect_statistics(t_ping *context, float rtt)
{
	t_statistics stats = context->statistics;

	if (rtt < context->statistics.t_min)
		context->statistics.t_min = rtt;
	if (rtt > context->statistics.t_max)
		context->statistics.t_max = rtt;
	context->statistics.t_total += (long long)rtt;
	context->statistics.t_total2 += (long long)rtt * (long long)rtt;
}

void	prepare_packet(t_ping *context, char *packet)
{
	unsigned short	checksum;

	context->icmp = (struct icmphdr *)packet;
	ft_bzero(packet, sizeof(packet));
	context->icmp->type = ICMP_ECHO;
	context->icmp->code = 0;
	context->icmp->checksum = 0;
	context->icmp->un.echo.id = getpid();
	context->icmp->un.echo.sequence = htons(context->sequence);
	ft_memset(packet + sizeof(struct icmphdr), 'B', 56);
	context->packet = packet;
	context->payload = packet + sizeof(struct icmphdr);
}

void	send_icmp_request(t_ping *context)
{
	struct sockaddr	*target;
	unsigned short	new_checksum;
	ssize_t			bytes_sent;

	target = (struct sockaddr *)&(context->target_address);
	context->icmp->un.echo.sequence = htons(context->sequence);
	ft_memset(context->packet + sizeof(struct icmphdr), 'B', 56);
	new_checksum = calculate_checksum(context->icmp, 64);
	context->icmp->checksum = new_checksum;
	bytes_sent = sendto(context->sockfd, context->packet, 64, 0, target, sizeof(*target));
	if (bytes_sent <= 0)
		if (context->b_verbose)
			perror("sendto");
	context->statistics.p_sent++;
}

void	print_icmp_error(char type, char code)
{
	switch (type)
	{
	case ICMP_DEST_UNREACH:
		printf("fortnite\n");
		break;
	case ICMP_ECHO:
		printf("echo request\n");
	default:
		printf("reply type: %d, reply code: %d\n", type, code);
		break;
	}
}

void	construct_output(struct msghdr msg, long b_recv, struct timeval *s, t_ping *context)
{
	struct icmphdr	*icmp_hdr;
	struct iphdr	*ip_hdr;
	struct timeval	end;
	long long		rtt;
	char			packet_origin[16];
	char			*buffer;

	ft_bzero(packet_origin, 16);
	buffer = msg.msg_iov->iov_base;
	ip_hdr = (struct iphdr *)buffer;
	icmp_hdr = (struct icmphdr *)(buffer + sizeof(struct iphdr));
	inet_ntop(AF_INET, &((struct sockaddr_in *)msg.msg_name)->sin_addr, packet_origin, 16);
	b_recv -= sizeof(struct iphdr);
	if (icmp_hdr->type == ICMP_ECHOREPLY && icmp_hdr->un.echo.id == getpid())
	{
		gettimeofday(&end, NULL);
		rtt = (end.tv_sec - s->tv_sec) * 10000000 + (end.tv_usec - s->tv_usec);
		context->statistics.p_recv++;
		collect_statistics(context, rtt);
		printf("%ld bytes from %s: icmp_seq=%d ttl=%d time=%ld.%03ld ms\n",
			b_recv, packet_origin, ntohs(icmp_hdr->un.echo.sequence), ip_hdr->ttl, (long)rtt/1000, (long)rtt%1000);
	}
	else
		print_icmp_error(icmp_hdr->type, icmp_hdr->code);
}

void	receive_icmp_reply(t_ping *context, struct timeval *start)
{
	ssize_t				bytes_recv;
	char				buffer[84];
	struct msghdr		msg;
	struct sockaddr_in	sender_addr;
	struct iovec		iov;

	ft_bzero(&buffer, sizeof(buffer));
	ft_bzero(&msg, sizeof(msg));
	iov.iov_base = buffer;
	iov.iov_len = sizeof(buffer);
	msg.msg_name = &sender_addr;
	msg.msg_namelen = (socklen_t) sizeof(sender_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	bytes_recv = recvmsg(context->sockfd, &msg, 0);
	if (bytes_recv < 0)
	{
		if (context->b_verbose)
			perror("recvmsg");
		return ;
	}
	construct_output(msg, bytes_recv, start, context);
}
