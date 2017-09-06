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
//#include <curses.h>
  #include <unistd.h>
  using namespace std;

  typedef vector<char> vch;

  #define MAX_ACTION 7
  #define MAX_SIZE 10
  
  char gameArea[50][50];
  vector<vch> myMap;
  char num_player;
  int xx, yy;

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



  void updateGameArea()
  {
    printf("\n \033[2J");
    for(int i = 0 ; i < 50 ; ++i){
      for(int j = 0 ; j < 50 ; ++j){
        printf("%c",gameArea[i][j]);
      }
      printf("\n");
    }
  }

  void readSD(int clientSD){
    string x,y,player;
    char protocol[MAX_ACTION];
    int n;
    while(true){
      n = read(clientSD,protocol,MAX_ACTION);
      printf("Mi protocolo %s \n",protocol);

      player = protocol[0];
      if(protocol[1] == '0' && num_player == '-') num_player = protocol[0];
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
      //drawMap();
	
      bzero(protocol,MAX_ACTION);
    }
    shutdown(clientSD, SHUT_RDWR); 
    close(clientSD);
  }

  void writeSD(int clientSD){
    int n;
    int gotKey;
    string x, y;
    char protocol[MAX_ACTION];
    char buffer[2];
    //updateGameArea();
    protocol[0] = '0';
    protocol[1] = '0';
    protocol[2] = '0';
    protocol[3] = '0';
    protocol[4] = '0';
    protocol[5] = '0';
    protocol[6] = -'0';

    n = write(clientSD,protocol,MAX_ACTION);
    while(true){
      cin.getline(buffer,2);

      protocol[1] = '1';
      if(buffer[0] == 'e') xx--, protocol[6] = '1';
      else if(buffer[0] == 'x') xx++, protocol[6] = '2';
      else if(buffer[0] == 's') yy--, protocol[6] = '3';
      else if(buffer[0] == 'd') yy++, protocol[6] = '4';
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
      //system("cls");
      //gotKey = getch();
      //switch(gotKey){
      // case 72: printf("72");
      //}
      //
      //string temp="123456";
      //strcpy(protocol,temp.c_str());
      printf("Mi protocolo es: %s", protocol);
      n = write(clientSD,protocol,MAX_ACTION);
      //updateGameArea();
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
    stSockAddr.sin_port = htons(41000);
    Res = inet_pton(AF_INET, "127.0.0.1", &stSockAddr.sin_addr);
 
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

    bzero(action,MAX_ACTION);
    action[0] = '-';
    action[1] = '1';
    action[2] = '0';
    action[3] = '0';
    action[4] = '0';
    action[5] = '0';
    action[6] = '0';
    initializeMap(myMap);
    num_player = action[0];
    act = action[1];
    x = action[2];
    x.push_back(action[3]);
    cout<<x<<endl;
    xx = stoi(x);
    y = action[4];
    y.push_back(action[5]);

    yy = stoi(y);
    buffer = action;
    cout<<buffer<<endl;
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

    return 0;gameArea[25][25]='A';
  }
