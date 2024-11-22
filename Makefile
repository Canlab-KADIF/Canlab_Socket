CXX             = g++ 
SRCS            = main.cpp 
OBJS            = main.o 
TARGET          = client 
LIBS            = -lpthread
INC             = -I./include
 
all : $(TARGET)
	$(CXX) -o $(TARGET) $(OBJS) $(INC) $(LIBS)
 
$(TARGET) :
	$(CXX) -c $(SRCS) $(INC) $(LIBS)
 
clean :
	rm -f $(TARGET)
	rm -f *.o

