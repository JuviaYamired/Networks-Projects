  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <thread>
  #include <iostream>
  #include <vector>
  #include <unistd.h>
  using namespace std;

  typedef vector<char> vch;

  const unsigned int HEADER_SIZE= 2;       //[playerId][action]
  const unsigned int A_MOV_SIZE= 5;        //[x][x][y][y][direction]
  const unsigned int A_SHT_SIZE= 5;        //[x][x][y][y][direction]
  const unsigned int A_CHT_SIZE= 3;        //[size][of][message]
  const unsigned int A_HOK_SIZE= 1;        //[playerId]
  const unsigned int MAX_ACTION= 7;
  const unsigned int MAX_SIZE= 10;
  const unsigned int MIN_SIZE= 5;
  const unsigned int MAX_HIT= 4;

  const string A_MOVE=   "m";   //Move  Action
  const string A_SHOOT=  "s";   //Shoot Action
  const string A_HURT=   "h";   //Hurt  Action
  const string A_KILL=   "k";   //Kill  Action
  const string A_MATRIX= "x";   //Kill  Action
  const string A_CHAT=   "c";   //Chat  Action
  const string keyShoot= "12345678"; //Shoot Directions
  char gameArea[50][50];
  vector<vch> myMap;
  vector<vector<unsigned int> > game_matrix;
  char num_player;
  string playerId;
  bool dead = false;
  //int xx, yy;

  //Transforms an int to a sized string e.g., intToStr(21,4) => 0021, intToStr(9,3) => 009
  string intToStr(int num, int size){
    string result;
    for(int i= 0; i < size; i++)
      result+='0';
    for(int i = size - 1; i >= 0; --i){
      result[i] = std::to_string(num%10)[0];
      num = num/10;
    }
    return result;
  }

  void initializeMap(vector<vch>& myMap){
  vch dummy;
  for(int i = 0; i < MAX_SIZE; i++){
    dummy.clear();
    for(int j = 0; j < MAX_SIZE; j++){
      dummy.push_back(' ');
    }
    myMap.push_back(dummy);
  }
  }

  void drawMap(){
  for(int i = 0; i < myMap.size(); i++){
    for(int j = 0; j < myMap.size(); j++){
      cout<<myMap[i][j];
    }
    cout<<endl;
  }
  }

  void printGameMatrix(){
    for(unsigned int i = 0; i < game_matrix.size(); i++){
      for(unsigned int j = 0; j < game_matrix[i].size(); j++){
        cout<<game_matrix[i][j]<<" ";
      }
      cout<<endl;
    }
  }

  void updateMatrix(string my_matrix){
  vector<unsigned int> holder;
  string num;
  bool anterior = false;
  game_matrix.clear();
  for(unsigned int i = 0; i < my_matrix.size(); i++){
    if(my_matrix[i] == ','){
      holder.push_back(stoi(num));
      num = "";
    }
    else if(my_matrix[i] == ';'){
      if(my_matrix[i-1] != ';') holder.push_back(stoi(num));
      game_matrix.push_back(holder);
      holder.clear();
      num = "";
    }
    else{
      num.push_back(my_matrix[i]);
    }
  }
  game_matrix.pop_back();
}

  char getNumPlayer(char letterPlayer){
  string abc = "-ABC";
  char index;
  for(unsigned int i = 0; i < abc.size(); i++){
    if(abc[i] == letterPlayer){
      index = char(i + '0');
      return index;
    }
  }
  return '-';
}

