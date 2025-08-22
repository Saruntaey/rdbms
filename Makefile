TARGET_NAME = dbms
OUTDIR = out
MATH_EXP_DIR = ../math_expression_parser
MATH_EXP_LIB_DIR = ../math_expression_parser/out
CC = g++
CFLAGS = -g -MMD -MP
LDFLAGS = -Isrc/parser -Isrc/core -Isrc/b_plus_tree_lib -I$(MATH_EXP_DIR)
TARGET = $(OUTDIR)/$(TARGET_NAME)
SRCS = $(shell find src -type f \( -name '*.c' -o -name '*.cpp' \))
OBJS = $(OUTDIR)/lex.yy.o $(patsubst src/%.cpp, $(OUTDIR)/%.o, $(filter %.cpp, $(SRCS))) \
				$(patsubst src/%.c, $(OUTDIR)/%.o, $(filter %.c, $(SRCS)))
DEPS = $(OBJS:.o=.d)

TEST_SRCS = $(shell find test -type f -name '*.c')
TEST_OBJS = $(patsubst %.c, $(OUTDIR)/%.o, $(TEST_SRCS))
TEST_DEPS = $(TEST_OBJS:.o=.d)

$(TARGET): $(OBJS) 
	$(CC) $(CFLAGS) -o $@ $^ -lfl -L$(MATH_EXP_LIB_DIR) -lmx

$(OUTDIR)/%.o: src/%.c | $(OUTDIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LDFLAGS) -c -o $@ $<

$(OUTDIR)/b_plus_tree_lib/%.o: src/b_plus_tree_lib/%.c | $(OUTDIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -fpermissive $(LDFLAGS) -c -o $@ $<

$(OUTDIR)/%.o: src/%.cpp | $(OUTDIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LDFLAGS) -c -o $@ $<

src/lex.yy.c: src/parser/parser.l
	lex -o $@ $<

$(OUTDIR):
	mkdir $@

$(OUTDIR)/test_exe: $(filter-out %main.o, $(OBJS)) $(TEST_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lfl -L$(MATH_EXP_LIB_DIR) -lmx

$(OUTDIR)/test/%.o: test/%.c | $(OUTDIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LDFLAGS) -c -o $@ $<

.PHONY: clean run re run_test

clean:
	rm -f $(TARGET) lex.yy.c $(OBJS) $(DEPS) $(TEST_OBJS) $(TEST_DEPS) $(OUTDIR)/test_exe

run: $(TARGET)
	./$(TARGET)

run_test: $(OUTDIR)/test_exe 
	./$(OUTDIR)/test_exe

re: clean $(TARGET)

-include $(DEPS)
