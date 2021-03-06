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

  //Size of the types of messages
  const unsigned int HEADER_SIZE= 2;       //[playerId][action]..
  const unsigned int A_MOV_SIZE= 5;        //..[x][x][y][y][direction]
  const unsigned int A_SHT_SIZE= 5;        //..[x][x][y][y][direction]
  const unsigned int A_CHT_SIZE= 3;        //..[size][of][message]
  const unsigned int A_HOK_SIZE= 1;        //..[playerId]

  //Posible actions
  const string A_MOVE=   "m";   //Move  Action
  const string A_SHOOT=  "s";   //Shoot Action
  const string A_HURT=   "h";   //Hurt  Action
  const string A_KILL=   "k";   //Kill  Action
  const string A_MATRIX= "x";   //Matrix  Action
  const string A_CHAT=   "c";   //Chat  Action
  const string keyShoot= "12345678"; //Shoot Directions

  //Keep a list of the clients socket descriptors
  vector<int> clients;
  //Matrix that store damage done to the players
  vector<vector<unsigned int> > game_matrix;
  //Keep track of players position to be send to new players
  vector<string> playersLastPosition;
  //Counter we use to assign a playerId to a new player
  int ctrNextPlayerId = 1;

  //Convert an integer to a string e.g., 1 => "1"
  string intToStr (int x)
  {
    stringstream str;
    str << x;
    return str.str();
  }

  //Build of the JSON matrix
  string OKson()
  {
    string matrix;
    for(unsigned int i = 0; i < game_matrix.size(); i++){
      for(unsigned int j = 0; j < game_matrix[i].size(); j++){
        matrix += intToStr(game_matrix[i][j]) + ",";
      }
      matrix.pop_back();
      matrix += ";";
    }
    matrix += ";";
    return matrix;
  }

  void updateMatrix(char protocol[3])
  {
    unsigned int i = protocol[0] - '0';
    unsigned int j = protocol[2] - '0';
    game_matrix[i][j]++;
  }

  //Build the protocol to send the matrix to the players
  string buildMatrixProtocol()
  {
    string matriz_formato = OKson();
    string strProtocol = intToStr(matriz_formato.size());
    while(strProtocol.size() < 3){
      strProtocol = "0" + strProtocol;
    }

    strProtocol = "1x" + strProtocol + matriz_formato;
    return strProtocol;
  }

  //Bot in charge of give new players a playerId and reply each of them with the protocol
  //send by all of the players
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

    //If new player sending playerId and Initial position
    if( protocol[0] == '0') {
      playerId = intToStr(ctrNextPlayerId);
      protocol[0] = playerId[0];
      protocol[1] = 'm';

      ctrNextPlayerId++;
      printf("New Player, Sending Initial Protocol %s\n", protocol);
      playersLastPosition.push_back(protocol);

      //Broadcasting the new player position
      for (int i= 0; i < clients.size(); i++){
        n = write(clients[i],protocol,HEADER_SIZE + A_MOV_SIZE);
        if (n < 0) perror("ERROR writing to socket");
      }

      //Sending the position of the other players to the new player
      if(clients.size() > 1){
        for (int i= 0; i < playersLastPosition.size();i++){
            n = write(clientSD,playersLastPosition[i].data(),HEADER_SIZE + A_MOV_SIZE);
            if (n < 0) perror("ERROR writing to socket");
        }
      }

    }

    //Communication between client - server
    while(true){
      //Reading the Header of the messages sent by the client
      delete[] protocol;
      protocol = new char[HEADER_SIZE];
      n = read(clientSD,protocol,HEADER_SIZE);
      if (n < 0) perror("ERROR writing to socket");

      //Storing the protocol sent by the client to do the broadcast later
      joinProtocol = protocol;

      //Storing the playerId who sent the message
      playerId=   protocol[0];

      //Storing the action that the player is going to do
      ptcAction=  protocol[1];

      //If the player moves
      if(ptcAction == A_MOVE){
        //Resize to retrieve message
        dynMessageSize = A_MOV_SIZE;
        delete[] protocol;
        protocol = new char[dynMessageSize];

        //Retrieving the message
        n = read(clientSD,protocol,dynMessageSize);
        if (n < 0) perror("ERROR writing to socket");

        //Joinning the full protocol to do the broadcast
        joinProtocol += protocol;
        cout<<"Protocol Ready :"<< joinProtocol <<endl;

        //We store the move protocol to let new players know where the other players are
        playersLastPosition[atoi(playerId.c_str())-1] = joinProtocol;
      }

      //if the player shoots
      else if(ptcAction == A_SHOOT){
        //Resize to retrieve message
        dynMessageSize = A_SHT_SIZE;
        delete[] protocol;
        protocol = new char[dynMessageSize];

        //Retrieving the message
        n = read(clientSD,protocol,dynMessageSize);
        if (n < 0) perror("ERROR writing to socket");

        //Joinning the full protocol to do the broadcast
        joinProtocol += protocol;
        cout<<"Protocol Ready :"<< joinProtocol <<endl;
      }

      //If the player chats
      else if(ptcAction == A_CHAT){
        //Resize to retrieve message
        dynMessageSize = A_CHT_SIZE;
        delete[] protocol;
        protocol = new char[dynMessageSize];

        //Retrieving the message
        n = read(clientSD,protocol,dynMessageSize);
        if (n < 0) perror("ERROR writing to socket");

        //Storing the size of the actual message
        dySizeStr=  protocol[0];
        dySizeStr+= protocol[1];
        dySizeStr+= protocol[2];
        dynMessageSize =  atoi(dySizeStr.c_str());
        joinProtocol+= protocol;

        //Resize to retrieve message
        delete[] protocol;
        protocol = new char[dynMessageSize];

        //Retrieving the message
        n = read(clientSD,protocol,dynMessageSize);
        if (n < 0) perror("ERROR writing to socket");

        //Joinning the full protocol to do the broadcast
        joinProtocol += protocol;
        cout<<"Protocol Ready :"<< joinProtocol <<endl;
      }

      //If a player is hurt or killed
      else if(ptcAction == "h" || ptcAction == "k"){
        //Resize to retrieve message
        dynMessageSize= A_HOK_SIZE;
        delete[] protocol;
        protocol = new char[dynMessageSize];

        //Retrieving the message
        n = read(clientSD, protocol, dynMessageSize);
        if (n < 0) perror("ERROR writing to socket");
	joinProtocol += protocol[0];
	  
        game_matrix[playerId[0] - '0' - 1][protocol[0] - '0' - 1]++;
        strProtocol = buildMatrixProtocol();
        cout<<"MATRIX : "<<strProtocol<<endl;
	for(int i = 0; i < clients.size(); i++){
        n = write(clients[i], strProtocol.data(), strProtocol.size());
        if (n < 0) perror("ERROR writing to socket");
  	    //n = write(clients[i],matrix_protocol,strProtocol.size());
  	  }
      }

      //Broadcasting the protocol to all the players
      for (int i=0;i<clients.size();i++){
          n = write(clients[i], joinProtocol.data(), joinProtocol.size());
          if (n < 0) perror("ERROR writing to socket");
      }
    }
    shutdown(clientSD, SHUT_RDWR);
    close(clientSD);
    cout<<"shutdown"<<endl;
  }

  int main()
  {
    vector<unsigned int> holder;
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

      //Keeps the clients' sockets in a vector.
      clients.push_back(ConnectFD);
      //Adds a new player to the matrix.
      holder.clear();
      for(unsigned int i = 0; i < game_matrix.size(); i++){
        holder.push_back(0);
        game_matrix[i].push_back(0);
      }
      holder.push_back(0);
      game_matrix.push_back(holder);
	    
      std::thread(bot,ConnectFD).detach();
    }

    close(SocketFD);
    return 0;
  }
