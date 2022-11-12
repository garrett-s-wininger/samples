RS_SRC_DIR = example/src
RS_OUTPUT_DIR = example/target/debug

all: main

main: main.cpp $(RS_OUTPUT_DIR)/libexample.a
	g++ main.cpp -L$(RS_OUTPUT_DIR) -lexample -ldl -lpthread -o $@

$(RS_OUTPUT_DIR)/libexample.a : $(RS_SRC_DIR)/lib.rs
	cd $(RS_SRC_DIR) && cargo build

clean:
	test -z "main" || rm -f main
	rm -f *.o
	rm -f *.a
	cd $(RS_SRC_DIR) && cargo clean
