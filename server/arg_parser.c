#include "args_parser.h"

#include <stdio.h>
#include <string.h>

static
void print_help() {
	printf("Welcome to tantivy-server\n");
	printf("usage:\n");
	printf("-u --unix-sock =unix socket path e.g. /my/socket.path\n");
	printf("-f --log-file =log file path path e.g. /my/log.txt\n");
	printf("-n --newline [n no, y yes] = auto append new line character to log if missing.\n");	
}

int parse_args(int argc, char** argv, 
	       char **unix_socket_path,
	       char **log_file_path,
	       bool *auto_append_newline) {
	
	enum { read_args, 
		unix_socket_path_next, 
		log_file_path_next,
		appened_newline_next
	} arg_state = read_args;
	
	
	*unix_socket_path = "/tmp/tantivy.logger.unix.sock";
	*log_file_path = "/tmp/tantivy.log";
	*auto_append_newline = true;
	
	while(argc--) {
		
		switch (arg_state) {
			
		case read_args:
			if (strcmp(*argv, "-h") == 0 
			|| strcmp(*argv, "--help") == 0) {
			       print_help();
			       return 1;
			}
			else if (strcmp(*argv, "-u") == 0
				 || strcmp(*argv, "--unix-sock") == 0) {
				arg_state = unix_socket_path_next;
			}
			else if (strcmp(*argv, "-f") == 0
				 || strcmp(*argv, "--log-file") == 0) {
				arg_state = log_file_path_next;
			}
			else if (strcmp(*argv, "-n") == 0 
				 || strcmp(*argv, "--newline") == 0) {
				arg_state = appened_newline_next;
			}		
			break;
			
		case unix_socket_path_next:
			*unix_socket_path = strdup(*argv);
			arg_state = read_args;
			break;
		
		case log_file_path_next:
			*log_file_path = strdup(*argv);
			arg_state = read_args;
			break;
			
		case appened_newline_next:	
			*auto_append_newline = strcmp(*argv, "y") == 0
			                || strcmp(*argv, "yes") == 0;
			arg_state = read_args;
			break;
		}
		
                *argv++;
	}
	
	printf("tantivy logging server is using:\n\n");
	printf("unix_socket_path:    %s\n", *unix_socket_path);
	printf("log_file_path:       %s\n", *log_file_path);
	printf("auto_append_newline: %d\n\n", *auto_append_newline);
	
	return 0;
}

