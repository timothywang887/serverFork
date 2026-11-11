a.out: main.cxx session.cxx protocol.cxx headers/session.H headers/protocol.H headers/comm.H headers/clientdata.H
	g++ main.cxx session.cxx protocol.cxx -I/usr/include/GameNetworkingSockets /usr/lib/x86_64-linux-gnu/libGameNetworkingSockets.so -Wall -Wextra -Wpedantic 
	echo "done."
fuck: connector.cpp headers/session.H headers/protocol.H headers/comm.H headers/clientdata.H
	g++ connector.cpp -I/usr/include/GameNetworkingSockets /usr/lib/x86_64-linux-gnu/libGameNetworkingSockets.so -Wall -Wextra -Wpedantic /usr/lib/x86_64-linux-gnu/libmysqlcppconn.so
	echo "complete - now running a out"
	./a.out
