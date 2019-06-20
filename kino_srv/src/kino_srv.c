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
#include <sys/types.h>
#include <math.h>
#include <float.h>
#if defined(_WIN32)
#include <winsock2.h>
//#include <Winsock.h>
typedef int socklen_t;
#define close(a) closesocket(a)
#define SOCKOPT_ARG_COMPAT(a) ((const char *) a)
#define SOCKINIT WSADATA WsaData;WSAStartup( MAKEWORD(2,2), &WsaData );
#define SOCKSTOP WSACleanup();

typedef struct {
	int connect_fd;
	char * buffer;
	struct sockaddr_in * client_socket_address;

}win_params_t;


#else

#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SOCKINIT
#define SOCKSTOP

#endif

/*net*/
#define PORT 					(4444)
#define IP_ADRESS_SERVER		"127.0.0.1"
#define SOCKET_QUEUE_LENGHT		(10)
/*io*/
#define INPUT_NUMBER_LENGHT		(16)
#define INPUT_BUFFER_LENGHT		(1024)
#define OUTPUT_BUFFER_LENGHT	(64)
/*parsing*/
#define OPERATION_SIMBOL_LENGHT	(1)
#define INPUT_VALUE_MAX			(999)
#define INPUT_VALUE_MIN			(-999)
#define SEPARATOR  				" "
#define END_SIMBOL_CODE			(0x0a)
/*check and validation*/
typedef enum Validations {
	VALIDATION_OK,
	VALIDATION_ERROR_FIRST_NUMBER_LENGHT,
	VALIDATION_ERROR_FIRST_NUMBER_RANGE,
	VALIDATION_ERROR_FIRST_NUMBER_INCORRECT_OR_ZERO,
	VALIDATION_ERROR_SECOND_NUMBER_LENGHT,
	VALIDATION_ERROR_SECOND_NUMBER_RANGE,
	VALIDATION_ERROR_SECOND_NUMBER_INCORRECT_OR_ZERO,
	VALIDATION_ERROR_UNKNOWN_OPERATION,
	VALIDATION_ERROR_INPUT_DATA,
} Errors;

static int operation_cnt;

typedef struct {
	char * operation_simbol;
	double (*operation)(double, double);
} function_types;

typedef struct {
	double fn;
	double sn;
	char * operation;
	char * buff;
	char * string_first_number;
	char * string_second_number;
	char * operation_string;
} in_data;

/*parsing and check validations*/
static int define_and_fill_numbers_in_input_struct(in_data * indata);
static int parse_and_check_validity(in_data * indata);
static double select_operation_and_process_calculation(in_data * indata,
		function_types * func_types);
static void set_error_msg(char * buff, int error);
/*net functions*/
static int init_server(int type, const struct sockaddr *address_server,
		socklen_t address_lenght, int queue_lenght);
static int send_all(int connect_fd, char *buffer, int numBytes, int flags);
static int recv_all(int connect_fd, char *buffer, int numBytes, int flags);
/*calculations*/
static void init_operations(function_types ** f_types);
static void add_operation_to_operations(function_types ** f_types,
		char * operation_simbol, double (*operation)(double fn, double sn));
static char * calculate_expression(char * buff_);
/*calculations - operations*/
static double operation_addition(double fn, double sn);
static double operation_subtraction(double fn, double sn);
static double operation_multiplication(double fn, double sn);
static double operation_division(double fn, double sn);
static double operation_exponentiation(double fn, double sn);
static double operation_minimum(double fn, double sn);
static double operation_maximum(double fn, double sn);
static double operation_hypotenuse(double fn, double sn);
static double operation_difference(double fn, double sn);
static double operation_remainder(double fn, double sn);

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

	return VALIDATION_OK;
}

