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
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>

//net
#define PORT 					(4444)
#define IP_ADRESS_SERVER 		"127.0.0.1"
#define SOCKET_QUEUE_LENGHT		(10)
//io
#define INPUT_NUMBER_LENGHT		(16)
#define INPUT_BUFFER_LENGHT 	(1024)
#define OUTPUT_BUFFER_LENGHT 	(64)
//parsing
#define OPERATION_SIMBOL_LENGHT (1)
#define INPUT_VALUE_MAX 		(999)
#define INPUT_VALUE_MIN 		(-999)
#define SEPARATOR  " "
#define OPERATIONS '+','-','/',':','*','^','M','m','h','d','r'
#define END_SIMBOL_CODE			(0x0a)
//check and validation
#define VALIDATION_OK 										  (0)
#define VALIDATION_ERROR_FIRST_NUMBER_LENGHT                  (1)
#define VALIDATION_ERROR_FIRST_NUMBER_RANGE                   (2)
#define VALIDATION_ERROR_FIRST_NUMBER_INCORRECT_OR_ZERO	      (3)
#define VALIDATION_ERROR_SECOND_NUMBER_LENGHT                 (4)
#define VALIDATION_ERROR_SECOND_NUMBER_RANGE                  (5)
#define VALIDATION_ERROR_SECOND_NUMBER_INCORRECT_OR_ZERO  	  (6)
#define VALIDATION_ERROR_UNKNOWN_OPERATION                    (8)
#define VALIDATION_ERROR_INPUT_DATA                           (10)

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
static int init_server(int type, const struct sockaddr *address_server,
		socklen_t address_lenght, int queue_lenght);
static int send_all(int connect_fd, char *buffer, int numBytes, int flags);
static int recv_all(int connect_fd, char *buffer, int numBytes, int flags);

