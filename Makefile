NAME		:= Shelving

OBJ_FOLDER	:= obj
SRC_FOLDER	:= src
BIN_FOLDER	:= bin

CPP_POSTFIX	:= cpp
OBJ_POSTFIX	:= o
DEP_POSTFIX	:= d
CPP_FILES	:= $(wildcard $(SRC_FOLDER)/*.$(CPP_POSTFIX))
OBJ_FILES	:= $(addprefix $(OBJ_FOLDER)/,$(notdir $(CPP_FILES:.$(CPP_POSTFIX)=.$(OBJ_POSTFIX))))

CXXFLAGS	+= -std=c++14 -Wall -Wextra -pedantic-errors
CXXFLAGS	+= -g
#CXXFLAGS	+= -O3

TARGET		:= $(BIN_FOLDER)/$(NAME)

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	$(CXX) $(LDFLAGS) -o $@ $^

$(OBJ_FOLDER)/%.$(OBJ_POSTFIX): $(SRC_FOLDER)/%.$(CPP_POSTFIX)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJ_FOLDER)/*

CXXFLAGS	+= -MMD
-include $(OBJ_FILES:.$(OBJ_POSTFIX)=.$(DEP_POSTFIX)