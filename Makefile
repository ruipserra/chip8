CFLAGS += -std=c11
CFLAGS += -Wall -Wextra -Werror -pedantic

SRCDIR = ./src
TESTSDIR = ./tests
BUILDDIR = ./build

ROM ?= ./rocket.ch8

EXECUTABLE = $(BUILDDIR)/chip8
OBJS = $(addprefix $(BUILDDIR)/, \
	main.o \
	chip8.o \
	miniterm.o \
)

TEST_RUNNER = $(BUILDDIR)/tests
TEST_OBJS = $(filter-out ./build/main.o, $(OBJS)) $(addprefix $(BUILDDIR)/, \
	test_runner.o \
)

LINK = $(CC) $(LDFLAGS) -o $@ $^
COMPILE = @mkdir -p $(BUILDDIR); \
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all
all: $(EXECUTABLE) $(TEST_RUNNER) test

$(EXECUTABLE): $(OBJS)
	$(LINK)

$(TEST_RUNNER): $(TEST_OBJS)
	$(LINK)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(COMPILE)

$(BUILDDIR)/%.o: $(TESTSDIR)/%.c
	$(COMPILE)

.PHONY: run
run: $(EXECUTABLE)
	@$(EXECUTABLE) $(ROM)

.PHONY: test
test: $(TEST_RUNNER)
	@$(TEST_RUNNER)

.PHONY: clean
clean:
	rm -rf $(BUILDDIR)

.PHONY: format
format:
	find . -type f -regex ".*\.[ch]$$" -exec clang-format -style=Google -i {} \;
