CXX = g++
AR = ar
LD = g++
WINDRES = windres

INC = -I../engine -I../dependencies/glm/include
CFLAGS = -Wall -std=c++20 -fexceptions
RCFLAGS = 
RESINC = 
LIBDIR = 
LIB = -lengine
LDFLAGS = 

SRC = $(wildcard *.cpp)
OBJ_NAMES = $(patsubst %.cpp,%.o,$(SRC))

# ------------------------
# TEST Configuaration
# ------------------------

INC_TEST = $(INC)
CFLAGS_TEST = $(CFLAGS) -g -D_TEST
RESINC_TEST = $(RESINC)
RCFLAGS_TEST = $(RCFLAGS)
LIBDIR_TEST = $(LIBDIR) -L../engine/bin/Release
LIB_TEST = $(LIB) -lglut -lGLU -lGL
LDFLAGS_TEST = $(LDFLAGS)
OBJDIR_TEST = obj/Test
DEP_TEST =
OUT_TEST = bin/Test/test_runner

OBJ_TEST = $(addprefix $(OBJDIR_TEST)/,$(OBJ_NAMES))

# ------------------------
# BUILD Target
# ------------------------

all: test

clean: clean_test

# ------------------------
# TEST Target
# ------------------------

before_test:
	test -d bin/Test || mkdir -p bin/Test
	test -d $(OBJDIR_TEST) || mkdir -p $(OBJDIR_TEST)

after_test:

test: before_test out_test run_test after_test

out_test: $(OBJ_TEST) $(DEP_TEST)
	$(LD) $(LIBDIR_TEST) $(OBJ_TEST) -o $(OUT_TEST) $(LDFLAGS_TEST) $(LIB_TEST)

$(OBJDIR_TEST)/%.o: %.cpp
	$(CXX) $(CFLAGS_TEST) $(INC_TEST) -c $< -o $@

run_test: out_test
	@echo "Execution of test..."
	LD_LIBRARY_PATH=../engine/bin/Release ./$(OUT_TEST)
	@echo "Test done. Return: $$?"

clean_test:
	rm -f $(OBJ_TEST) $(OUT_TEST)
	rm -rf bin/Test
	rm -rf $(OBJDIR_TEST)

.PHONY: all clean \
	test before_test after_test clean_test out_test run_test
