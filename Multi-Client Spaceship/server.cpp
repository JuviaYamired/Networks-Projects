  /* Server code in C */
 
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <vector>
  #include <sstream>
  #include <thread>
  #include <string.h>
  #include <iostream>
  #include <unistd.h>
 
  using namespace std;

  #define MAX_ACTION 7
  vector<int> clients;
  vector<string> playersLastPosition;
  int nextNewPlayer = 1;

  string intToStr (int x)
  {
      stringstream str;
      str << x;
      return str.str();
  }

  
  void bot(int clientSD)
  {
    char protocol[MAX_ACTION];
    string intToString;
    string ProtocolString;
    char sendProtocol[MAX_ACTION];
    int holdPlayerNumber = nextNewPlayer;
    int n;
    n = read(clientSD,protocol,MAX_ACTION);
    if (n < 0) perror("ERROR reading from socket");  

    if(protocol[0] == '0'){          
      intToString = intToStr(nextNewPlayer);
      protocol[0] = intToString[0];
      protocol[1] = '0';
      protocol[2] = '0';
      protocol[3] = '0';
      protocol[4] = '0';
      protocol[5] = '0';
      protocol[6] = '0';
      nextNewPlayer++;
      printf("newUser %s\n", protocol);
      playersLastPosition.push_back(protocol);
    }

    /*if(clients.size() > 1)
    {
      for (int i=0; i < playersLastPosition.size();i++){
        if(i!=holdPlayerNumber+1){
          n = write(clientSD,playersLastPosition[i].data(),MAX_ACTION);
          if (n < 0) perror("ERROR writing to socket");
        }
      } 
    }*/

    for (int i=0;i<clients.size();i++){
      n = write(clients[i],protocol,MAX_ACTION);
      if (n < 0) perror("ERROR writing to socket");
    } 

    while(true){
      n = read(clientSD,protocol,MAX_ACTION);
      playersLastPosition[holdPlayerNumber-1] = protocol;
      if (n < 0) perror("ERROR reading from socket");
      for (int i=0;i<clients.size();i++){
          n = write(clients[i],protocol,MAX_ACTION);
          if (n < 0) perror("ERROR writing to socket");
      } 
    }
    shutdown(clientSD, SHUT_RDWR);
    close(clientSD);
  }

  int main()
  {
    struct sockaddr_in stSockAddr;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    int ConnectFD = 0;
 
    if(-1 == SocketFD)
    {
      perror("can not create socket");
      exit(EXIT_FAILURE);
    }
    
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
 
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(41001);
    stSockAddr.sin_addr.s_addr = INADDR_ANY;
 
    if(-1 == bind(SocketFD,(const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
    {
      perror("error bind failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
 
    if(-1 == listen(SocketFD, 10))
    {
      perror("error listen failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
 
    while(true)
    {
      ConnectFD = accept(SocketFD, NULL, NULL);

      if(0 > ConnectFD)
      {
        perror("error accept failed");
        close(SocketFD);
        exit(EXIT_FAILURE);
      }
    
      clients.push_back(ConnectFD);
      std::thread(bot,ConnectFD).detach();
    }
 
    close(SocketFD);
    return 0;
  }
