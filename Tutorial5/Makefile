CC = mpicc
CFLAGS = -O2 -Wall

SRCDIR = src
BINDIR = bin

TARGETS = $(BINDIR)/problem1_hello \
          $(BINDIR)/problem2_blocking \
          $(BINDIR)/problem3_nonblocking \
          $(BINDIR)/problem4_ring \
          $(BINDIR)/problem5_grid \
          $(BINDIR)/problem5_cartesian

all: $(BINDIR) $(TARGETS)

$(BINDIR):
	mkdir -p $(BINDIR)

$(BINDIR)/problem1_hello: $(SRCDIR)/problem1_hello.c
	$(CC) $(CFLAGS) $< -o $@

$(BINDIR)/problem2_blocking: $(SRCDIR)/problem2_blocking.c
	$(CC) $(CFLAGS) $< -o $@

$(BINDIR)/problem3_nonblocking: $(SRCDIR)/problem3_nonblocking.c
	$(CC) $(CFLAGS) $< -o $@

$(BINDIR)/problem4_ring: $(SRCDIR)/problem4_ring.c
	$(CC) $(CFLAGS) $< -o $@

$(BINDIR)/problem5_grid: $(SRCDIR)/problem5_grid.c
	$(CC) $(CFLAGS) $< -o $@

$(BINDIR)/problem5_cartesian: $(SRCDIR)/problem5_cartesian.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(BINDIR)

.PHONY: all clean
