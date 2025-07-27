CC = g++-13
CFLAGS = -g -MMD -MP
LDFLAGS = -Isrc/parser -Isrc/core
TARGET_NAME = dbms
OUTDIR = out
TARGET = $(OUTDIR)/$(TARGET_NAME)
SRCS = $(shell find src -type f \( -name '*.c' -o -name '*.cpp' \))
OBJS = $(OUTDIR)/lex.yy.o $(patsubst src/%.cpp, $(OUTDIR)/%.o, $(filter %.cpp, $(SRCS))) \
				$(patsubst src/%.c, $(OUTDIR)/%.o, $(filter %.c, $(SRCS)))
DEPS = $(OBJS:.o=.d)

$(TARGET): $(OBJS) 
	$(CC) $(CFLAGS) -o $@ $^ -lfl

$(OUTDIR)/%.o: src/%.c | $(OUTDIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LDFLAGS) -c -o $@ $<

$(OUTDIR)/%.o: src/%.cpp | $(OUTDIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LDFLAGS) -c -o $@ $<

src/lex.yy.c: src/parser/parser.l
	lex -o $@ $<

$(OUTDIR):
	mkdir $@

.PHONY: clean run re

clean:
	rm -f $(TARGET) lex.yy.c $(OBJS) $(DEPS)

run: $(TARGET)
	./$(TARGET)

re: clean $(TARGET)

-include $(DEPS)