static int parse_and_check_validity(in_data * indata) {

	indata->string_first_number = strtok(indata->buff, SEPARATOR);

	if (indata->string_first_number != NULL) {
		indata->operation = strtok(NULL, SEPARATOR);
	} else {
		return VALIDATION_ERROR_INPUT_DATA;
	}
	if (indata->operation != NULL) {
		indata->string_second_number = strtok(NULL, SEPARATOR);
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

static double operation_addition(double fn, double sn) {
	return fn + sn;
}
static double operation_subtraction(double fn, double sn) {
	return fn - sn;
}
static double operation_multiplication(double fn, double sn) {
	return fn * sn;
}
static double operation_division(double fn, double sn) {
	return fn / sn;
}
static double operation_exponentiation(double fn, double sn) {
	return pow(fn, sn);
}
static double operation_minimum(double fn, double sn) {
	return fmin(fn, sn);
}
static double operation_maximum(double fn, double sn) {
	return fmax(fn, sn);
}
static double operation_hypotenuse(double fn, double sn) {
	return hypot(fn, sn);
}
static double operation_difference(double fn, double sn) {
	return fdim(fn, sn);
}
static double operation_remainder(double fn, double sn) {
	return remainder(fn, sn);
}

static double select_operation_and_process_calculation(in_data * indata,
		function_types * func_types) {
int n;
	for (n = 0; n < operation_cnt; n++) {
		if (!strcmp((func_types + n)->operation_simbol, indata->operation)) {
			return (func_types + n)->operation(indata->fn, indata->sn);
			break;
		}
	}

	return 0;
}

static void set_error_msg(char * buff, int error) {
	switch ((Errors) error) {
	case VALIDATION_OK:

		break;
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
static int check_valid_operarion_simbol(in_data * indata,
		function_types * func_types) {
	int n;
	for (n = 0; n < operation_cnt; n++) {
		if (!strcmp((func_types + n)->operation_simbol, indata->operation)) {

			return VALIDATION_OK;
		}
	}

	return VALIDATION_ERROR_UNKNOWN_OPERATION;
}
static char * calculate_expression(char * buff_) {

	in_data input_data;
	input_data.buff = buff_;

	int err = parse_and_check_validity(&input_data);

	if (err == VALIDATION_OK) {
		function_types * func_types = malloc(sizeof(function_types));

		init_operations(&func_types);
		err = check_valid_operarion_simbol(&input_data, func_types);
		if (err == VALIDATION_OK) {
			double rez_dbl = select_operation_and_process_calculation(
					&input_data, func_types);
			int rez_int = (int) rez_dbl;
			if (fabs(rez_dbl - (double) rez_int)
					<= DBL_EPSILON
							* fmax(fabs(rez_dbl), fabs((double) rez_int))) {
				snprintf(buff_, OUTPUT_BUFFER_LENGHT, "%d\n", rez_int);
			} else {
				snprintf(buff_, OUTPUT_BUFFER_LENGHT, "%0.2f\n", rez_dbl);
			}
		}
		free(func_types);
	}
	if (err != VALIDATION_OK)
		set_error_msg(buff_, err);

	return buff_;
}
static void init_operations(function_types ** f_types) {
	operation_cnt = 0;
	add_operation_to_operations(f_types, "+", operation_addition);
	add_operation_to_operations(f_types, "-", operation_subtraction);
	add_operation_to_operations(f_types, "*", operation_multiplication);
	add_operation_to_operations(f_types, "/", operation_division);
	add_operation_to_operations(f_types, ":", operation_division);
	add_operation_to_operations(f_types, "^", operation_exponentiation);
	add_operation_to_operations(f_types, "m", operation_minimum);
	add_operation_to_operations(f_types, "M", operation_maximum);
	add_operation_to_operations(f_types, "h", operation_hypotenuse);
	add_operation_to_operations(f_types, "d", operation_difference);
	add_operation_to_operations(f_types, "r", operation_remainder);

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
	int counter_bytes_sended_per_iter = 0;

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
	int counter_bytes_recived_per_iter = 0;
	int counter_total_bytes = 0;

	while ((counter_total_bytes < numBytes)
			&& counter_bytes_recived_per_iter != -1
			&& !strrchr(buffer, END_SIMBOL_CODE)) {
		counter_bytes_recived_per_iter = recv(connect_fd,
				buffer + counter_total_bytes, numBytes - counter_total_bytes,
				flags);

		if (counter_total_bytes == -1) {
			close(connect_fd);
			printf("[-]Error while receive data. Connection will be closed\n");
			exit(1);
		} else {
			counter_total_bytes += counter_bytes_recived_per_iter;
		}
	}

	buffer[counter_total_bytes] = '\0';
	return (counter_total_bytes < 1 ? -1 : counter_total_bytes);

}

static void add_operation_to_operations(function_types ** f_types,
		char * operation_simbol, double (*operation)(double fn, double sn)) {

	if (!(*f_types = realloc(*f_types,
			sizeof(function_types) * (operation_cnt + 1)))) {
		printf("Error allocation memory\n");
		exit(1);
	}
	(*f_types + operation_cnt)->operation_simbol = operation_simbol;
	(*f_types + operation_cnt)->operation = operation;
	operation_cnt++;
}
#if defined(_WIN32)
DWORD WINAPI win_thread(void * param) {
	win_params_t * ptr = (win_params_t *) param;

	while (1) {
		if (recv_all(ptr->connect_fd, ptr->buffer, INPUT_BUFFER_LENGHT, 0)
				== -1) {
			printf(
					"[-]Error:Data was not received. Connection will be closed\n");
			close(ptr->connect_fd);
			return (0);
		}

		if (!strcmp(ptr->buffer, "exit\r\n")) {

			printf("Disconnected from %s:%d\n",
					(inet_ntoa(ptr->client_socket_address->sin_addr)) == 0 ?
							"address not defined" :
							inet_ntoa(ptr->client_socket_address->sin_addr),
					ntohs(ptr->client_socket_address->sin_port));
			close(ptr->connect_fd);
			return (0);
		} else {
			printf("Client: %s\n", ptr->buffer);
			calculate_expression(ptr->buffer);
			if (send_all(ptr->connect_fd, ptr->buffer, strlen(ptr->buffer), 0)
					== -1)
				return (0);

			memset(ptr->buffer, 0, sizeof(ptr->buffer));
		}

	}

	return 0;
}
#endif
int main() {

	int socket_fd;
	struct sockaddr_in seraddr;
	char buffer[INPUT_BUFFER_LENGHT] = { 0 };

	SOCKINIT
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
#if defined(_WIN32)

		static win_params_t params;
		params.connect_fd = connect_fd;
		params.buffer = buffer;
		params.client_socket_address = &client_socket_address;

		HANDLE hThr = CreateThread( NULL, 0,
				(LPTHREAD_START_ROUTINE) win_thread, (void*) &params, 0, NULL);
		if (NULL == hThr) {
			printf("[-]Error: Failed to create thread.\n");
		}

#else

		signal(SIGCHLD, SIG_IGN);
		if (fork() == 0) {
			close(socket_fd);
			while (1) {
				if (recv_all(connect_fd, buffer, INPUT_BUFFER_LENGHT, 0)
						== -1) {
					printf(
							"[-]Error:Data was not received. Connection will be closed\n");
					close(connect_fd);
					return (0);
				}

				if (!strcmp(buffer, "exit\r\n")) {

					printf("Disconnected from %s:%d\n",
							(inet_ntoa(client_socket_address.sin_addr)) == 0 ?
							"address not defined" :
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

#endif
		SOCKSTOP
	}

	return 0;
}
