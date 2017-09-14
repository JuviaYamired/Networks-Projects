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

  #define MAX_ACTION 8
  #define MAX_HIT 4
  #define MAX_SIZE 20
#define MAX_CAPACITY 1025
  
  char gameArea[50][50];
  vector<vch> myMap;
vector<vector<unsigned int> > game_matrix;
  char num_player;
  int xx, yy;
bool dead = false;

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

void printGameMatrix(){
  for(unsigned int i = 0; i < game_matrix.size(); i++){
    for(unsigned int j = 0; j < game_matrix[i].size(); j++){
      cout<<game_matrix[i][j]<<" ";
    }
    cout<<endl;
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
  cout<<"i "<<i<<" filas: "<<game_matrix[0].size()<<endl;
  for(unsigned int j = 0; j < game_matrix.size(); j++){
    sum += game_matrix[j][i];
  }
  cout<<"suma: "<<sum<<endl;
  if(sum < 3) return 'H';
  return 'K';
}

void updateBullet(int x, int y, char dir, char myPlayer, int socketFD){
  bool start = true;
  char protocol[MAX_HIT];
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
      if(num_player == myPlayer){
	protocol[0] = myPlayer;
	protocol[1] = hurtOrKilled(myMap[x][y]);
	protocol[2] = getNumPlayer(myMap[x][y]);
	protocol[3] = '\0';
	int n = write(socketFD,protocol,MAX_HIT);
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
    string x,y,player, strProtocol;
    char protocol[MAX_CAPACITY];
    int n;
    while(true){
      bzero(protocol,MAX_CAPACITY);
      n = read(clientSD,protocol,2);
      printf("Mi read protocolo %s \n",protocol);

      strProtocol = protocol[0];
      strProtocol += protocol[1];
      cout<<"Leo inicial: "<<strProtocol<<endl;
      player = protocol[0];
      if(protocol[1] == '0' && num_player == '-') {
	num_player = protocol[0];
	cout<<"HOla"<<endl;
      }

      if(protocol[1] == '0' || protocol[1] == '1' || protocol[1] == '2'){
	cout<<"leo"<<endl;
	bzero(protocol, MAX_CAPACITY);
	n = read(clientSD, protocol, MAX_ACTION - 2);
	strProtocol += protocol;
	cout<<"Mi protocolo leido todo es: "<<strProtocol<<endl;
	strProtocol.copy(protocol, MAX_ACTION);
	x = protocol[2];
	x+= protocol[3];

	y = protocol[4];
	y+= protocol[5];
	if(protocol[1] == '0' || protocol[1] == '1'){
	  updateMap(myMap, stoi(x), stoi(y), stoi(player));
	  drawMap();
	}
	else if(protocol[1] == '2'){
	  std::thread(updateBullet,stoi(x),stoi(y),protocol[6], protocol[0],clientSD).detach();
	}
      }
      
      else if(protocol[1] == 'H' || protocol[1] == 'K'){
	bzero(protocol, MAX_CAPACITY);
	n = read(clientSD, protocol, MAX_HIT - 2);
	strProtocol += protocol;
	if(strProtocol[1] == 'K'){
	  if(strProtocol[2] == num_player) dead = true;
	}
	strProtocol.copy(protocol, MAX_HIT);
	cout<<strProtocol<<endl;
      }
      else if(protocol[1] == 'X'){
	bzero(protocol, MAX_CAPACITY);
	n = read(clientSD, protocol, 3);
	strProtocol = protocol;
	n = read(clientSD, protocol, stoi(strProtocol));
	strProtocol = protocol;
	updateMatrix(strProtocol);
	printGameMatrix();
	cout<<"La matrix es: "<<strProtocol<<endl;
      }
      //drawMap();
	
      bzero(protocol,MAX_CAPACITY);
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
    protocol[6] = '0';
    protocol[7] = '\0';

    n = write(clientSD,protocol,MAX_ACTION);
    //printf("Mi write protocolo es: %s", protocol);
    cin.ignore();
    while(true){
      //cin.ignore(100,'\n');
      bzero(protocol,MAX_ACTION);
      cin.getline(buffer,2);
      //cin>>n;

      if(!dead){
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
	protocol[7] = '\0';
	printf("Mi write protocolo es: %s", protocol);
	n = write(clientSD,protocol,MAX_ACTION);
      }
      

     
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
    
    cout<<"Puerto: ";
    cin>>port;
    cout<<"IP: ";
    cin>>IP;
    if (-1 == SocketFD)
    {
      perror("cannot create socket");
      exit(EXIT_FAILURE);
    }
 
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
 
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(port);
    Res = inet_pton(AF_INET, "192.168.1.108", &stSockAddr.sin_addr);
 
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
  
    std::thread(writeSD,SocketFD).detach();   
    std::thread(readSD,SocketFD).detach();
    while(true){
      
    }

    //shutdown(SocketFD, SHUT_RDWR);
 
    //close(SocketFD);

    return 0;gameArea[25][25]='A';
}
