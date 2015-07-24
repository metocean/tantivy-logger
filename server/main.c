#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "args_parser.h"
#include "log_parser.h"

struct {
	uv_loop_t *loop;
	uv_pipe_t log_file_pipe;
	uv_pipe_t unix_sock_pipe;

	char *unix_socket_path;
	char *log_file_path;
	bool auto_append_newline;
	
} g_app;

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
	
	uv_write(req, (uv_stream_t*)&g_app.log_file_pipe, &wrbuf, 1, after_write);
}

static void on_close(uv_handle_t* client) {
	
	if (client->data)
		free_log_parser_context((log_parser_context_t *)client->data);
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
	client->data = create_log_parser_context(g_app.auto_append_newline);
	
	uv_pipe_init(server->loop, client, 0);
	
	if (uv_accept(server, (uv_stream_t*) client)) {
		//error
		uv_close((uv_handle_t*)client, on_close);
		return;
	}
	
	uv_read_start((uv_stream_t*) client, alloc_buffer, after_read);
}

void kill_server(int sig) {
    uv_fs_t req;
    uv_fs_unlink(g_app.loop, &req, g_app.unix_socket_path, NULL);
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
	
	int r;
	int log_file_fd;
	uv_fs_t file_request;
	
	if ((r = parse_args(argc, argv, 
			 &g_app.unix_socket_path, 
			 &g_app.log_file_path, 
			 &g_app.auto_append_newline)))
		return r == 1 ? (EXIT_SUCCESS) : r;
	
	g_app.loop = uv_default_loop();
	
	uv_pipe_init(g_app.loop, &g_app.unix_sock_pipe, 0);

	signal(SIGINT, kill_server);
	
	if ((r = uv_pipe_bind(&g_app.unix_sock_pipe, g_app.unix_socket_path))) {
		fprintf(stderr, "Bind error %s\n", uv_err_name(r));
		return 1;
	}
	if ((r = uv_listen((uv_stream_t*) &g_app.unix_sock_pipe, 128, on_socket_connection))) {
		fprintf(stderr, "Listen error %s\n", uv_err_name(r));
		return 2;
	}
	
	
	log_file_fd = uv_fs_open(g_app.loop, 
				 &file_request, 
				 g_app.log_file_path, 
				 O_CREAT | O_RDWR, 
				 0644, 
				 NULL);
	
	uv_pipe_init(g_app.loop, &g_app.log_file_pipe, 0);
	
	if ((r = uv_pipe_open(&g_app.log_file_pipe, log_file_fd))){
		fprintf(stderr, "Listen error %s\n", uv_err_name(r));
		return 3;
	}
	
	return uv_run(g_app.loop, UV_RUN_DEFAULT);
}
