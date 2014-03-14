#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

int main(int argc, char **argv)
{
	int opt;
	unsigned short port;
	struct in_addr addr;
	struct sockaddr_in socket_addr;
	int verbose;
	int listening_socket;
	int new_connection;
	int sender;
	int reciever;
	char identification[2];
	char message_buffer[256];
	unsigned char *data_buffer;
	long data_buffer_size;
	int bytes_read;
	int bytes_written;
	int end;
	long int input;

	verbose = 0;
	port = 3033;
	addr.s_addr = INADDR_ANY;
	sender = -1;
	reciever = -1;
	data_buffer_size = 1048576UL; 

	while ((opt = getopt(argc, argv, "va:p:m:")) != -1)
	{
		switch(opt)
		{
		case 'v': /* verbose */
			verbose = 1;
		break;

		case 'a': /* address */
			if (inet_aton(optarg, &addr) == 0)
			{
				fprintf(stderr, "%s: invalid address -- '%s'\n", argv[0], optarg);
				return EXIT_FAILURE;
			}
		break;

		case 'p': /* port */
			errno = 0;
			input = strtol(optarg, NULL, 10);

			if (errno != 0)
			{
				if (errno == EINVAL)
				{
					fprintf(stderr, "%s: unable to convert -- '%s'\n", argv[0], optarg);
				}
				else if (errno == ERANGE)
				{
					fprintf(stderr, "%s: out of range -- '%s'\n", argv[0], optarg);
				}
				else
				{
					fprintf(stderr, "%s: unknown problem\n", argv[0]);
				}
				
				return EXIT_FAILURE;
			}

			if (input < 0)
			{
				fprintf(stderr, "%s: port number negative -- '%s'\n", argv[0], optarg);
				return EXIT_FAILURE;
			}

			if (input > 65535)
			{
				fprintf(stderr, "%s: port number too high -- '%s'\n", argv[0], optarg);
				return EXIT_FAILURE;
			}

			port = (unsigned short)input;
		break;

		case '?':
			fprintf(stderr, "%s: invalid option -- '%c'\n", argv[0], optopt);
			fprintf(stderr, "Usage: %s [-v] [-a addr] [-p port]\n", argv[0]);
		return EXIT_FAILURE;

		default:
			return EXIT_FAILURE;
		}
	}

	if ((listening_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "%s: error opening listening socket\n", argv[0]);
		return EXIT_FAILURE;
	}

	if (verbose == 1)
	{
		fprintf(stdout, "Creating the listening socket\n");
	}

	memset(&socket_addr, 0, sizeof(socket_addr));
	socket_addr.sin_family = AF_INET;
	socket_addr.sin_addr.s_addr = addr.s_addr;
	socket_addr.sin_port = htons(port);

	if (bind(listening_socket, (struct sockaddr *) &socket_addr, 
	    sizeof(socket_addr)) < 0)
	{
		fprintf(stderr, "%s: error binding socket\n", argv[0]);
		return EXIT_FAILURE;
	}

	if (verbose == 1)
	{
		fprintf(stdout, "Binding to the address\n");
	}

	if (listen(listening_socket, 10) < 0)
	{
		/* TODO handle errno */
		fprintf(stderr, "%s: error listening on socket\n", argv[0]);
		return EXIT_FAILURE;
	}

	if (verbose == 1)
	{
		fprintf(stdout, "Listening on the socket\n");
	}

	fprintf(stdout, "Waiting for new connections\n");

	while (1)
	{
		new_connection = accept(listening_socket, NULL, NULL);
		fprintf(stdout, "New connection!\n");

		bytes_read = read(new_connection, identification, 2);
		if (bytes_read < 2)
		{
			fprintf(stdout, "Discarding client: bad identification\n");
			continue;
		}

		if (strncmp(identification, "S", 2) == 0)
		{
			if (sender > 0)
			{
				fprintf(stdout, "Discarding client: we already have a sender\n");
				continue;
			}
			sender = new_connection;
		}
		else if (strncmp(identification, "R", 2) == 0)
		{
			if (reciever > 0)
			{
				fprintf(stdout, "Discarding client: we already have a reciever\n");
				continue;
			}
			reciever = new_connection;
		}
		else
		{
			fprintf(stdout, "Discarding client: bad identification\n");
			continue;
		}

		if (sender > 0 && reciever > 0)
		{
			break;
		}
	}

	memset(message_buffer, 0, 256);
	bytes_read = read(reciever, message_buffer, 255);
	if (bytes_read < 0)
	{
		fprintf(stderr, "%s: unable to determine block size\n", argv[0]);
		return EXIT_FAILURE;
	}

	/* TODO add support for endings like M or K or G */
	errno = 0;
	data_buffer_size = strtol(message_buffer, NULL, 10);
	if (errno != 0)
	{
		if (errno == EINVAL)
		{
			fprintf(stderr, "%s: unable to convert -- '%s'\n", argv[0], optarg);
		}
		else if (errno == ERANGE)
		{
			fprintf(stderr, "%s: out of range -- '%s'\n", argv[0], optarg);
		}
		else
		{
			fprintf(stderr, "%s: unknown problem\n", argv[0]);
		}

		return EXIT_FAILURE;
	}

	data_buffer = (unsigned char*)malloc(data_buffer_size);

	bytes_written = write(sender, message_buffer, 255);
	if (bytes_written < 255)
	{
		fprintf(stderr, "%s: unable to send all data -- %d/255\n", argv[0],
		        bytes_written);
		return EXIT_FAILURE;
	}

	end = 0;
	while (1)
	{
		bytes_read = read(sender, data_buffer, data_buffer_size);
		if (bytes_read < data_buffer_size)
		{
			end = 1;
		}

		bytes_written = write(reciever, data_buffer, data_buffer_size);
		if (bytes_read != bytes_written)
		{
			fprintf(stderr, "%s: %d read, %d written\n", argv[0], bytes_read, 
			        bytes_written);
			return EXIT_FAILURE;
		}

		if (end == 1)
		{
			break;
		}
	}

	return EXIT_SUCCESS;
}

