# Makefile for sdu
PROJECT_NAME = sdu

# macros
COMPILING_FLAG = $(CXX_ADDITION_FLAGS)
LINK_FLAG = -r

# project directory
OBJ_DIR =  $(OBJ_ROOT)$(PROJECT_NAME)
TARGET_DIR = ../lib
TARGET = $(TARGET_DIR)/lib$(PROJECT_NAME).a

SRC_DIR = src
SRC = $(wildcard $(SRC_DIR)/*.cpp)
DEPS = $(addprefix $(OBJ_DIR)/, $(patsubst %cpp, %d, $(filter %cpp, $(notdir $(SRC)))))
OBJS = $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(basename $(notdir $(SRC)))))

# includ and lib directory
INCLUDE_DIRS = ./include
LIBARAY_DIRS =
LIBARAYS = 
All_INCLUDE_DIRS = $(addprefix -I,$(INCLUDE_DIRS))
ALL_LIBARAY_DIRS = $(addprefix -L,$(LIBARAY_DIRS))
ALL_LIBARAYS = $(addprefix -l,$(LIBARAYS))

# rule all
all : $(TARGET)
	@echo build $(PROJECT_NAME) End!

# for target
$(TARGET) : $(OBJS)
	@echo linking $(TARGET)
	@$(AR) $(LINK_FLAG) -o $@ $(OBJS) $(ALL_LIBARAY_DIRS) $(ALL_LIBARAYS)

# for .o
$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(TARGET_DIR)
	@echo building $<
	@$(CXX) $(COMPILING_FLAG) $(All_INCLUDE_DIRS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c $< -o $@

# clean
clean:
	@echo clean $(PROJECT_NAME)
	@rm -f $(TARGET) $(OBJS) $(DEPS)
.PHONY: clean

# include rules
-include $(DEPS)

# help
help:
	@echo $(DEPS)
.PHONY: help