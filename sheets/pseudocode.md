### processClientsRoundRobin()
```
FUNCTION processClientsRoundRobin():
    // Calculate number of clients to process in this cycle
    SET clients_to_process = MIN(active_clients.size(), MAX_CLIENTS_PER_CYCLE)
    SET operations_performed = 0
    
    FOR i = 0 TO clients_to_process - 1:
        // Get next client in round-robin order
        SET current_client_index = (current_client_index + 1) % active_clients.size()
        SET client_fd = active_clients[current_client_index]
        
        // Skip clients that have been removed
        IF client_fd not in client_connections:
            REMOVE client_fd from active_clients
            DECREMENT i and clients_to_process
            CONTINUE
        
        GET connection from client_connections
        
        // Process according to client state with operation limits
        IF connection.state is READING_HEADERS or READING_BODY:
            SET operations = handleClientDataWithLimit(client_fd, MAX_OPERATIONS_PER_CLIENT)
        ELSE IF connection.state is SENDING_RESPONSE:
            SET operations = handleClientWriteWithLimit(client_fd, MAX_OPERATIONS_PER_CLIENT)
        ELSE:
            // Client in PROCESSING or COMPLETE state
            SET operations = 1
            processRequest(connection)
        
        INCREMENT operations_performed by operations
        
        // Check if client should be removed from active list
        IF connection.state is COMPLETE:
            REMOVE client_fd from active_clients
        
        // Prevent monopolization by checking operation quota
        IF operations_performed >= MAX_OPERATIONS_PER_CYCLE:
            BREAK
```# HTTP Server Pseudocode

## Classes and Data Structures

### HttpServerError
- Class that extends standard error class for server-specific exceptions
- Constructor takes an error message string

### ClientConnection
- Class to manage individual client connections
- Properties:
  - fd: file descriptor for the client socket
  - state: enum {READING_HEADERS, READING_BODY, PROCESSING, SENDING_RESPONSE, COMPLETE}
  - request: pointer to HttpRequest object
  - response: pointer to HttpResponse object
  - buffer: string to store incoming data
  - bytes_sent: number of bytes sent in response
  - last_activity: timestamp of last activity
  - operation_quota: remaining operations allowed in current cycle
- Constructor takes client socket file descriptor
- Destructor cleans up request and response objects

### HttpServer
- Main server class
- Properties:
  - servers: reference to vector of Server configurations
  - epoll_fd: file descriptor for epoll instance
  - events: array to store epoll events
  - server_map: mapping of server socket FDs to Server objects
  - client_connections: mapping of client socket FDs to ClientConnection objects
  - active_clients: queue of client FDs for round-robin processing
  - max_per_client_cycle: maximum operations per client in one cycle
  - current_client_index: index of current client being processed
- Constructor takes vector of Server configurations
- Destructor cleans up resources

## HttpServer Methods

### init()
```
FUNCTION init():
    // Initialize round-robin variables
    SET current_client_index = -1
    SET MAX_CLIENTS_PER_CYCLE = 100
    SET MAX_OPERATIONS_PER_CLIENT = 10
    SET MAX_OPERATIONS_PER_CYCLE = 1000
    SET MAX_WRITE_CHUNK_SIZE = 4096
    
    FOR EACH server IN servers:
        FOR EACH listen_address IN server.listen:
            SET host = listen_address.host OR empty for all interfaces
            SET port = listen_address.port
            
            CREATE address info with hints for TCP socket
            GET address info for host:port
            
            FOR EACH address IN address_info:
                CREATE socket
                IF socket creation failed:
                    CONTINUE to next address
                
                SET socket to reuse address
                SET socket to non-blocking mode
                
                BIND socket to address
                IF bind successful:
                    BREAK loop
                ELSE:
                    CLOSE socket
                    CONTINUE to next address
            
            IF no successful bind:
                THROW error "Socket binding failed"
            
            START listening on socket
            
            ADD socket to epoll with EPOLLIN | EPOLLET events
            STORE mapping from socket to server in server_map
```

### setNonBlocking()
```
FUNCTION setNonBlocking(sockfd):
    GET current socket flags
    IF failed:
        CLOSE socket
        THROW error "Failed to get socket flags"
    
    SET socket flags to include O_NONBLOCK
    IF failed:
        CLOSE socket
        THROW error "Failed to set socket non-blocking"
```

### handleNewConnection()
```
FUNCTION handleNewConnection(server_fd):
    SET connections_accepted = 0
    SET MAX_CONNECTIONS_PER_CYCLE = 20
    
    WHILE connections_accepted < MAX_CONNECTIONS_PER_CYCLE:
        ACCEPT new connection from server_fd
        IF accept failed:
            IF error is EAGAIN or EWOULDBLOCK:
                BREAK loop (no more connections pending)
            ELSE:
                THROW error "Accept failed"
        
        INCREMENT connections_accepted
        SET new connection to non-blocking mode
        
        CREATE new ClientConnection object for client_fd
        STORE in client_connections map
        
        ADD client_fd to epoll with EPOLLIN | EPOLLET events
        ADD client_fd to active_clients queue
```

