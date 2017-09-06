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
 
  using namespace std;

  #define MAX_ACTION 7
  vector<int> clients;
  int nextNewPlayer = 1;

  string intToStr (int x)
  {
      std::stringstream str;
      str << x;
      return str.str();
  }

  void bot(int clientSD)
  {
    char protocol[MAX_ACTION];
    string intToString;
    char sendProtocol[MAX_ACTION];
    int n;

    while(true){

      n = read(clientSD,protocol,MAX_ACTION);
      if (n < 0) perror("ERROR reading from socket");
      //printf("Here is the message: [%s]\n",buffer);
      //buffer2 = buffer;
      //buffer2 = "You Send: " + buffer2;
      //strcpy(buffer,buffer2.c_str());
      //holdProtocol = protocol;
      //strcpy(sendProtocol,holdProtocol.c_str());    

      for (int i=0;i<clients.size();i++){
        if(protocol[0] == '0'){
          printf("newUser");
          intToString = intToStr(nextNewPlayer);
          protocol[0] = intToString[0];
          protocol[1] = '0';
          protocol[2] = '0';
          protocol[3] = '0';
          protocol[4] = '0';
          protocol[5] = '0';
	  protocol[6] = '0';
          nextNewPlayer++;
          n = write(clients[i],protocol,MAX_ACTION);
	  
        }
        else{
          n = write(clients[i],protocol,MAX_ACTION);
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
    struct sockaddr_in stSockAddr;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    int ConnectFD = 0;
    //char buffer[256];
    //int n;
 
    if(-1 == SocketFD)
    {
      perror("can not create socket");
      exit(EXIT_FAILURE);
    }
    
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
 
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(41000);
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
    
     //n = read(ConnectFD,buffer,255);
     //if (n < 0) perror("ERROR reading from socket");
     //printf("Here is the message: [%s]\n",buffer);
     //n = write(ConnectFD,"I got your message",18);
     //if (n < 0) perror("ERROR writing to socket");
 
     /* perform read write operations ... */
 
      //shutdown(ConnectFD, SHUT_RDWR);
 
      //close(ConnectFD);
    }
 
    close(SocketFD);
    return 0;
  }
