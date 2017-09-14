 /* Client code in C */
 
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
  #include <curses.h>
  #include <unistd.h>
  using namespace std;

  typedef vector<char> vch;

  #define MAX_ACTION 7
  #define MAX_SIZE 10
  #define MIN_SIZE 5

  char gameArea[50][50];
  vector<vch> myMap;
  char num_player;
  char playerId;
  int xx, yy;

  string intToStr(int num){
    string result;
    result="000";
    for(int i = 2 ; i >= 0; --i){
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

  void updateBullet(int x, int y, char dir){
  bool start = true;
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
    myMap[x][y] = '.';
    drawMap();
    sleep(1);
  }
  myMap[x][y] = ' ';
  drawMap();
  return;
  }

  void updateMap(vector<vch>& myMap,int x, int y, int player){

    string abc = "-ABC";

  for(int i = 0; i < myMap.size(); i++){
    for(int j = 0; j < myMap.size(); j++){
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
    string x,y,player;
    char *protocol;
    int dynMessageSize;
    string dySizeStr;
    int n;

    protocol = new char[MIN_SIZE];

    n = read(clientSD,protocol,MIN_SIZE);
    playerId = protocol[0];
    //num_player = protocol[0];
    while(true){
      if(protocol[1] == 'c'){
        dySizeStr =  protocol[2];
        dySizeStr += protocol[3];
        dySizeStr += protocol[4];
        dynMessageSize =  atoi(dySizeStr.c_str());

        delete[] protocol;
        protocol = new char[dynMessageSize];
        n = read(clientSD,protocol,dynMessageSize);
        if (n < 0) perror("ERROR writing to socket");

        printf("%s\n", protocol);
      }
      //bzero(protocol,MIN_SIZE);
      n = read(clientSD,protocol,MIN_SIZE);
      printf("mi protocolo: %s \n",protocol );
      if (n < 0) perror("ERROR reading from socket");
    }
    /* while(true){
      printf("Mi protocolo %s \n",protocol);

      player = protocol[0];
      //if(protocol[1] == '0' && num_player == '-') num_player = protocol[0];
      x = protocol[2];
      x+= protocol[3];

      y = protocol[4];
      y+= protocol[5];

      if(protocol[1] != '2'){
	      updateMap(myMap, stoi(x), stoi(y), stoi(player));
	      drawMap();
      }
      else if(protocol[1] == '2'){
	      std::thread(updateBullet,stoi(x),stoi(y),protocol[6]).detach();
      }
      else if(protocol[1] == 'c'){

      }
      //bzero(protocol,MAX_ACTION);
      n = read(clientSD,protocol,MAX_ACTION);
      if (n < 0) perror("ERROR reading from socket");
    }*/
    shutdown(clientSD, SHUT_RDWR); 
    close(clientSD);
  }

  void writeSD(int clientSD){
    int n;
    int gotKey;
    string x, y;
    char* protocol;
    char buffer[2];
    string msgSend;
    string msgChat;

    protocol = new char[MIN_SIZE];

    protocol[0] = '0';
    protocol[1] = '0';
    protocol[2] = '0';
    protocol[3] = '0';
    protocol[4] = '0';

    n = write(clientSD,protocol,MIN_SIZE);
    if (n < 0) perror("ERROR writing to socket");
    
    while(true){
      cin>>msgSend;
      if (msgSend == "c"){
        cin.ignore();
        getline(cin,msgChat);
        cout<<"NUMERO"<<intToStr(12)<<endl;
        msgSend = playerId + msgSend +  intToStr(msgChat.size()) + msgChat;
        n = write(clientSD,msgSend.data(),MIN_SIZE + msgChat.size());
        if (n < 0) perror("ERROR writing to socket"); 
      }


      /*
      cin.getline(buffer,2);

      protocol[1] = '1';
      if(buffer[0] == 'e') xx--, protocol[6] = '1';
      else if(buffer[0] == 'x') xx++, protocol[6] = '2';
      else if(buffer[0] == 's') yy--, protocol[6] = '3';
      else if(buffer[0] == 'd') yy++, protocol[6] = '4';
      else if(buffer[0] == 'c') {
        cin>>message;
      }
      else protocol[6] = buffer[0], protocol[1] = '2';
      x = to_string(xx);
      y = to_string(yy);
      if(x.size() == 1) x = '0' + x;
      if(y.size() == 1) y = '0' + y;
      protocol[0] = num_player;
      //protocol[1] = '1';
      protocol[2] = x[0];
      protocol[3] = x[1];
      protocol[4] = y[0];
      protocol[5] = y[1];
      */
      //system("cls");
      //gotKey = getch();
      //switch(gotKey){
      // case 72: printf("72");
      //}
      //
      //string temp="123456";
      //strcpy(protocol,temp.c_str());
      //n = write(clientSD,protocol,MAX_ACTION);
      //updateGameArea();*/
    }
    shutdown(clientSD, SHUT_RDWR);
    close(clientSD);
  }
  
  int main(void)
  {
    struct sockaddr_in stSockAddr;
    //int Res;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    //int n;
    //char buffer[6];

    
    char message[256], action[MAX_ACTION];
    char act;
    string x, y;
    
    char key;
    string buffer, IP;
    int Res;
    int n, port;
    bool disco = false;
    
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
    //while(true)
    //{
    std::thread(writeSD,SocketFD).detach();   
    std::thread(readSD,SocketFD).detach();
    //cin>>message;
    //cin.getline(buffer,6);
    //n= write(SocketFD,message.data(),message.size());
    //n = write(SocketFD,buffer,6);
    /* perform read write operations ... */
    //char newbuffer[6];
    //n = read(SocketFD,buffer,6);
    // if (n < 0) perror("ERROR reading from socket");
    // printf("%s \n",buffer);
    //}
    /**for(int i = 0 ; i < 50 ; ++i){
      for(int j = 0 ; j <50 ; ++j){
        gameArea[i][j]='.';
      }
    }
    gameArea[25][25]='A';
    gameArea[25][26]='A';
    gameArea[26][25]='A';
    gameArea[26][26]='A';**/
    while(true){
      
    }

    //shutdown(SocketFD, SHUT_RDWR);
 
    //close(SocketFD);

    return 0;
  }
