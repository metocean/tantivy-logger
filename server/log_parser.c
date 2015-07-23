#include "log_parser.h"

#include <stdlib.h>
#include <string.h>

log_parser_context_t * create_log_parser_context()
{
	log_parser_context_t *cnt;
	
	cnt = (log_parser_context_t *)malloc(sizeof(log_parser_context_t));
	cnt->state = state_reset;	
	cnt->entry = NULL;
	cnt->length = 0;
	cnt->append_newlines = true;
	cnt->max_log_entry_size = (1024 * 1024 * 2); // 2 MB
	cnt->start_of_length_char = '~';
	cnt->end_of_length_char = ';';
	
	return cnt;
}

void free_log_parser_context(log_parser_context_t *cnt) 
{	
	if (cnt->entry)
		free(cnt->entry);	
	free(cnt);
}

void parse_log(log_parser_context_t *cnt, 
               const char *data, 
               int nread, 
               on_log_entry_found_cb callback)
{
	int log_remaining;
	int data_remaining;
	int to_copy;	
	int i;
	
//	if (!cnt || !data || !callback)
//		return;
	
	i = 0;
	while (i < nread) {
		switch (cnt->state){
		case state_reset:
			if (data[i++] == cnt->start_of_length_char) {
				cnt->state = state_reading_length;
				cnt->length_pos = 0;
				cnt->length = 0;
				cnt->position = 0;
				
				if (cnt->entry) {
					free(cnt->entry);
					cnt->entry = NULL;
				}
			}
			break;
			
		case state_reading_length:
			if (cnt->length_pos >= sizeof (cnt->length_str) - 1) {
				cnt->state = state_reset;
			}
			else if (data[i] == cnt->end_of_length_char) {
				cnt->state = state_reading_log_entry;
				i++;				
				cnt->length_str[cnt->length_pos] = 0;
				cnt->length = atoi(cnt->length_str);
				
				if (cnt->entry) {
					free(cnt->entry);
					cnt->entry = NULL;
				}
				
				if (cnt->length <= cnt->max_log_entry_size) {
					cnt->entry = malloc(cnt->length + 2);
					cnt->entry[cnt->length] = 0;
					cnt->entry[cnt->length + 1] = 0;
				}
			}
			else {
				cnt->length_str[cnt->length_pos++] = data[i++];
			}
			break;			
				
		case state_reading_log_entry:			
					
			log_remaining = cnt->length - cnt->position;
			data_remaining = nread - i;			
			to_copy = log_remaining < data_remaining ?
				  log_remaining : data_remaining;
			
			if (cnt->entry)
				memcpy(&cnt->entry[cnt->position],
				       &data[i],
				       to_copy);
			
			cnt->position += to_copy;
			i += to_copy;
			
			if (cnt->position == cnt->length) {
				cnt->state = state_reset;
				
				if (cnt->entry) {
					
					if (cnt->append_newlines
					   && cnt->entry[cnt->length - 1] != '\n')
						cnt->entry[cnt->length++] = '\n';					
						
					callback(cnt->entry,  cnt->length);
					
					cnt->entry = NULL;
				}
			}
			
			break;
		}
	}
}
