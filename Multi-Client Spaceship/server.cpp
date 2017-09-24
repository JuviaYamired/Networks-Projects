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

  #define HEADER_SIZE 5
  #define A_MOV_SIZE 5

  vector<int> clients;
  vector<vector<unsigned int> > game_matrix;
  vector<string> playersLastPosition; //Keep track of players position to be send to new players
  int ctrNextPlayerId = 1;

  string intToStr (int x)
  {
      stringstream str;
      str << x;
      return str.str();
  }

  inline unsigned int getSize(){
  unsigned int tam = game_matrix.size();
  return 1 + tam + ((tam - 1) * tam) + (tam * tam) + 1;
}

string OKson(){
  string matrix;
  for(unsigned int i = 0; i < game_matrix.size(); i++){
    for(unsigned int j = 0; j < game_matrix[i].size(); j++){
      matrix += intToStr(game_matrix[i][j]) + ",";
    }
    matrix.pop_back();
    matrix += ";";
  }
  //matrix += ";";
  return matrix;
}

void updateMatrix(char protocol[3]){
  unsigned int i = protocol[0] - '0';
  unsigned int j = protocol[2] - '0';
  game_matrix[i][j]++;
}

void updateMatrixKilled(char endP){
  unsigned int i = endP - '0' - 1;
  game_matrix.erase(game_matrix.begin()+i);
  for(unsigned int j = 0; j < game_matrix.size(); j++){
    game_matrix[j].erase(game_matrix[j].begin()+i);
  }

  for(unsigned int i = 0; i < game_matrix.size(); i++){
    for(unsigned int j = 0; j < game_matrix.size(); j++){
      cout<<game_matrix[i][j]<<" ";
    }
    cout<<endl;
  }
}

string buildMatrixProtocol(){
  string matriz_formato = OKson();
  string strProtocol = intToStr(matriz_formato.size());
  while(strProtocol.size() < 3){
    strProtocol = "0" + strProtocol;
  }

  strProtocol = "1x" + strProtocol + matriz_formato;
  return strProtocol;
}

  void bot(int clientSD)
  {
    char* protocol;
    int dynMessageSize;
    string dySizeStr, playerId, ptcAction, joinProtocol, strProtocol;
    int holdPlayerNumber = ctrNextPlayerId;
    int n;

    protocol = new char[HEADER_SIZE + A_MOV_SIZE];
    // First comunication to set client's player id
    n = read(clientSD,protocol,HEADER_SIZE + A_MOV_SIZE);
    if (n < 0) perror("ERROR writing to socket");

    if( protocol[0] == '0') {
      playerId = intToStr(ctrNextPlayerId);
      protocol[0] = playerId[0];
      protocol[1] = 'm';
      protocol[4] = '5';
      ctrNextPlayerId++;
      printf("New Player, Sending Initial Protocol %s\n", protocol);
      playersLastPosition.push_back(protocol);

      //Broadcasting the new player protocol
      for (int i= 0; i < clients.size(); i++){
        n = write(clients[i],protocol,HEADER_SIZE + 5);
        if (n < 0) perror("ERROR writing to socket");
      }

      //Sending the new player the position of the other players
      if(clients.size() > 1){
        for (int i= 0; i < playersLastPosition.size();i++){
            n = write(clientSD,playersLastPosition[i].data(),HEADER_SIZE + 5);
            if (n < 0) perror("ERROR writing to socket");
        }
      }


    }

    //Communication between client - server
    while(true){
      delete[] protocol;
      protocol = new char[HEADER_SIZE];
      n = read(clientSD,protocol,HEADER_SIZE);
      if (n < 0) perror("ERROR writing to socket");
      joinProtocol = protocol;
      playerId=   protocol[0];
      ptcAction=  protocol[1];
      dySizeStr=  protocol[2];
      dySizeStr+= protocol[3];
      dySizeStr+= protocol[4];
      dynMessageSize =  atoi(dySizeStr.c_str());

      delete[] protocol;
      protocol = new char[dynMessageSize];

      n = read(clientSD,protocol,dynMessageSize);
      if (n < 0) perror("ERROR writing to socket");
      joinProtocol += protocol;

      cout<<"Protocol Ready :"<< joinProtocol <<endl;

      if(ptcAction == "m"){
        playersLastPosition[atoi(playerId.c_str())-1] = joinProtocol;
      }
      else if(ptcAction == "h" || ptcAction == "k"){
        strProtocol = buildMatrixProtocol();
        cout<<"MATRIX : "<<strProtocol<<endl;
        n = write(clientSD, strProtocol.data(), strProtocol.size());
        if (n < 0) perror("ERROR writing to socket");
  	    //n = write(clients[i],matrix_protocol,strProtocol.size());
  	  }
      //Broadcasting the update of the player protocol
      for (int i=0;i<clients.size();i++){
          cout<<"Sending"<<endl;
          n = write(clients[i],joinProtocol.data(),HEADER_SIZE + dynMessageSize);
          if (n < 0) perror("ERROR writing to socket");
      }
    }
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
