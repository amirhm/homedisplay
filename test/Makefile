INCLUDES=-I../../ -I../pico-sdk/src/boards/include/boards/ -I../src 



# -I../pico-sdk/src/common/pico_base/include

src=test.c ../src/display.c ../src/Ubuntu.c 
all:
	gcc $(INCLUDES) $(src) -lm -DTEST -g
	./a.out
