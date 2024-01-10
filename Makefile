comp:
	gcc -o target target.c -pthread
	gcc -o ddchck ddchck.c -pthread
	gcc -shared -fPIC -o ddmon.so ddmon.c -ldl

run:
	LD_PRELOAD="./ddmon.so" ./target

rund:
	LD_PRELOAD="./ddmon.so" ./target -d

clean:
	rm target ddmon.so
