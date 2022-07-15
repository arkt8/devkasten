EX=""

test:
	@echo $(EX)
	make clean
	make compile ex=$(EX)
	make luatest

clean:
	rm -rf ./out

compile:
	mkdir -p out
	gcc -c -fPIC -O2 ex_$(EX).c -o out/$(EX).o
	gcc -shared -lc -o out/$(EX).so out/$(EX).o

luatest:
	LUA_CPATH=./out/?.so lua ex_$(EX).lua
