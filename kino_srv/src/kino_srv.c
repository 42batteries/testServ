/*
 ============================================================================
 Name        : kino1.c
 Author      : nh
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World Server in C, Ansi-style
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>

//net
#define PORT 					4444
#define IP_ADRESS_SERVER 		"127.0.0.1"
#define SOCKET_QUEUE_LENGHT		10
//io
#define INPUT_NUMBER_LEGHT 		16
#define INPUT_BUFFER_LEGHT 		1024
#define OUTPUT_BUFFER_LENGHT 	64
//parsing
#define OPERATION_SIMBOL_LENGHT 1
#define INPUT_VALUE_MAX 		999
#define INPUT_VALUE_MIN 		-999
#define SEPARATOR  " "
#define OPERATIONS '+','-','/',':','*','^','M','m','h','d','r'
//check and validation
#define VALIDATION_OK 										  0
#define VALIDATION_ERROR_FIRST_NUMBER_LEGHT                   1
#define VALIDATION_ERROR_FIRST_NUMBER_RANGE                   2
#define VALIDATION_ERROR_FIRST_NUMBER_INCORRECT_OR_ZERRO      3
#define VALIDATION_ERROR_SECOND_NUMBER_LEGHT                  4
#define VALIDATION_ERROR_SECOND_NUMBER_RANGE                  5
#define VALIDATION_ERROR_SECOND_NUMBER_INCORRECT_OR_ZERRO     6
#define VALIDATION_ERROR_UNKNOWN_OPERATION                    8
#define VALIDATION_ERROR_INPUT_DATA                           10

typedef struct {
	double fn;
	double sn;
	char * operation;
	char * buff;
	char * string_first_number;
	char * string_second_number;
	char * operation_string;
} in_data;

static int define_and_fill_numbers_in_input_struct(in_data * indata);
static int parse_and_check_validity(in_data * indata);
static double select_operation_and_process_calculation(in_data * indata);
static void set_error_msg(char * buff, int error);
static char * calculate_expression(char * buff_);

static int define_and_fill_numbers_in_input_struct(in_data * indata) {
	if (strlen(indata->string_first_number) > INPUT_NUMBER_LEGHT)
		return VALIDATION_ERROR_FIRST_NUMBER_LEGHT;

	if (strlen(indata->string_second_number) > INPUT_NUMBER_LEGHT)
		return VALIDATION_ERROR_SECOND_NUMBER_LEGHT;

	indata->fn = atof(indata->string_first_number);
	indata->sn = atof(indata->string_second_number);

	if (indata->fn == 0) {
		return VALIDATION_ERROR_FIRST_NUMBER_INCORRECT_OR_ZERRO;
	}
	if (indata->sn == 0)
		return VALIDATION_ERROR_SECOND_NUMBER_INCORRECT_OR_ZERRO;

	if (indata->fn < INPUT_VALUE_MIN || indata->fn > INPUT_VALUE_MAX)
		return VALIDATION_ERROR_FIRST_NUMBER_RANGE;

	if (indata->sn < INPUT_VALUE_MIN || indata->sn > INPUT_VALUE_MAX)
		return VALIDATION_ERROR_SECOND_NUMBER_RANGE;

	const char operations[] = { OPERATIONS };
	for (int n = 0; n < sizeof(operations); n++) {
		if (!strncmp(operations + n, indata->operation,
				OPERATION_SIMBOL_LENGHT)) {
			return VALIDATION_OK;
		}
	}
	return VALIDATION_ERROR_UNKNOWN_OPERATION;
}

static int parse_and_check_validity(in_data * indata) {
	const char separator[1] = SEPARATOR;

	indata->string_first_number = strtok(indata->buff, separator);

	if (indata->string_first_number != NULL) {
		indata->operation = strtok(NULL, separator);
	} else {
		return VALIDATION_ERROR_INPUT_DATA;
	}
	if (indata->operation != NULL) {
		indata->string_second_number = strtok(NULL, separator);
	} else {
		return VALIDATION_ERROR_INPUT_DATA;
	}
	if (indata->string_second_number != NULL) {
		int err = define_and_fill_numbers_in_input_struct(indata);
		if (err == VALIDATION_OK) {
			return VALIDATION_OK;
		} else {
			return err;
		}
	} else {
		return VALIDATION_ERROR_INPUT_DATA;
	}

	return VALIDATION_OK;
}

static double select_operation_and_process_calculation(in_data * indata) {
	if (!strcmp(indata->operation, "+")) {
		return indata->fn + indata->sn;
	} else if (!strcmp(indata->operation, "-")) {
		return indata->fn - indata->sn;
	} else if (!strcmp(indata->operation, ":")
			|| !strcmp(indata->operation, "/")) {
		return indata->fn / indata->sn;
	} else if (!strcmp(indata->operation, "*")) {
		return indata->fn * indata->sn;
	} else if (!strcmp(indata->operation, "^")) {
		return pow(indata->fn, indata->sn);
	} else if (!strcmp(indata->operation, "h"))
	//hypotenuse, sqrt (x² + y²)
			{
		return hypot(indata->fn, indata->sn);
	} else if (!strcmp(indata->operation, "d"))
	//select_operation_and_process_calculation of the positive difference between x and y, fmax (x − y, 0)
			{
		return fdim(indata->fn, indata->sn);
	} else if (!strcmp(indata->operation, "M"))
	//the largest value among x and y
			{
		return fmax(indata->fn, indata->sn);
	} else if (!strcmp(indata->operation, "m"))
	//smallest value among x and y
			{
		return fmin(indata->fn, indata->sn);
	} else if (!strcmp(indata->operation, "r"))
	//calculates the remainder of the division according to IEC 60559
			{
		return remainder(indata->fn, indata->sn);
	} else {
		return 0;
	}
}

static void set_error_msg(char * buff, int error) {
	switch (error) {
	case VALIDATION_ERROR_FIRST_NUMBER_LEGHT:
		strcpy(buff, "Error: Too long first number.\n");
		break;

	case VALIDATION_ERROR_FIRST_NUMBER_RANGE:
		strcpy(buff, "Error: The first number is out of range.\n");
		break;

	case VALIDATION_ERROR_FIRST_NUMBER_INCORRECT_OR_ZERRO:
		strcpy(buff,
				"Error: Wrong first number entered or non numeric data entered.\n");
		break;

	case VALIDATION_ERROR_SECOND_NUMBER_LEGHT:
		strcpy(buff, "Error: Too long second number.\n");
		break;

	case VALIDATION_ERROR_SECOND_NUMBER_RANGE:
		strcpy(buff, "Error: The second number is out of range.\n");
		break;

	case VALIDATION_ERROR_SECOND_NUMBER_INCORRECT_OR_ZERRO:
		strcpy(buff,
				"Error: Wrong second number entered or non numeric data entered.\n");
		break;
	case VALIDATION_ERROR_UNKNOWN_OPERATION:
		strcpy(buff, "Error:  Invalid operation on numbers.\n");
		break;

	case VALIDATION_ERROR_INPUT_DATA:
		strcpy(buff,
				"Error: Error input data. Numbers and (or) operation not recognized.\n");
		break;

	}

}
static char * calculate_expression(char * buff_) {

	in_data input_data;
	input_data.buff = buff_;

	int err = parse_and_check_validity(&input_data);

	if (err == VALIDATION_OK) {
		//do select_operation_and_process_calculation
		double rez_dbl = select_operation_and_process_calculation(&input_data);
		int rez_int = (int) rez_dbl;
		if (rez_dbl == (double) rez_int) {
			snprintf(buff_, OUTPUT_BUFFER_LENGHT, "%d\n", rez_int);
		} else {
			snprintf(buff_, OUTPUT_BUFFER_LENGHT, "%0.2f\n", rez_dbl);
		}
	} else {
		set_error_msg(buff_, err);
	}

	return buff_;
}

static int initserver(int type, const struct sockaddr *addr, socklen_t len,
		int qlen) {
	printf("[+]Server Socket is created.\n");
	int fd;

	if ((fd = socket(addr->sa_family, type, 0)) < 0) {
		printf("[-]Error in connection.\n");
		return 0;
	}
	if (bind(fd, addr, len) < 0) {
		printf("[-]Error in binding.\n");
		close(fd);
		return -1;
	}

	if (type == SOCK_STREAM || type == SOCK_SEQPACKET) {
		if (listen(fd, qlen) == 0) {
			printf("[+]Listening....\n");
		} else {
			printf("[-]Error in binding.\n");
			close(fd);
			return -1;
		}
	}

	return fd;
}
static int sendall(int s, char *buf, int len, int flags) {
	int total = 0;
	int n;

	while (total < len) {
		n = send(s, buf + total, len - total, flags);
		if (n == -1) {
			break;
		}
		total += n;

	}
	if (n == -1) {
		printf("[-]Error:Data was not send. Connection will be closed\n");
		close(s);
	}
	return (n == -1 ? -1 : total);
}
static int recvall(int socket, char *buffer, int numBytes, int flags) {
	int ret; // Return value for 'recv'
	int receivedBytes; // Total number of bytes received

	// Retrieve the given number of bytes.
	receivedBytes = 0;
	while ((receivedBytes < numBytes) && ret != -1
			&& strncmp(buffer + receivedBytes - sizeof("\r\n"), "\r\n",
					sizeof("\r\n"))) {
		printf("buff: %s \n", buffer + 1 + receivedBytes - sizeof("\r\n"));
		ret = recv(socket, buffer + receivedBytes, numBytes - receivedBytes, 0);

		if (ret == -1) {
			close(socket);
			printf("[-]Error while receive data. Connection will be closed\n");
			exit(1);
		} else if (ret == 0) {

			receivedBytes += ret;
			break;
		} else {
			receivedBytes += ret;
		}
	}

	buffer[receivedBytes] = '\0';
	return (receivedBytes < 1 ? -1 : receivedBytes);

}
int main() {

	int socket_fd;
	struct sockaddr_in seraddr;
	char buff[INPUT_BUFFER_LEGHT];

	seraddr.sin_family = AF_INET;
	seraddr.sin_addr.s_addr = inet_addr(IP_ADRESS_SERVER);
	seraddr.sin_port = htons(PORT);

	if ((socket_fd = initserver(SOCK_STREAM, (struct sockaddr *) &seraddr,
			sizeof(seraddr), SOCKET_QUEUE_LENGHT)) == -1) {
		printf("[-]Error initialization server\n");
		return 0;
	}

	while (1) {

		int connect_fd;
		struct sockaddr_in cli_addr;
		socklen_t clilen = sizeof(cli_addr);

		if ((connect_fd = accept(socket_fd, (struct sockaddr *) &cli_addr,
				&clilen)) == -1) {
			printf("[-]Error accept client\n");
			continue;
		}
		pid_t childpid;
		if ((childpid = fork()) == 0) {
			while (1) {
				if (recvall(connect_fd, buff, INPUT_BUFFER_LEGHT, 0) == -1) {
					printf(
							"[-]Error:Data was not received. Connection will be closed\n");
					close(connect_fd);
					return 0;
				}

				if (!strcmp(buff, "exit\r\n")) {
					strcpy(buff, "Connection closed\n");
					if (sendall(connect_fd, buff, strlen(buff), 0) == -1)
						return 0;

					printf("Disconnected from %s:%d\n",
							inet_ntoa(cli_addr.sin_addr),
							ntohs(cli_addr.sin_port));
					close(connect_fd);
					return 0;
				} else {
					printf("Client: %s\n", buff);
					calculate_expression(buff);
					if (sendall(connect_fd, buff, strlen(buff), 0) == -1)
						return 0;

					memset(buff, 0, sizeof(buff));
				}
			}
		}

	}

	return 0;
}