### handleClientDataWithLimit()
```
FUNCTION handleClientDataWithLimit(client_fd, max_operations):
    GET connection from client_connections map
    IF connection not found:
        RETURN 0
    
    UPDATE last_activity timestamp
    SET operations_performed = 0
    
    WHILE operations_performed < max_operations:
        READ data into buffer
        IF read failed:
            IF error is EAGAIN or EWOULDBLOCK:
                BREAK (no more data available now)
            ELSE:
                CLEAN UP client (error occurred)
                RETURN operations_performed
        
        IF read returned 0:
            CLEAN UP client (connection closed)
            RETURN operations_performed
            
        INCREMENT operations_performed
        
        APPEND data to connection buffer
        
        IF buffer size > MAX_REQUEST_SIZE:
            CREATE 413 Entity Too Large response
            SET connection state to SENDING_RESPONSE
            UPDATE epoll to wait for EPOLLOUT
            RETURN operations_performed
        
        IF connection state is READING_HEADERS:
            FIND end of headers marker "\r\n\r\n"
            IF found:
                PARSE headers from buffer
                IF parsing failed:
                    CREATE 400 Bad Request response
                    SET connection state to SENDING_RESPONSE
                    UPDATE epoll to wait for EPOLLOUT
                    RETURN operations_performed
                
                REMOVE headers from buffer
                
                IF request expects body:
                    SET connection state to READING_BODY
                ELSE:
                    SET connection state to PROCESSING
                    RETURN operations_performed
        
        IF connection state is READING_BODY:
            GET expected content length
            IF buffer size >= content length:
                SET request body from buffer
                REMOVE body data from buffer
                SET connection state to PROCESSING
                RETURN operations_performed
    
    RETURN operations_performed
```

### handleClientWriteWithLimit()
```
FUNCTION handleClientWriteWithLimit(client_fd, max_operations):
    GET connection from client_connections map
    IF connection not found OR state is not SENDING_RESPONSE:
        RETURN 0
    
    UPDATE last_activity timestamp
    SET operations_performed = 0
    
    WHILE operations_performed < max_operations:
        GET response data and calculate remaining bytes
        
        IF remaining == 0:
            BREAK
        
        WRITE chunk of remaining data to socket (limited by MAX_WRITE_CHUNK_SIZE)
        
        IF write failed:
            IF error is EAGAIN or EWOULDBLOCK:
                BREAK (try again later)
            ELSE:
                CLEAN UP client (error occurred)
                RETURN operations_performed
        
        INCREMENT operations_performed
        UPDATE bytes_sent counter
        
        IF all data sent:
            IF request indicates keep-alive:
                RESET connection for next request:
                    - Create new HttpRequest
                    - Delete old response
                    - Reset state to READING_HEADERS
                    - Reset bytes_sent counter
                UPDATE epoll to wait for EPOLLIN
            ELSE:
                CLEAN UP client (close connection)
            BREAK
    
    RETURN operations_performed
```

### processRequest()
```
FUNCTION processRequest(connection):
    FIND appropriate server config:
        GET Host header from request
        FOR EACH server in servers:
            IF Host matches server_name:
                SET server = matched server
                BREAK
        
        IF no server found AND servers not empty:
            SET server = first server (default)
    
    CREATE HttpResponse based on request and server config
    SET connection state to SENDING_RESPONSE
    
    UPDATE epoll to wait for EPOLLOUT on client socket
```

### checkTimeouts()
```
FUNCTION checkTimeouts():
    GET current time
    CREATE list for connections to remove
    
    FOR EACH connection in client_connections:
        IF current_time - connection.last_activity > TIMEOUT_SECONDS:
            ADD connection to remove list
    
    FOR EACH fd in remove list:
        CLEAN UP client fd
        REMOVE fd from active_clients if present
```

### cleanupClient()
```
FUNCTION cleanupClient(fd):
    FIND connection in client_connections map
    IF found:
        REMOVE fd from epoll
        CLOSE socket
        DELETE connection object
        REMOVE from client_connections map
        REMOVE fd from active_clients if present
```

### run()
```
FUNCTION run():
    SET last_timeout_check = current time
    
    WHILE true:
        WAIT for events with 1 second timeout
        
        IF wait failed:
            IF error is EINTR:
                CONTINUE (interrupted by signal)
            THROW error "Epoll wait failed"
        
        // Update active_clients queue with new clients from events
        FOR EACH event:
            GET file descriptor
            
            IF fd is in server_map:
                HANDLE new connection
            ELSE IF event has EPOLLIN or EPOLLOUT flags:
                IF fd is not in active_clients:
                    ADD fd to active_clients
            
            IF event has error or hangup flags:
                IF fd is not in server_map:
                    CLEAN UP client
                    REMOVE fd from active_clients if present
        
        // Process clients in round-robin fashion
        IF active_clients not empty:
            processClientsRoundRobin()
        
        GET current time
        IF current_time - last_timeout_check > 5:
            CHECK for timeouts
            UPDATE last_timeout_check
```

### cleanup()
```
FUNCTION cleanup():
    FOR EACH client in client_connections:
        CLOSE socket
        DELETE connection object
    CLEAR client_connections map
    
    FOR EACH server socket in server_map:
        CLOSE socket
    CLEAR server_map
```

## HttpRequest (Simplified)

### Properties
- method: HTTP method (GET, POST, etc.)
- uri: Request URI
- http_version: HTTP version
- headers: Map of header names to values
- body: Request body data
- is_complete: Flag indicating if request is complete

### Methods
- parseHeaders(header_data): Parse HTTP headers
- expectsBody(): Check if request expects a body
- getContentLength(): Get Content-Length value
- setBody(body_data): Set request body
- getHeader(name): Get header value
- keepAlive(): Check if connection should be kept alive

## HttpResponse (Simplified)

### Properties
- status_code: HTTP status code
- headers: Map of header names to values
- body: Response body
- response_data: Complete formatted response

### Methods
- Constructor(code): Create response with status code
- Constructor(request, server): Create response based on request and server
- buildResponse(): Build formatted response string
- getStatusText(code): Get text description for status code
- getData(): Get complete response data
