#include <stdint.h>


struct rudp_socket_t {
    
};

struct rudp_options_t {
    
};


struct rudp_socket_t* rudp_socket(struct rudp_options_t* options);

int32_t rudp_recv(struct rudp_socket_t* socket);
int32_t rudp_send(struct rudp_socket_t* socket);
int32_t rudp_close(struct rudp_socket_t* socket);

int32_t rudp_connect(struct rudp_socket_t* socket);

int32_t rudp_bind(struct rudp_socket_t* socket);
int32_t rudp_listen(struct rudp_socket_t* socket);
struct rudp_socket_t* rudp_accept(struct rudp_socket_t* socket);
