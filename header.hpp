/************************************************************************
 * File:	header.hpp
 * Project:	DNS Resolvver
 * Subject: Network Applications and Network Administration(ISA)
 * Author:	Damián Sova (xsovad06)
 * Date:	18.11.2020
 ***********************************************************************/

// Libraries
#include <iostream>					// cerr
#include <fstream>					// ifstream
#include <vector>					// vector<>
#include <getopt.h>					// getopt_long()				
#include <string.h>					// sting
#include <netdb.h>
#include <unistd.h> 
#include <sys/socket.h> 

// DNS HEADER QR VALUE
#define QUERY				0
#define RESPONSE			1

// QUERY TYPE VALUES
#define A					1

// QUERY CLASS VALUES
#define IN					1

// QUERY RCODE VALUES
#define RCODE_OK			0x0000
#define RCODE_NOTIMP		0x0004

// QDCOUNT/ARCOUNT OPTIONS
#define NO_QUESTION			0

// RD/RA RECURSION DESIRED/AVAILABLE
#define NO_RECURSION		0

// DNS CONSTANTS
#define MAX_SIZE			512			// Max size of DNS packet
#define DNS_WAIT_TIME		10			// Sec waiting for DNS response
#define DNS_PORT			"53"		// Port number for connection with DNS server

// SUCCESSFULNESS OF CONNECTION FUNCTIONS
#define SUCCESS				0
#define UNSUCCESS			-1
#define END					-2			// Response to user was sent

// DNS HEADER FLAGS MASKS
#define FLAG_QR				0x8000		// Response value(1), query value(0)
#define FLAG_OPCODE			0x7800		// Query type, standard query(0) 
#define FLAG_RD				0x0100		// Recursion denied
#define FLAG_RA				0x0080		// Recursion available
#define RCODE				0x000f		// Response code

// FLAGS LINE ANSWERS
#define NOTIMP				32772		// QR = 1(RESPONSE), RCODE = 4(NOTIMP)
#define REFUSED				32773		// QR = 1(RESPONSE), RCODE = 5(REFUSED)

// GLOBALS
std::string target_domain = "";			// IP/domain name from dns query as string
int user_socket;						// Socket for connection with User
int dns_socket;							// Socket for connect to DNS server
struct sockaddr_in server_addr;			// Address struct for binding with user port
struct sockaddr_in client_addr;			// Address struct of user, where to send responses
char *message;							// Recieved DNS query from user
unsigned int message_len;				// Lenght of the recieved query

// Class representing and handling with arguments reached from the command line
class Arguments
{
	public:
		std::string server;				// String with IP/domain name for dns server
		std::string port_number;		// Port number on which are we listening, as string
		std::vector<std::string> undesired_domians;	// Vector of undesired domains from file
		
		// Load Undesired domains to vector of strings from file
		static int load_file(Arguments *arguments, std::string filter_file);

		// Function parses arguments from command line and saves the into the instance of this class
		static Arguments* arg_processor(int argc, char **argv);

	// Arguments constructor, variable initialization
	Arguments()
	{
		this->server = "";				// String initialization
		this->port_number = "53";		// Default DNS port number is 53
	}
};

// Structure for DNS header
typedef struct dns_headers{
	u_int16_t ID;						// | 0| 1 2 3 4| 5| 6| 7| 8| 9 0 1| 2 3 4 5|
	u_int16_t Flags;	 				// |QR| Opcode |AA|TC|RD|RA|   Z  |  RCODE |
	u_int16_t Qcount;					// Question number
	u_int16_t Acount;
	u_int16_t NScount;
	u_int16_t ARcount;					// Aditional record section
} dns_header;

// Structure for question header
typedef struct dns_questions{
	u_int16_t Qtype :16;				// Only A supported
	u_int16_t Qclass :16;				// Only IN supported
} dns_question;

// Function for printing help message when only parameter --help/-h is set
void print_help();

// Print error message on strerr
void error_message(const char *message);

// Parse cname field in dns header
char* cname_handler(char *start_label);

// Create connection with user
int bind_to_user(std::string port_number);

// Recieve and check DNS query, if unsupported query, send response to user
int user_query_handler(Arguments *args);

// Create socket for DNS server and open connection
int create_dns_connection(const char *server);

// Send DNS query from user to DNS server, recieve response and send it to user
int dns_query_user_response();