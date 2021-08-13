/************************************************************************
 * File:	dns.cpp
 * Project:	DNS Resolver
 * Subject: Network Applications and Network Administration(ISA)
 * Author:	Damián Sova (xsovad06)
 * Date:	18.11.2020
 ***********************************************************************/

// Included header with libraries, constants,
// struct declaration and function prototypes
#include "header.hpp"

using namespace std;

/************************************************************************
 ****************************** MAIN FUNCTION ***************************
***********************************************************************/
int main(int argc, char *argv[]) {
	// Proccessing program arguments
	Arguments *args = Arguments::arg_processor(argc, argv);
	if (args == NULL) {
		print_help();
		delete args;
		return EXIT_FAILURE;
	}

	// Create connection with user
	if (bind_to_user(args->port_number) != SUCCESS) {
		delete args;
		free(message);
		return EXIT_FAILURE;
	}

	// Infinity loop
	while(true)
	{
		// Handle with user request, check query
		if (user_query_handler(args) != SUCCESS) {
			continue;
		}

		// Connect DNS server
		if (create_dns_connection(args->server.c_str()) != SUCCESS) {
			continue;
		}

		// Send query to server and response to user
		if (dns_query_user_response() != SUCCESS) {
			continue;
		}
	}
}

/************************************************************************
 * ************ IMPLEMENTATION OF THE ADDITIONAL FUNCTIONS **************
 * *********************************************************************/

// Function for printing help message when only parameter --help/-h is set 
// or if invalid arguments are set
void print_help() {
	cout <<
	"					DNS resolver\n\n"
	"		Program use:\n"
	"		dns -s <IP_adress|Domain_name> [-p <Port_number>] -f filter_file\n"
	"		-s <IP_adress|Domain_name>	of DNS server> (resolver) where DNS queries are sent\n"
	"		-p <Port_number>		where program is listening, defaul port number is 53\n"
	"		-f <filter_file>		consinsts of unsupported domain names\n"
	"		-h/--help			Show help\n";
	exit(EXIT_SUCCESS);
}

// Print error message on strerr
void error_message(const char *message) {
	perror(message);
	exit(EXIT_FAILURE);
}

// Load Undesired domains to vector of strings from file
int Arguments::load_file(Arguments *arguments, string filter_file) {
	// Attach an input stream to the filter file
	ifstream input_stream(filter_file);
	if (!input_stream) {
		perror("Can not open input file!");
		return(EXIT_FAILURE);
	}
	string domain;

	// Store every domain from file
	while (getline(input_stream, domain))
	{
		arguments->undesired_domians.push_back(domain);
	}
	return EXIT_SUCCESS;
}

// Function parses arguments from command line and saves the into the instance of this class
Arguments* Arguments::arg_processor(int argc, char** argv) {	
	Arguments *args = new Arguments();			// Instance of the class Arguments
	unsigned int arg_processed = 0;				// Number of actualy proccessed arguments
	bool server_was_set = false;				// If server was set
	bool file_was_set = false;				// If file was set

	const char* const short_opts = "s:p:f:h";	// Short arguments options
	const option long_opts[] = {
		{"help", no_argument, nullptr, 'h'},
		{nullptr, no_argument, nullptr, 0}
	};											// Long arguments options 

	// Till all arguments are processed
	while (true)
	{
		// Function for parsing program arguments
		const char opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);
		// End of the parsing
		if (opt == -1 && server_was_set && file_was_set) {
			return args;
		}
		else if (opt == -1 && !server_was_set) {
			cerr << "Missing argument -s <server>" << endl;
			return NULL;
		}
		else if (opt == -1 && !file_was_set) {
			cerr << "Missing argument -f <filter_file>" << endl;
			return NULL;
		}

		// Parsing options
		switch (opt) {
			// -s <server>
			case 's':
				if (optarg[0] != '-') {
					args->server = optarg;
					server_was_set = true;
					arg_processed++;
					break;
				}
				else {
					perror("Missing value after argument -s.");
					return NULL;
				}
			
			// -p <port_number>
			case 'p':
				try {
					args->port_number = to_string(stoi(optarg));
				}
				catch(...) {
					perror("Missing or incorrect value after argument -p.");
					return NULL;
				}
				arg_processed++;
				break;
			// -f <filter_file>
			case 'f':
				if (Arguments::load_file(args, optarg) == UNSUCCESS) {
					return NULL;
				}
				file_was_set = true;
				arg_processed++;
				break;
			// -h/--help
			case 'h':
				if (arg_processed == 0) {
					print_help();
					delete args;
					exit(EXIT_SUCCESS);
				}
				return NULL;
			// Other options raise argument error
			default:
				return NULL;
		}
	}
}

// Parse cname field in dns header
char* cname_handler(char *start_label) {
	while(*start_label != 0)
	{
		// For the number in the first octet of label print the next characters from label
		int8_t label_value = *((int8_t *)(start_label));
		for(int8_t i = 0; i < label_value; i++) {
			// Write target domain name to string
			target_domain.push_back(start_label[1 + i]);
		}
		// get to the next label = Pointer to the token label + 1 byte (label number) + label chars itself
		start_label += label_value + 1;
		// If its not last label, print dot
		if (*start_label != 0) {
			target_domain.append(".");
		}
	}
	// Return the the label + skip the 0x00
	return start_label + 1;
}

