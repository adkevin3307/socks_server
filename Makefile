EXE = socks_server hw4.cgi
OBJ_DIR = obj

SOURCES_SOCKS = $(wildcard src/*.cpp)
SOURCES_CGI = $(wildcard src/console_cgi/*.cpp)

OBJS_SOCKS = $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(basename $(notdir $(SOURCES_SOCKS)))))
OBJS_CGI = $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(basename $(notdir $(SOURCES_CGI)))))

CXXFLAGS = -std=c++17 -I./include -Wall -O2

LIBS = -lpthread -lboost_system -lboost_filesystem

all: create_object_directory $(EXE)
	@echo Compile Success

create_object_directory:
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: src/console_cgi/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

socks_server: $(OBJS_SOCKS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

hw4.cgi: $(OBJS_CGI)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -rf $(EXE) $(OBJ_DIR)

