compileProducer:
	gcc -c producer.c
	gcc producer.o -o producer

compileConsumer:
	gcc -c consumer.c
	gcc consumer.o -o consumer

run:
	./producer fifo 1 file1 60
	./consumer fifo consumerOutput 60