// Create connection with user
int bind_to_user(string port_number) {
	// Creating socket file descriptor 
	if ((user_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
		perror("Socket creation failed.");
		return EXIT_FAILURE;
	} 

	// O out the addresses
	memset(&server_addr, 0, sizeof(server_addr)); 
	memset(&client_addr, 0, sizeof(client_addr)); 
		
	// Filling server information 
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY; 
	server_addr.sin_port = htons(stoi(port_number)); 
		
	// Bind the socket with the server address 
	if (::bind(user_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == UNSUCCESS ) { 
		cerr << "Bind failed: " << strerror(errno) << endl;
		return EXIT_FAILURE;
	}

	// Allocate memmory for recieved message
	message = (char *)malloc(MAX_SIZE);
	if (!message) {
		perror("Message allocation failed.");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

// Recieve and check DNS query, if unsupported query, send response to user
int user_query_handler(Arguments *args) {
	unsigned int client_len; 
	client_len = sizeof(client_addr);
	strcpy(message, "");					// Clear message content because of infinity loop
	message_len = 0;
 
	// Recieved packet
	if ((message_len = recvfrom(user_socket, message, MAX_SIZE, MSG_WAITALL, (struct sockaddr *) &client_addr, &client_len)) < SUCCESS ) {
		cerr << "Recieved from failed: " << strerror(errno) << endl;
		return EXIT_FAILURE;
	}

	// Using structure for parsing recieved packet
	dns_header *dns_header_query = (dns_header *)message;
	char *dns_questions_start = message + sizeof(dns_header);
	
	// Clearance because of infinity loop
	target_domain = "";

	// Store the domain name in the question to global variable target_domain
	dns_question *dns_question_end = (dns_question*)cname_handler(dns_questions_start);

	// DNS question type must be A, otherwise send back NOTIMP
	if ((ntohs(dns_header_query->Flags & FLAG_QR)) == QUERY && (ntohs(dns_question_end->Qtype)) != A) {
		// Set flags for NOTIMP response
		dns_header_query->Flags &= 0x0000;
		dns_header_query->Flags = htons(NOTIMP);

		// Send back response NOTIMP to the User
		if ((sendto(user_socket, (const char *)message, message_len, 0, (const struct sockaddr *) &client_addr, client_len)) == UNSUCCESS) {
			cerr << "Send to failed: " << strerror(errno) << endl;
			return EXIT_FAILURE;
		}
		return END;	
	}

	// Sequence filter of undesired domains
	for (auto domain:args->undesired_domians) {
		if (target_domain.find(domain) != string::npos) {
			// Set flags for REFUSED response
			dns_header_query->Flags &= 0x0000;
			dns_header_query->Flags = htons(REFUSED);

			// Send back response REFUSED from DNS server to the User
			if ((sendto(user_socket, (const char *)message, message_len, 0, (const struct sockaddr *) &client_addr, client_len)) == UNSUCCESS) {
				cerr << "Send to failed: " << strerror(errno) << endl;
				return EXIT_FAILURE;
			}
			return END;
		}
	}
	return EXIT_SUCCESS;
}

// Create socket for DNS server and open connection
int create_dns_connection(const char *server) {
	int rc;
	struct addrinfo hints;
	struct addrinfo *info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	// Getting details about dns server
	if ((rc = getaddrinfo(server, DNS_PORT, &hints, &info)) != SUCCESS) {
		cerr << "Get address info failed :" << gai_strerror(rc) << endl;
		return(EXIT_FAILURE);
	}

	// Iterate throught the whole list of address structures until we successfully connect
	for (; info != NULL; info = info->ai_next) {
		// Creating socket for DNS server connection
		if ((dns_socket = socket(info->ai_family, info->ai_socktype, info->ai_protocol)) == UNSUCCESS) {
			continue;				// Success, continue establishing conection
		}

		// Establishing connection with DNS server		
		if ((connect(dns_socket, info->ai_addr, info->ai_addrlen)) != UNSUCCESS) {
			break;					// Success, breaking iteration
		}

		close(dns_socket);
	}
	// No address succeeded
	if (info == NULL) {
		perror("Could not connect DNS server.");
		return EXIT_FAILURE;
	}
	freeaddrinfo(info);
	return EXIT_SUCCESS;
}

// Send DNS query from user to DNS server, recieve response and send it to user
int dns_query_user_response() {
	// Send the packet
	if ((send(dns_socket, (const char *)message, message_len, 0)) == UNSUCCESS) {
		cerr << "Sent failed: " << strerror(errno) << endl;
		return(EXIT_FAILURE);
	}

	// set timeout on recv 
	struct timeval timeout;
	timeout.tv_sec = DNS_WAIT_TIME;
	timeout.tv_usec = 0;
	setsockopt(dns_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

	// Recieved response from DNS server
	if ((message_len = recv(dns_socket, message, MAX_SIZE, 0)) < SUCCESS) {
		cerr << "Recieve failed: " << strerror(errno) << endl;
		return(EXIT_FAILURE);
	}
	close(dns_socket);

	// Send back response from DNS server to the User
	if ((sendto(user_socket, (const char *)message, message_len, 0, (const struct sockaddr *) &client_addr, (unsigned int)sizeof(client_addr))) == UNSUCCESS) {
		cerr << "Sent to failed: " << strerror(errno) << endl;
		return(EXIT_FAILURE);
	}
	return EXIT_SUCCESS;
}
