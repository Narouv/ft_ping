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

void	prepare_packet(t_ping *context)
{
	char			*packet;
	unsigned short	checksum;

	packet = ft_calloc(64, sizeof(char));
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
	struct sockaddr	*t;
	unsigned short	new_checksum;
	ssize_t			b_s;

	t = (struct sockaddr *)&(context->target_address);
	context->icmp->un.echo.sequence = htons(context->sequence);
	ft_memset(context->packet + sizeof(struct icmphdr), 'B', 56);
	new_checksum = calculate_checksum(context->icmp, 64);
	context->icmp->checksum = new_checksum;
	b_s = sendto(context->sockfd, context->packet, 64, 0, t, sizeof(*t));
	if (b_s <= 0)
		if (context->b_verbose)
			perror("sendto");
}

void	construct_output(struct msghdr msg, long b_recv, struct timeval *s)
{
	struct icmphdr	*icmp_hdr;
	struct timeval	e;
	float			rtt;
	char			po[16];
	char			*buffer;

	ft_bzero(po, 16);
	buffer = msg.msg_iov->iov_base;
	icmp_hdr = (struct icmphdr *)(buffer + sizeof(struct iphdr));
	inet_ntop(AF_INET, &((struct sockaddr_in *)msg.msg_name)->sin_addr, po, 16);
	b_recv -= sizeof(struct iphdr);
	if (icmp_hdr->type == ICMP_ECHOREPLY && icmp_hdr->un.echo.id == getpid())
	{
		gettimeofday(&e, NULL);
		rtt = (e.tv_sec - s->tv_sec) * 1000 + (e.tv_usec - s->tv_usec) / 1000;
		printf("%ld bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
			b_recv, po, ntohs(icmp_hdr->un.echo.sequence), buffer[8], rtt);
	}
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
	construct_output(msg, bytes_recv, start);
}

void	initialize_ping(t_ping *context)
{
	struct timeval start;

	while (1)
	{
		gettimeofday(&start, NULL);
		send_icmp_request(context);
		receive_icmp_reply(context, &start);
		context->sequence++;
		sleep(1);
	}
}
