/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   util.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rnauke <rnauke@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/14 09:02:46 by rnauke            #+#    #+#             */
/*   Updated: 2024/12/14 09:02:46 by rnauke           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ping.h"

long	llsqrt(long long a)
{
	long long prev = ~((long long)1 << 63);
	long long x = a;

	if (x > 0) {
		while (x < prev) {
			prev = x;
			x = (x + (a / x)) / 2;
		}
	}
	return (long)x;
}

void print_icmp_reply(const char *buffer, ssize_t length)
{
	if (length <= 0) {
		printf("No data received or error occurred.\n");
		return;
	}

	// Print the entire raw packet
	printf("\nRaw Packet Data (%ld bytes):\n", length);
	for (ssize_t i = 0; i < length; i++) {
		if (i % 16 == 0) printf("\n%04lx: ", i); // Print offset every 16 bytes
		printf("%02x ", (unsigned char)buffer[i]);
	}
	printf("\n\n");

	// Interpret the IP header
	struct iphdr *ip_header = (struct iphdr *)buffer;
	printf("--- IP Header ---\n");
	printf("Version: %d\n", ip_header->version);
	printf("Header Length: %d words -> %d bytes\n", ip_header->ihl, ip_header->ihl * 4);
	printf("Total Length: %d bytes\n", ntohs(ip_header->tot_len));
	printf("Time to Live: %d\n", ip_header->ttl);
	printf("Protocol: %d\n", ip_header->protocol);
	printf("Source IP: %s\n", inet_ntoa(*(struct in_addr *)&ip_header->saddr));
	printf("Destination IP: %s\n", inet_ntoa(*(struct in_addr *)&ip_header->daddr));

	// Calculate the start of the ICMP header
	size_t ip_header_length = ip_header->ihl * 4;
	if (length < ip_header_length + sizeof(struct icmphdr)) {
		printf("Packet is too small to contain ICMP header.\n");
		return;
	}

	struct icmphdr *icmp_header = (struct icmphdr *)(buffer + ip_header_length);

	printf("\n--- ICMP Header ---\n");
	printf("Type: %d\n", icmp_header->type);
	printf("Code: %d\n", icmp_header->code);
	printf("Checksum: 0x%04x\n", ntohs(icmp_header->checksum));
	printf("ID: %d\n", ntohs(icmp_header->un.echo.id));
	printf("Sequence: %d\n", ntohs(icmp_header->un.echo.sequence));

	// Calculate the start of the ICMP payload
	size_t icmp_header_length = sizeof(struct icmphdr);
	size_t payload_offset = ip_header_length + icmp_header_length;
	size_t payload_length = length - payload_offset;

	if (payload_length > 0) {
		printf("\n--- ICMP Payload (%ld bytes) ---\n", payload_length);
		for (size_t i = payload_offset; i < length; i++) {
			if ((i - payload_offset) % 16 == 0) printf("\n%04lx: ", i - payload_offset); // Print offset every 16 bytes
			printf("%02x ", (unsigned char)buffer[i]);
		}
		printf("\n");
	} else {
		printf("\nNo ICMP Payload.\n");
	}
}

void print_icmp_packet(const char *buffer, ssize_t length)
{
	if (length <= 0) {
		printf("No data received or error occurred.\n");
		return;
	}

	// Print the entire raw packet
	printf("\nRaw Packet Data (%ld bytes):\n", length);
	for (ssize_t i = 0; i < length; i++) {
		if (i % 16 == 0) printf("\n%04lx: ", i); // Print offset every 16 bytes
		printf("%02x ", (unsigned char)buffer[i]);
	}
	printf("\n\n");

	// Calculate the start of the ICMP header
	if (length < sizeof(struct icmphdr)) {
		printf("Packet is too small to contain ICMP header.\n");
		return;
	}

	struct icmphdr *icmp_header = (struct icmphdr *)(buffer);

	printf("\n--- ICMP Header ---\n");
	printf("Type: %d\n", icmp_header->type);
	printf("Code: %d\n", icmp_header->code);
	printf("Checksum: 0x%04x\n", ntohs(icmp_header->checksum));
	printf("ID: %d\n", ntohs(icmp_header->un.echo.id));
	printf("Sequence: %d\n", ntohs(icmp_header->un.echo.sequence));

	// Calculate the start of the ICMP payload
	size_t icmp_header_length = sizeof(struct icmphdr);
	size_t payload_offset = icmp_header_length;
	size_t payload_length = length - payload_offset;

	if (payload_length > 0) {
		printf("\n--- ICMP Payload (%ld bytes) ---\n", payload_length);
		for (size_t i = payload_offset; i < length; i++) {
			if ((i - payload_offset) % 16 == 0) printf("\n%04lx: ", i - payload_offset); // Print offset every 16 bytes
			printf("%02x ", (unsigned char)buffer[i]);
		}
		printf("\n");
	} else {
		printf("\nNo ICMP Payload.\n");
	}
}
