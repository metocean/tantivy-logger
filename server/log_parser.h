/* 
 * File:   log_parsing.h
 * Author: gregc
 *
 * Created on 23 July 2015, 12:55 AM
 */

#ifndef LOG_PARSING_H
#define	LOG_PARSING_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef enum { 
	state_reset = 0, 
	state_reading_length,
	state_reading_log_entry
} state_t;

typedef struct log_parser_context {
	// we have two states:
	// state_reading_length = We reading two bytes to determine the log entry length
	// state_reading_log_entry = Here we a reading the actual log data.
	state_t state;
	char length_str[12];
	int length_pos;
	int length;
	int position;
	char *entry;
        
        bool append_newlines;
        char start_of_length_char;
        char end_of_length_char;
        int max_log_entry_size;
        
} log_parser_context_t;

log_parser_context_t * create_log_parser_context();
void free_log_parser_context(log_parser_context_t *context);

typedef void (*on_log_entry_found_cb)(char *entry, int length);

void parse_log(log_parser_context_t *context, 
               const char *data, 
               int nread, 
               on_log_entry_found_cb callback);

#ifdef	__cplusplus
}
#endif

#endif	/* LOG_PARSING_H */

