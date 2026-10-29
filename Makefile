# per-project
SFML = E:/libraries/SFML-3.1.0
DLLS = libs/sfml
LIBS = -lsfml-graphics -lsfml-window -lsfml-system

# generic
CXX      = g++
CXXFLAGS = -std=c++20 -Iinc -I$(SFML)/include -MMD -MP
SRCS     = $(wildcard src/*.cpp)
OBJS     = $(addprefix build/,$(notdir $(SRCS:.cpp=.o)))
vpath %.cpp $(dir $(SRCS))

.PHONY: run clean

main: $(OBJS)
	$(CXX) $(OBJS) -o main -L$(SFML)/lib $(LIBS)

build/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: main
	copy $(subst /,\,$(DLLS))\*.dll .
	main.exe
	-del $(notdir $(wildcard $(DLLS)/*.dll))

clean:
	-del /Q $(subst /,\,$(OBJS) $(OBJS:.o=.d)) main.exe

-include $(OBJS:.o=.d)