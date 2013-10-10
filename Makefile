parser: interpreter.o interpreter_main.o parser.o tokenizer.o garbage.o
	clang -g interpreter.o interpreter_main.o parser.o tokenizer.o garbage.o -o interpreter

garbage.o: garbage.c
	clang -c -g garbage.c

interpreter_main.o: interpreter_main.c
	clang -c -g interpreter_main.c

interpreter.o: interpreter.c
	clang -c -g interpreter.c

parser.o: parser.c
	clang -c -g parser.c

tokenizer.o: tokenizer.c
	clang -c -g tokenizer.c

tokenizer.c: tokenizer.l
	flex -o tokenizer.c tokenizer.l

clean: 
	rm -rf *.o interpreter