
typedef struct {
int socket;
char active;
} Thread_handler;

int init_server (int port, int max_connections);

int accept_connections (int socket);

int close_connection (int handler);

int open_TCP_socket ();

int bind_socket (int socket, int port);

int set_queue_length (int socket, int length);

void *thread_routine (void *arg);