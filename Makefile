CXX = g++-6
CXXFLAGS = -g -std=c++14 -lm -Wl,--warn-common,--fatal-warnings 
EXEC = asm
OBJECTS = asm.o scanner.o
DEPENDS = ${OBJECTS:.o=.d}

${EXEC}: ${OBJECTS}
	${CXX} ${CXXFLAGS} ${OBJECTS} -o ${EXEC}

-include ${DEPENDS}

clean:
	rm ${OBJECTS} ${EXEC} ${DEPENDS}
.PHONY: clean
