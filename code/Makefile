build:
	gcc process_generator.c -o process_generator
	gcc clk.c -o clk
	gcc scheduler.c -o scheduler -lm
	gcc process.c -o process
	./process_generator processes.txt -mem 2 -q 1 -sch 2


