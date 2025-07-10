Name=webserv
CXX=c++ -g -O0   #-fsanitize=address 
CXXFLAGS=-Wall -Wextra -Werror -std=c++98  

SRCS= webserv.cpp src/HttpServer.cpp  src/Lexer.cpp  src/Parser.cpp\
	  src/Connection.cpp src/Request.cpp src/Response.cpp\
	  src/GetMethodResponse.cpp src/ProcessRequest.cpp src/Post.cpp src/MultiPart.cpp\
	  src/CGI.cpp

OBJS=$(SRCS:.cpp=.o)

RM=rm -f

all: $(Name)

$(Name): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)
	@echo "Compilation complete. Executable: $@"
	@echo "Run the server with: ./$(Name) <config_file>"

%.o: %.cpp
	$(CXX) -I include $(CXXFLAGS) -c $< -o $@
	@echo "Compiling $< to $@"
	@echo "Object file created: $@"

clean:
	$(RM) $(OBJS)
	@echo "Cleaned up object files."

fclean: clean
	$(RM) $(Name)
	@echo "Cleaned up executable."

re: fclean all
	@echo "Recompiled everything."
.PHONY: all clean fclean re

# To use this Makefile, place it in the same directory as your source files.