static int define_and_fill_numbers_in_input_struct(in_data * indata) {
	if (strlen(indata->string_first_number) > INPUT_NUMBER_LENGHT)
		return VALIDATION_ERROR_FIRST_NUMBER_LENGHT;

	if (strlen(indata->string_second_number) > INPUT_NUMBER_LENGHT)
		return VALIDATION_ERROR_SECOND_NUMBER_LENGHT;

	indata->fn = atof(indata->string_first_number);
	indata->sn = atof(indata->string_second_number);

	if (indata->fn == 0) {
		return VALIDATION_ERROR_FIRST_NUMBER_INCORRECT_OR_ZERO;
	}
	if (indata->sn == 0)
		return VALIDATION_ERROR_SECOND_NUMBER_INCORRECT_OR_ZERO;

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
	case VALIDATION_ERROR_FIRST_NUMBER_LENGHT:
		strcpy(buff, "Error: Too long first number.\n");
		break;

	case VALIDATION_ERROR_FIRST_NUMBER_RANGE:
		strcpy(buff, "Error: The first number is out of range.\n");
		break;

	case VALIDATION_ERROR_FIRST_NUMBER_INCORRECT_OR_ZERO:
		strcpy(buff,
				"Error: Wrong first number entered or non numeric data entered.\n");
		break;

	case VALIDATION_ERROR_SECOND_NUMBER_LENGHT:
		strcpy(buff, "Error: Too long second number.\n");
		break;

	case VALIDATION_ERROR_SECOND_NUMBER_RANGE:
		strcpy(buff, "Error: The second number is out of range.\n");
		break;

	case VALIDATION_ERROR_SECOND_NUMBER_INCORRECT_OR_ZERO:
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

static int init_server(int type, const struct sockaddr *address_server,
		socklen_t address_lenght, int queue_lenght) {
	printf("[+]Server Socket is created.\n");
	int fd;

	if ((fd = socket(address_server->sa_family, type, 0)) < 0) {
		printf("[-]Error in connection.\n");
		return 0;
	}
	if (bind(fd, address_server, address_lenght) < 0) {
		printf("[-]Error in binding.\n");
		close(fd);
		return -1;
	}

	if (type == SOCK_STREAM || type == SOCK_SEQPACKET) {
		if (listen(fd, queue_lenght) == 0) {
			printf("[+]Listening....\n");
		} else {
			printf("[-]Error in binding.\n");
			close(fd);
			return -1;
		}
	}

	return fd;
}
static int send_all(int connect_fd, char *buffer, int numBytes, int flags) {
	int counter_total_bytes = 0;
	int counter_bytes_sended_per_iter;

	while (counter_total_bytes < numBytes) {
		counter_bytes_sended_per_iter = send(connect_fd,
				buffer + counter_total_bytes, numBytes - counter_total_bytes,
				flags);
		if (counter_bytes_sended_per_iter == -1) {
			break;
		}
		counter_total_bytes += counter_bytes_sended_per_iter;

	}
	if (counter_bytes_sended_per_iter == -1) {
		printf("[-]Error:Data was not send. Connection will be closed\n");
		close(connect_fd);
	}
	return (counter_bytes_sended_per_iter == -1 ? -1 : counter_total_bytes);
}

static int recv_all(int connect_fd, char *buffer, int numBytes, int flags) {
	int counter_bytes_recived_per_iter;
	int counter_total_bytes;

	counter_total_bytes = 0;
	while ((counter_total_bytes < numBytes)
			&& counter_bytes_recived_per_iter != -1
			&& !strrchr(buffer, END_SIMBOL_CODE)) {
		counter_bytes_recived_per_iter = recv(connect_fd,
				buffer + counter_total_bytes, numBytes - counter_total_bytes,
				0);

		if (counter_total_bytes == -1) {
			close(connect_fd);
			printf("[-]Error while receive data. Connection will be closed\n");
			exit(1);
		} else if (counter_bytes_recived_per_iter == 0) {

			counter_total_bytes += counter_bytes_recived_per_iter;
			break;
		} else {
			counter_total_bytes += counter_bytes_recived_per_iter;
		}
	}

	buffer[counter_total_bytes] = '\0';
	return (counter_total_bytes < 1 ? -1 : counter_total_bytes);

}
int main() {

	int socket_fd;
	struct sockaddr_in seraddr;
	char buffer[INPUT_BUFFER_LENGHT];

	seraddr.sin_family = AF_INET;
	seraddr.sin_addr.s_addr = inet_addr(IP_ADRESS_SERVER);
	seraddr.sin_port = htons(PORT);

	if ((socket_fd = init_server(SOCK_STREAM, (struct sockaddr *) &seraddr,
			sizeof(seraddr), SOCKET_QUEUE_LENGHT)) == -1) {
		printf("[-]Error initialization server\n");
		return 0;
	}

	while (1) {

		int connect_fd;
		struct sockaddr_in client_socket_address;
		socklen_t client_socket_lenght = sizeof(client_socket_address);

		if ((connect_fd = accept(socket_fd,
				(struct sockaddr *) &client_socket_address,
				&client_socket_lenght)) == -1) {
			printf("[-]Error accept client\n");
			continue;
		}
		pid_t child_pid;
		signal(SIGCHLD, SIG_IGN);
		if ((child_pid = fork()) == 0) {
			close(socket_fd);
			while (1) {
				if (recv_all(connect_fd, buffer, INPUT_BUFFER_LENGHT, 0) == -1) {
					printf(
							"[-]Error:Data was not received. Connection will be closed\n");
					close(connect_fd);
					return (0);
				}

				if (!strcmp(buffer, "exit\r\n")) {

					printf("Disconnected from %s:%d\n",
							inet_ntoa(client_socket_address.sin_addr),
							ntohs(client_socket_address.sin_port));
					close(connect_fd);
					return (0);
				} else {
					printf("Client: %s\n", buffer);
					calculate_expression(buffer);
					if (send_all(connect_fd, buffer, strlen(buffer), 0) == -1)
						return (0);

					memset(buffer, 0, sizeof(buffer));
				}
			}
		} else {
			close(connect_fd);
		}

	}

	return 0;
}
