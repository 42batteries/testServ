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

int main() {

	int sockfd, ret;
	struct sockaddr_in serverAddr;

	int newSocket;
	struct sockaddr_in newAddr;

	socklen_t addr_size;

	char buffer[INPUT_BUFFER_LEGHT];
	pid_t childpid;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Server Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr(IP_ADRESS_SERVER);

	ret = bind(sockfd, (struct sockaddr*) &serverAddr, sizeof(serverAddr));
	if (ret < 0) {
		printf("[-]Error in binding.\n");
		exit(1);
	}
	printf("[+]Bind to port %d\n", PORT);

	if (listen(sockfd, SOCKET_QUEUE_LENGHT) == 0) {
		printf("[+]Listening....\n");
	} else {
		printf("[-]Error in binding.\n");
	}

	while (1) {
		newSocket = accept(sockfd, (struct sockaddr*) &newAddr, &addr_size);
		if (newSocket < 0) {
			exit(1);
		}
		printf("Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr),
				ntohs(newAddr.sin_port));

		if ((childpid = fork()) == 0) {
			close(sockfd);

			while (1) {
				recv(newSocket, buffer, INPUT_BUFFER_LEGHT, 0);
				if (!strcmp(buffer, "exit\r\n")) {
					strcpy(buffer, "Connection closed");
					send(newSocket, buffer, strlen(buffer), 0);
					printf("Disconnected from %s:%d\n",
							inet_ntoa(newAddr.sin_addr),
							ntohs(newAddr.sin_port));
					break;
				} else {
					printf("Client: %s\n", buffer);
					strcpy(buffer, calculate_expression(buffer));
					send(newSocket, buffer, strlen(buffer), 0);
					bzero(buffer, sizeof(buffer));
				}
			}
		}

	}

	close(newSocket);

	return 0;
}
