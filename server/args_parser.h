/* 
 * File:   args_parser.h
 * Author: gregc
 *
 * Created on 24 July 2015, 2:39 PM
 */

#ifndef ARGS_PARSER_H
#define	ARGS_PARSER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>

int parse_args(int argc, char** argv, 
	       char **unix_socket_path,
	       char **log_file_path,
	       bool *auto_append_newline);

#ifdef	__cplusplus
}
#endif

#endif	/* ARGS_PARSER_H */