char hurtOrKilled(char letterPlayer){
  char letterPlayerNum = getNumPlayer(letterPlayer);
  unsigned int i = (letterPlayerNum - '0') - 1;
  unsigned int sum = 0;
  for(unsigned int j = 0; j < game_matrix.size(); j++){
    sum += game_matrix[j][i];
  }
  cout<<"suma: "<<sum<<endl;
  if(sum < 3) return 'h';
  return 'k';
}

  void updateBullet(int x, int y, char dir, char myPlayer, int socketFD){
  bool start = true;
  char protocol[HEADER_SIZE + A_HOK_SIZE];
  while(x > 0 && x < (MAX_SIZE-1) && y > 0 && y < (MAX_SIZE-1)){
    if(myMap[x][y]=='.') myMap[x][y] = ' ';

    if(dir == '1') x--;
    else if(dir == '2') x--,y--;
    else if(dir == '3') y--;
    else if(dir == '4'){
      if(start) x++;
      x++, y--;
    }
    else if(dir == '5'){
      if(start) x++;
      x++;
    }
    else if(dir == '6'){
      if(start) x++, y++;
      x++, y++;
    }
    else if(dir == '7'){
      if(start) y++;
      y++;
    }
    else if(dir == '8'){
      if(start) x--, y++;
      x--, y++;
    }
    start = false;
    if(myMap[x][y] != ' ' && myMap[x][y] != '.'){
      if(playerId[0] == myPlayer){
          cout<<"Entre Colision"<<endl;
	         protocol[0]= myPlayer;
           protocol[1]= hurtOrKilled(myMap[x][y]);
           protocol[2]= getNumPlayer(myMap[x][y]);
           cout<<"enviando Colision"<<endl;
	         int n = write(socketFD,protocol,HEADER_SIZE + A_HOK_SIZE);
           if (n < 0) perror("ERROR reading from socket");
	         printf("BALA: %s", protocol);
      }

      myMap[x][y] = ' ';
      break;
    }
    myMap[x][y] = '.';
    drawMap();
    sleep(1);
  }
  myMap[x][y] = ' ';
  drawMap();
  return;
  }

  void updateMap(vector<vch>& myMap, int x, int y, int player)
  {
    string abc = "-ABC";
    for(int i= 0; i < myMap.size(); i++){
      for(int j= 0; j < myMap.size(); j++){
        if(myMap[i][j] == abc[player]) myMap[i][j] = ' ';
      }
    }

    char pl = abc[player];
    for(int i = 0; i < myMap.size(); i++){
      for(int j = 0; j < myMap.size(); j++){
        if(i == x && j == y) {
          myMap[i][j] = pl;
          myMap[i][j+1] = pl;
          myMap[i+1][j] = pl;
          myMap[i+1][j+1] = pl;
        }
      }
    }
  }


  void readSD(int clientSD){
    string x,y,ptcPlayer,ptcAction;
    char *protocol;
    int dynMessageSize;
    string dySizeStr;
    int n;
    protocol = new char[HEADER_SIZE + A_MOV_SIZE];

    //Reading the first protocol send by the server to set playerId and initial position
    n = read(clientSD, protocol, HEADER_SIZE + A_MOV_SIZE);
    if (n < 0) perror("ERROR reading from socket");
    printf("Received initial protocol: %s\n", protocol);

    //Setting the playerId assigned from the server
    playerId = protocol[0];

    //Retrieving the position x from the protocol
    x=  protocol[2];
    x+= protocol[3];

    //Retrieving the position y from the protocol
    y=  protocol[4];
    y+= protocol[5];

    //Drawing the player in the map
    updateMap(myMap, stoi(x), stoi(y), stoi(playerId));
    drawMap();
    //Reading the next messages send by the server
    while(true){
      n = read(clientSD,protocol,HEADER_SIZE);
      if (n < 0) perror("ERROR reading from socket");
      //Which player is doing the action
      ptcPlayer = protocol[0];
      //Which action is going to do
      ptcAction = protocol[1];
      //Getting the size of the message
      //dySizeStr =  protocol[2];
      //dySizeStr += protocol[3];
      //dySizeStr += protocol[4];
      //dynMessageSize =  atoi(dySizeStr.c_str());
      //Resize to retrieve the message
      //delete[] protocol;
      //protocol = new char[dynMessageSize];
      //Verifying which action is going to take place
      if (ptcAction == A_MOVE) {
        printf("Reading protocol: %s\n", protocol);
        dynMessageSize= A_MOV_SIZE;

        //Retrieve the message
        n = read(clientSD, protocol, dynMessageSize);
        if (n < 0) perror("ERROR writing to socket");

        //Retrieving the position x from the protocol
        x=  protocol[0];
        x+= protocol[1];

        //Retrieving the position y from the protocol
        y=  protocol[2];
        y+= protocol[3];

        //Updating the map with the new position of the player that do the action
        updateMap(myMap, stoi(x), stoi(y), stoi(ptcPlayer));
        drawMap();
      }

      else if(ptcAction == A_SHOOT){
        printf("Shoot protocol: %s\n", protocol);
        dynMessageSize= A_SHT_SIZE;

        //Retrieve the message
        n = read(clientSD, protocol, dynMessageSize);
        if (n < 0) perror("ERROR writing to socket");

        //Retrieving the position x from the protocol
        x=  protocol[0];
        x+= protocol[1];

        //Retrieving the position y from the protocol
        y=  protocol[2];
        y+= protocol[3];
        std::thread(updateBullet,stoi(x),stoi(y),protocol[4], ptcPlayer[0],clientSD).detach();
      }

      else if(ptcAction == A_MATRIX){
        n = read(clientSD, protocol, 3);
        if (n < 0) perror("ERROR writing to socket");
        //Getting the size of the message
        dySizeStr =  protocol[0];
        dySizeStr += protocol[1];
        dySizeStr += protocol[2];
        dynMessageSize =  atoi(dySizeStr.c_str());

        string strProtocol;
        printf("Reading protocol: %s", protocol);
        //Retrieve the message
        n = read(clientSD, protocol, dynMessageSize);
        if (n < 0) perror("ERROR writing to socket");
        printf("%s\n", protocol);
	      strProtocol = protocol;
	      updateMatrix(strProtocol);
	      printGameMatrix();
	      cout<<"La matrix es: "<<strProtocol<<endl;
      }

      else if(ptcAction == A_HURT || ptcAction == A_KILL){
        printf("Reading protocol: %s", protocol);
        //Retrieve the message
        n = read(clientSD, protocol, dynMessageSize);
        if (n < 0) perror("ERROR writing to socket");
        printf("%s\n", protocol);

        if(ptcAction == A_KILL){
          if(protocol[0] == num_player)
            dead = true;
        }
      }

      else if(ptcAction == A_CHAT){
        delete[] protocol;
        protocol = new char[A_CHT_SIZE];

        n = read(clientSD, protocol, A_CHT_SIZE);
        if (n < 0) perror("ERROR writing to socket");

        //Getting the size of the message
        dySizeStr =  protocol[0];
        dySizeStr += protocol[1];
        dySizeStr += protocol[2];
        dynMessageSize =  atoi(dySizeStr.c_str());
        cout <<" chat messahe size"<< dynMessageSize<<endl;
        delete[] protocol;
        protocol = new char[dynMessageSize];

        //Retrieve the message
        n = read(clientSD,protocol,dynMessageSize);
        if (n < 0) perror("ERROR writing to socket");
        printf("Chat protocol: %s\n", protocol);
      }
    }
    shutdown(clientSD, SHUT_RDWR);
    close(clientSD);
  }

  void writeSD(int clientSD){
    unsigned int n, xX , yY;
    string x, y, msgSend, msgChat;
    xX= 0;
    yY= 0;

    //First connection to the server requesting playerId and initial position
    msgSend= "0000000";
    n = write(clientSD, msgSend.data(), HEADER_SIZE + A_MOV_SIZE);
    if (n < 0) perror("ERROR writing to socket");
    msgSend.clear();
    //Communication between client and server
    while(true){
      cin>>msgSend;
      if(dead){
        cout<<"You are dead"<<endl;
      }

      //If player move
      else if(msgSend == "w" or msgSend == "a" or msgSend == "s" or msgSend == "d"){
        switch (msgSend[0]) {
          case 'w': xX--;
                    msgSend = "1";
                    break;
          case 's': xX++;
                    msgSend = "2";
                    break;
          case 'a': yY--;
                    msgSend = "3";
                    break;
          case 'd': yY++;
                    msgSend = "4";
                    break;
        }

        //Build of the protocol
        msgSend = playerId + A_MOVE + intToStr(xX,2) + intToStr(yY,2) + msgSend;
        cout << "Sending Protocol :"<< msgSend << endl;

        //Sending the protocol to the server
        n = write(clientSD, msgSend.data(), HEADER_SIZE + A_MOV_SIZE);
        if (n < 0) perror("ERROR writing to socket");
      }

      //If player chat
      else if (msgSend == "c"){
        cin.ignore();
        getline(cin,msgChat);
        //Protocol
        msgSend = playerId + msgSend +  intToStr(msgChat.size(),3) + msgChat;
        n = write(clientSD,msgSend.data(),MIN_SIZE + msgChat.size());
        if (n < 0) perror("ERROR writing to socket");
      }

      //If player shoot
      else if (keyShoot.find(msgSend) >= 0){
        //Build of the protocol
        msgSend = playerId + A_SHOOT + intToStr(xX,2) + intToStr(yY,2) + msgSend;
        cout << "Sending Protocol :"<< msgSend << endl;

        //Sending the protocol to the server
        n = write(clientSD, msgSend.data(), HEADER_SIZE + A_SHT_SIZE);
        if (n < 0) perror("ERROR writing to socket");
      }
      msgSend.clear();
    }
    shutdown(clientSD, SHUT_RDWR);
    close(clientSD);
  }

  int main(void)
  {
    struct sockaddr_in stSockAddr;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    int n, Res;

    if (-1 == SocketFD)
    {
      perror("cannot create socket");
      exit(EXIT_FAILURE);
    }

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(41001);
    Res = inet_pton(AF_INET, "192.168.1.4", &stSockAddr.sin_addr);

    if (0 > Res)
    {
      perror("error: first parameter is not a valid address family");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
    else if (0 == Res)
    {
      perror("char string (second parameter does not contain valid ipaddress");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }

    if (-1 == connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
    {
      perror("connect failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
    initializeMap(myMap);
    std::thread(writeSD,SocketFD).detach();
    std::thread(readSD,SocketFD).detach();
    while(true){
    }
    return 0;
  }
