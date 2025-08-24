NAME = webserv

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -O3 -g3 -fsanitize=address
INCLUDES = -I include

SRCS = webserv.cpp \
       src/HttpServer.cpp src/Lexer.cpp src/Parser.cpp \
       src/Connection.cpp src/Request.cpp src/Response.cpp src/Error.cpp src/Get.cpp \
       src/MainResponse.cpp src/ProcessRequest.cpp src/Post.cpp src/MultiPart.cpp \
       src/CGI.cpp src/Delete.cpp src/EPost.cpp src/ConfigValidator.cpp src/Chunked.cpp

HDRS = include/CGI.hpp include/ConfigValidator.hpp include/Exceptions.hpp include/Lexer.hpp \
       include/Post.hpp include/Response.hpp include/ConfigData.hpp include/Connection.hpp include/HttpServer.hpp \
       include/MainResponse.hpp include/Parser.hpp include/Request.hpp webserv.hpp

OBJS = $(SRCS:.cpp=.o)

RM = rm -rf

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)
	@echo "Compilation complete. Executable: $@"
	@echo "Run the server with: ./$(NAME) <config_file>"

src/%.o: src/%.cpp $(HDRS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
	@echo "Compiled $<"

webserv.o: webserv.cpp $(HDRS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(RM) $(OBJS) webserv.o
	@echo "Cleaned object files."

fclean: clean
	$(RM) $(NAME)
	@echo "Removed executable."

re: fclean all

.PHONY: all clean fclean re