CXX=gcc
CFLAGS=-Wall
CXXFLAGS=
LDFLAGS=

batcheck : batcheck.c
	$(CXX) $(CFLAGS) $(CXXFLAGS) -o $@ $< $(LDFLAGS)
