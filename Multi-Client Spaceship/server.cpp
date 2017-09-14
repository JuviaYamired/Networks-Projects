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
  #include <chrono>
 
  using namespace std;

  #define MAX_ACTION 7
  #define MIN_ACTION 5

  vector<int> clients;
  vector<string> playersLastPosition; //Keep track of players position for new players
  int ctrNextPlayerId = 1;

  string intToStr (int x)
  {
      stringstream str;
      str << x;
      return str.str();
  }

  
  void bot(int clientSD)
  {
    char* protocol;
    int dynMessageSize;
    string dySizeStr;
    string playerId;

    string joinProtocol;
    char sendProtocol[MAX_ACTION];
    int holdPlayerNumber = ctrNextPlayerId;
    int n;

    protocol = new char[MIN_ACTION];

    // First comunication to set client's player id
    n = read(clientSD,protocol,MIN_ACTION);
    if (n < 0) perror("ERROR writing to socket");  
    
    if( protocol[0] == '0') {
      playerId = intToStr(ctrNextPlayerId);
      protocol[0] = playerId[0];
      ctrNextPlayerId++;
      printf("New Player %s\n", protocol);
      playersLastPosition.push_back(protocol); 

      //Broadcasting the new player protocol   
      for (int i = 0; i < clients.size(); i++){
        n = write(clients[i],protocol,MIN_ACTION);
        if (n < 0) perror("ERROR writing to socket");
      } 
    }
    

    //Continous communication between client - server
    while(true){
      delete[] protocol;
      protocol = new char[MIN_ACTION];
      n = read(clientSD,protocol,MIN_ACTION);
      if (n < 0) perror("ERROR writing to socket");
      joinProtocol = protocol;

      cout<<"Joined Protocol :" <<joinProtocol<<endl;

      dySizeStr =  protocol[2];
      dySizeStr += protocol[3];
      dySizeStr += protocol[4];
      dynMessageSize =  atoi(dySizeStr.c_str());

      delete[] protocol;
      protocol = new char[dynMessageSize];

      n = read(clientSD,protocol,dynMessageSize);
      if (n < 0) perror("ERROR writing to socket");
      joinProtocol += protocol;

      cout<<"Protocol Ready :"<< joinProtocol <<endl;
      //playersLastPosition[holdPlayerNumber-1] = protocol;

      //Broadcasting the update of the player protocol  
      for (int i=0;i<clients.size();i++){
          cout<<"sending"<<endl;
          n = write(clients[i],joinProtocol.data(),MIN_ACTION + dynMessageSize);
          if (n < 0) perror("ERROR writing to socket");
      } 
    }

    /*
    cout<<"Primer Read"<<endl;
    n = read(clientSD,protocol,MIN_ACTION);
    if (n < 0) perror("ERROR reading from socket");

    dySizeStr =  protocol[2];
    dySizeStr += protocol[3];
    dySizeStr += protocol[4];
    dySize =  atoi(dySizeStr.c_str());

    printf("%s\n",protocol);
    delete[] protocol;
    protocol = new char[dySize];
    
    cout<<"Segundo Read"<<endl;
    n = read(clientSD,protocol,dySize);
    printf("%s\n", protocol );

    cout<<"Termino Protocolo"<<endl;
    */
    //std::this_thread::sleep_for (std::chrono::seconds(20));
    /*if(protocol[0] == '0'){          
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
    }*/

    /*if(clients.size() > 1)
    {
      for (int i=0; i < playersLastPosition.size();i++){
        if(i!=holdPlayerNumber+1){
          n = write(clientSD,playersLastPosition[i].data(),MAX_ACTION);
          if (n < 0) perror("ERROR writing to socket");
        }
      } 
    }*/

    /*for (int i=0;i<clients.size();i++){
      n = write(clients[i],protocol,MAX_ACTION);
      if (n < 0) perror("ERROR writing to socket");
    } */

    /*while(true){
      n = read(clientSD,protocol,MAX_ACTION);
      playersLastPosition[holdPlayerNumber-1] = protocol;
      if (n < 0) perror("ERROR reading from socket");
      for (int i=0;i<clients.size();i++){
          n = write(clients[i],protocol,MAX_ACTION);
          if (n < 0) perror("ERROR writing to socket");
      } 
    }*/
    shutdown(clientSD, SHUT_RDWR);
    close(clientSD);
    cout<<"shutdown"<<endl;
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
