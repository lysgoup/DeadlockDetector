comp:
	gcc -g -o target target.c -pthread
	gcc -g -o target2 target2.c -pthread
	gcc -o ddchck ddchck.c -DDEBUG -pthread
	gcc -g -shared -fPIC -o ddmon.so ddmon.c -ldl

test:
	gcc -o test_ddchck test_ddchck.c -DDEBUG -pthread

run:
	LD_PRELOAD="./ddmon.so" ./target

rund:
	LD_PRELOAD="./ddmon.so" ./target -d

run2:
	LD_PRELOAD="./ddmon.so" ./target2

clean:
	rm target ddmon.so
