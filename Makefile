Name=webserv
CXX=c++ #-g3 -fsanitize=address 
CXXFLAGS=-Wall -Wextra -Werror -std=c++98  

SRCS= webserv.cpp src/HttpServer.cpp  src/Lexer.cpp  src/Parser.cpp\
	  src/Connection.cpp src/Request.cpp src/Response.cpp src/Error.cpp src/Get.cpp\
	  src/MainResponse.cpp src/ProcessRequest.cpp src/Post.cpp src/CGI.cpp src/Delete.cpp\
	  src/EPost.cpp src/ConfigValidator.cpp src/Chunked.cpp src/MultiPart.cpp\

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
.PHONY: clean  