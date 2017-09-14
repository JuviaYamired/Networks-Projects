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
  #include <unistd.h>
#include <iostream>
 
  using namespace std;

  #define MAX_ACTION 8
  #define MAX_HK 4
  #define MAX_CAPACITY 1025
  vector<int> clients;
vector<vector<unsigned int> > game_matrix;
  int nextNewPlayer = 1;

  string intToStr (int x)
  {
      std::stringstream str;
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
  matrix += ";";
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
  string strProtocol = intToStr(getSize());
  while(strProtocol.size() < 3){
      strProtocol = "0" + strProtocol;
  }

  strProtocol = "1X" + strProtocol + OKson();
  return strProtocol;
}

  void bot(int clientSD)
  {
    char protocol[MAX_CAPACITY];
    //char matrix_protocol[MAX_CAPACITY];
    string intToString, strProtocol;
    char sendProtocol[MAX_ACTION];
    int n;
    bool nuevo_cliente = false;

    while(true){
      nuevo_cliente = false;
      bzero(protocol,MAX_CAPACITY);
      n = read(clientSD,protocol,2);
      if (n < 0) perror("ERROR reading from socket");
      strProtocol = protocol[0];
      strProtocol += protocol[1];
      cout<<"Leyendo dos: "<<strProtocol<<endl;

      
      if(protocol[1] == '0' || protocol[1] == '1' || protocol[1] == '2'){
	bzero(protocol,MAX_CAPACITY);
	n = read(clientSD, protocol, MAX_ACTION - 2);

	if(n < 0) perror("ERROR reading from socket");
      }
      else if(protocol[1] == 'H' || protocol[1] == 'K'){
	bzero(protocol,MAX_CAPACITY);
	n = read(clientSD, protocol, MAX_HK - 2);
        game_matrix[strProtocol[0] - '0' - 1][protocol[0] - '0' - 1]++;
	if(strProtocol[1] == 'K'){
	  updateMatrixKilled(protocol[0]);
	}
	if(n < 0) perror("ERROR reading from socket");
      }
      /*else if(protocol[1] == 'X'){
	n = read(clientSD, protocl, MAX_ACTION);
	if(n < 0) perror("ERROR reading from socket");
      }*/

      strProtocol += protocol;
      //bzero(protocol,MAX_CAPACITY);
      strProtocol.copy(protocol,strProtocol.size());

      cout<<"Leyendo todo: "<<strProtocol<<endl;
      for (int i=0;i<clients.size();i++){
        if(protocol[0] == '0'){
	  nuevo_cliente = true;
          printf("newUser");
          intToString = intToStr(nextNewPlayer);
          protocol[0] = intToString[0];
          protocol[1] = '0';
          protocol[2] = '0';
          protocol[3] = '0';
          protocol[4] = '0';
          protocol[5] = '0';
	  protocol[6] = '0';
	  protocol[7] = '\0';
          nextNewPlayer++;

	  printf("Escribiendo: %s", protocol);
          n = write(clients[i],protocol,MAX_ACTION);
	  strProtocol = buildMatrixProtocol();

	  char matrix_protocol[strProtocol.size()];
	  bzero(matrix_protocol,strProtocol.size());
	  strProtocol.copy(matrix_protocol, strProtocol.size());

	  cout<<"TE envio cliente: "<<strProtocol<<endl;  
	  n = write(clients[i],matrix_protocol,strProtocol.size());
        }
        else{
	  if(protocol[1] == 'H' || protocol[1] == 'K'){
	    n = write(clients[i],protocol,MAX_HK);

	    strProtocol = buildMatrixProtocol();

	    char matrix_protocol[strProtocol.size()];
	    bzero(matrix_protocol,strProtocol.size());
	    strProtocol.copy(matrix_protocol, strProtocol.size());

	    
	    n = write(clients[i],matrix_protocol,strProtocol.size());
	  }
	  
	  else{
	    n = write(clients[i],protocol,MAX_ACTION);
	    if(nuevo_cliente){
	      strProtocol = buildMatrixProtocol();

	      char matrix_protocol[strProtocol.size()];
	      bzero(matrix_protocol,strProtocol.size());
	      strProtocol.copy(matrix_protocol, strProtocol.size());
	      n = write(clients[i],matrix_protocol,strProtocol.size());
	    }
	  }
          
          //bzero(buffer, sizeof(buffer));
          if (n < 0) perror("ERROR writing to socket");
        }
      }
    
    
    }
    shutdown(clientSD, SHUT_RDWR);
    close(clientSD);
    
  }

  int main()
  {
    vector<unsigned int> holder;
    struct sockaddr_in stSockAddr;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    int ConnectFD = 0;
    //char buffer[256];
    int port;

    cout<<"Puerto: ";
    cin>>port;
 
    if(-1 == SocketFD)
    {
      perror("can not create socket");
      exit(EXIT_FAILURE);
    }
    
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
 
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(port);
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
