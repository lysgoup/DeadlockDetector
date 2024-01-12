comp:
	gcc -g -o target target.c -pthread
	gcc -o ddchck ddchck.c -pthread
	gcc -g -shared -fPIC -o ddmon.so ddmon.c -ldl

test:
	gcc -o test_ddchck test_ddchck.c -DDEBUG -pthread

run:
	LD_PRELOAD="./ddmon.so" ./target

rund:
	LD_PRELOAD="./ddmon.so" ./target -d

clean:
	rm target ddmon.so
