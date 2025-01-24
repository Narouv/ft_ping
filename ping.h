#ifndef FT_PING_H
# define FT_PING_H

# include <netdb.h>
# include <unistd.h>
# include <sys/time.h>
# include <arpa/inet.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/ip.h>
# include <netinet/in.h>
# include <netinet/ip_icmp.h>

# include "./libft/libft.h"
# include <stdio.h>
# include <signal.h>
# include <string.h>
# include <stdlib.h>
# include <limits.h>

typedef struct s_statistics
{
	int	p_sent;
	int	p_recv;
	long long	t_total;
	long long	t_total2;
	long long	t_min;
	long long	t_max;
}	t_statistics;

typedef struct	s_ping
{
	char				*packet;
	struct  icmphdr		*icmp;
	char				*payload;
	struct sockaddr_in	target_address;
	t_statistics		statistics;
	char				*host_ip;
	char				*hostname;
	int					sockfd;
	int					b_verbose;
	int					b_quiet;
	int					b_noroute;
	int					time_to_live;
	int					linger_for;
	int					sequence;
}	t_ping;

// parse.c
char	*parse_args(char **argv, t_ping *context);
void    print_usage_error();
void	handle_expanded(char *option, t_ping *context, char ***argv);
void	handle_abbrev(char *option, t_ping *context);

// main.c
unsigned short	calculate_checksum(void *b, int len);

// ping.c
void	prepare_packet(t_ping *context, char *packet);
void	send_icmp_request(t_ping *context);
void	receive_icmp_reply(t_ping *context, struct timeval *start);

// util.c
long	llsqrt(long long a);
void	print_icmp_reply(const char *buffer, ssize_t length); // kuss an chatgpt
void	print_icmp_packet(const char *buffer, ssize_t length);


#endif