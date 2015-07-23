#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "log_parser.h"

uv_loop_t *loop;
uv_pipe_t file_pipe;
uv_pipe_t socket_pipe;

const char * unix_socker_path = "/tmp/test.sock";
const char * log_filename = "/tmp/test.log";

void after_write(uv_write_t *req, int status) {
	
	if (status < 0) {
	    fprintf(stderr, "Write error %s\n", uv_err_name(status));
	}
	
	free(req->data);
	free(req);
}

void on_log_entry_found(char *log_entry, int length) {
	
	uv_write_t *req;
	uv_buf_t wrbuf;
	
	req = (uv_write_t *) malloc(sizeof(uv_write_t));
	wrbuf = uv_buf_init(log_entry, length);
	req->data = log_entry;
	
	uv_write(req, (uv_stream_t*)&file_pipe, &wrbuf, 1, after_write);
}

static void on_close(uv_handle_t* client) {
	
	if (client->data)
		free_log_parser_context((log_parser_context_t *)client->data);
		//free(client->data);
	free(client);
}

void after_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {	
		
	if (nread < 0) {		
		if (nread != UV_EOF)
			fprintf(stderr, "Read error %s\n", uv_err_name(nread));
		
		free(buf->base);		
		uv_close((uv_handle_t*) client, on_close);
		return;
	}
	
	parse_log((log_parser_context_t *)client->data, 
		  buf->base, 
		  nread, 
		  on_log_entry_found);
	
	free(buf->base);
}

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
	
	buf->base = malloc(suggested_size);
	buf->len = suggested_size;
}

void on_socket_connection(uv_stream_t *server, int status) {	
	uv_pipe_t *client;
	
	if (status == -1) {
		// error!
		return;
	}

	client = (uv_pipe_t*) malloc(sizeof(uv_pipe_t));
	client->data = create_log_parser_context();
	
	uv_pipe_init(server->loop, client, 0);
	
	if (uv_accept(server, (uv_stream_t*) client)) {
		//error
		uv_close((uv_handle_t*)client, on_close);
		return;
	}
	
	uv_read_start((uv_stream_t*) client, alloc_buffer, after_read);
}

void remove_sock(int sig) {
    uv_fs_t req;
    uv_fs_unlink(loop, &req, unix_socker_path, NULL);
    exit(0);
}

int main() {
	
	loop = uv_default_loop();
	
	uv_pipe_init(loop, &socket_pipe, 0);

	signal(SIGINT, remove_sock);

	int r;
	if ((r = uv_pipe_bind(&socket_pipe, unix_socker_path))) {
		fprintf(stderr, "Bind error %s\n", uv_err_name(r));
		return 1;
	}
	if ((r = uv_listen((uv_stream_t*) &socket_pipe, 128, on_socket_connection))) {
		fprintf(stderr, "Listen error %s\n", uv_err_name(r));
		return 2;
	}
	
	int file_discriptor;
	uv_fs_t file_request;
	
	file_discriptor = uv_fs_open(loop, 
				 &file_request, 
				 log_filename, 
				 O_CREAT | O_RDWR, 
				 0644, 
				 NULL);

	uv_pipe_init(loop, &file_pipe, 0);
	
	if ((r = uv_pipe_open(&file_pipe, file_discriptor))){
		fprintf(stderr, "Listen error %s\n", uv_err_name(r));
		return 2;
	}
    
	return uv_run(loop, UV_RUN_DEFAULT);
}
