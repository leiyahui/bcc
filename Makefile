objects =  test_main.o common.o lex.o load_file.o memory.o output_message.o

test_main : $(objects)
	cc -o test_main $(objects)
test_main.o : bcc.h 
	cc -c -o test_main.o test/test_main.c
lex.o : common.h lex.h load_file.h memory.h output_message.h bcc.h
common.o : common.h
load_file.o : load_file.h
memory.o : memory.h
output_message.o : output_message.h


clean:
	rm $(objects) test_main
