# good breakdown: http://cs107e.github.io/guides/make/
CFLAGS = -O10 -Wall -Werror -std=gnu99
PROGS = 0-timer.o 1-timer.o 1-timer-fix.o 2-wait.o 3-fb.o 4-fb.o 1-timer-fix.real.o 5-barrier.o

all : $(PROGS)

%.o: %.c
	arm-none-eabi-gcc $(CFLAGS) -c $^ -o $@
	arm-none-eabi-objdump -d $@ > $@.dis

clean:
	rm -f  *.dis *.o *~ *.s
