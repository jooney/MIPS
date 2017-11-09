TARGET = MIPSsim 
#MUDUO = /home/jjz/muduotest/
#LDFLAGS = -pthread -lrt $(MUDUO)/base/libmuduo_base.a /home/jjz/Common/libZQCommon.so # $(MUDUO)/libmuduo_net.a  $(MUDUO)/libmuduo_base.a    
#INCLUDE = -I. -I$(MUDUO)/base -I/home/jjz/Common -I$(MUDUO)/poller -I$(MUDUO) 
CFLAGS = -g -std=c++11
OBJS = MIPSsim.o #Channel.o EventLoop.o Poller.o main.o DefaultPoller.o EPollPoller.o Thrd.o test.o
#VPATH = $(MUDUO):$(MUDUO)/poller
$(TARGET):$(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

$(OBJS):%.o:%.cpp
	$(CXX) -c $(INCLUDE) $(CFLAGS) $< -o $@
clean:
	-$(RM) $(TARGET)
	-$(RM) $(OBJS)
#	-$(RM) server.log
