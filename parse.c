/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rnauke <rnauke@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/14 07:22:12 by rnauke            #+#    #+#             */
/*   Updated: 2024/12/14 07:22:12 by rnauke           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ping.h"

void	print_help_exit(void)
{
	printf("Usage: ping [OPTION...] HOST ...\n");
	printf("Send ICMP ECHO_REQUEST packets to network hosts.\n");
	printf("\n Options valid for all request types:\n");
	printf("\n");
	printf("      --usage\t\tgive a short usage message\n");
	printf("  -?, --help\t\tgive this help list\n");
	exit(0);
}

void	print_usage_error(void)
{
	printf("\nTry 'ft_ping --help' or 'ft_ping --usage' for more info.\n");
	exit(1);
}

void	handle_abbrev(char *option, t_ping *context)
{
	size_t	i;

	i = 1;
	while (i < ft_strlen(option))
	{
		if (!ft_strncmp(&option[i], "?", 1))
			print_help_exit();
		else if (!ft_strncmp(&option[i], "v", 1))
		{
			printf("verbose = True\n");
			context->b_verbose = 1;
		}
		else
		{
			printf("invalid option %s\n", option);
			print_usage_error();
		}
		i++;
	}
}

// The exit(1) is only here because the program would not
// compile otherwise (no return in non-void function)
// in print_usage_error the program exits regardless
char	*get_next_argv(char *option, char ***argv)
{
	(*argv)++;
	if (**argv)
		return (**argv);
	else
	{
		printf("invalid input after %s\n", option);
		print_usage_error();
		exit(1);
	}
}

void	handle_expanded(char *option, t_ping *context, char ***argv)
{
	if (!ft_strncmp(option, "--usage", 6))
		printf("print a usage message here after finishing project\n");
	else if (!ft_strncmp(option, "--help", 5))
		print_help_exit();
	else if (!ft_strncmp(option, "--verbose", 9))
		context->b_verbose = 1;
	else
	{
		printf("invalid option %s\n", option);
		print_usage_error();
	}
}
