//Opravene chyby
//1. osetrenie vratenia hodnoty funkcie connect (client sa nepripoji bez servera)
//2. server pocuva prikazy zadavane zo standartneho vstupu
//3. info zobrazuje spravne hodnoty + nove textove subory na testovanie

//Posielanie suboru
//Idem poslat subor o velkosti 1000B
//Moj buffer ma velkost 300B
//Buffer musim odoslat na 4 krat (3x300 + 1x100B)
//Odoslem serveru spravu run
//Odoslem serveru spravu 1000 (alebo 4 - pocet bufferov)
//Odoslem v cykle 4 krat buffer 

//struct sockaddr_un {
    //sa_family_t sun_family;          
    //char        sun_path[108];
    //};

#include <stdio.h>         //kniznica pridavajuca funkcie na standartny vstup a vystup 
#include <string.h>        //kniznica pridavajuca funkcie na pracu so stringmi
#include <stdlib.h>        //kniznica pridavajuca funkcie na dynamicku alokaciu pamate
#include <sys/types.h>     //kniznica pridavajuca nove datove typy
#include <sys/socket.h>    //kniznica pridavajuca funkcie na pracu so soketmi
#include <sys/un.h>        //kniznica pridavajuca definicie pre UNIX sokety
#include <sys/select.h>    //kniznica pridavajuca select
#include <sys/time.h>      //kniznica pridavajuca definicie casovych datovych struktur
#include <sys/resource.h>  //kniznica pridavajuca definicie proces datovych struktur
#include <dirent.h>        //kniznica sluziaca k pristupe filom pouzivana k bonusu ls
#include <pthread.h>       //kniznica umoznujuca vyuzivat thready
#include <stdbool.h>       //kniznica pridavajuca novy datovy typ boolean

#define MAXSIZE 4096       //maximalna velkost buffra na citanie

int getrusage(int who, struct rusage *usage){  //funkcia naplnujuca strukturu usage memory usage hodnotou a procesor usage hodnotou
   int errorcode;  //premenna typu int, do ktorej sa ulozi chybova hlaska a nasledne sa funkcia osetri
   
    asm("push %2;"            //pushnutie 4bajtoveho parametra na zasobnik
        "push %1;"            //pushnutie 4bajtoveho parametra na zasobnik
        "push $0;"            //koniec pushovania parametrov
        "movl $117, %%eax;"   //movnutie hodnoty 117 (predstavujucu systemove volanie getrusage) 
        "int $0x80;"          //zavolanie jadra systemu (kernel)
        "add $12, %%esp"      //posunutie sa v registri esp o 12bytov
        :"=a"(errorcode)      //parameter errorcode ktory vracia funkcia
        :"D"(who),"S"(usage)  //parametre vracajuce funkciou
        :"memory"             //parameter vracajuci funkciou
        );

    if (errorcode<0) {  //ak sa do premennej errorcode zapise chyba vypise sa na obrazovku
        printf("error");
    }
    return errorcode;
}

int gettimeday(struct timeval *tp, struct timezone *tzp){
	asm("push %2;"              //pushnutie parametra na zasobnik
      "push %1;"              //pushnutie parametra na zasabonik
      "push $0;"              //koniec pushovania parametrov                  
	    "movl $116, %%eax;"     //movnutie hodnoty 117 (predstavujucu systemove volanie gettimeofday) 
      "int $0x80;"            //zavolanie jadra systemu (kernel)
      "pop %%eax;"            //popnutie hodnoty zo zasobnika
      "pop %%eax;"            //popnutie hodnoty zo zasobnika
      "pop %%eax;"            //popnutie hodnoty zo zasobnika
      :"=r"(tp)               //parameter vracajuci funkciou
      :"r"(tp),"r"(tzp)       //parametre vracajuce funkciou
      :"%eax");
}



void get_info(int s, bool choice){  //funkcia vypisujuca/posielajuca info na obrazovku alebo na socket
  struct rusage usage;  //struktura z kniznice sys/resource.h do ktorej sa ukladaju udaje o pouzitej pamati a proc. cse
	struct timeval val;	  //struktura z kniznice sys/time.h do ktorej sa ukladaju udaje o case
	char tmp[100];        //string do ktory sa bude pouzivat vo forme vystupu 
  char time[50];        //medzistring pouzivany na ziskanie casu z funkcie strftime
  time_t nowtime;
 
	getrusage(RUSAGE_SELF,&usage);  //volanie funkcie getrusage, ktora naplni strukturu  usage
	gettimeday(&val,NULL);          //volanie funkcie gettimeday, ktora naplni strukturu val
  strftime(time, sizeof(time), "Actual date: %d.%m.%Y Time: %H:%M:%S", localtime((time_t *)&val.tv_sec));  //zformatovanie vystupu funkie gettimeday do citatelnej podoby
 
  if(choice){	  //ak sa zavola funkcia zo strany socketov, vypise vystup spat na ne
    write(s,"-------------------- INFO ----------------------\n",49);
    sprintf(tmp,"| %s       |\n",time);
    write(s,tmp,strlen(tmp));
    sprintf(tmp,"| Memory usage is: %ld                           |\n",usage.ru_maxrss);
    write(s,tmp,strlen(tmp));
    sprintf(tmp,"| Processor time usage is: %lds %ldms              |\n",usage.ru_utime.tv_sec,usage.ru_utime.tv_usec);
    write(s,tmp,strlen(tmp));
    write(s,"------------------------------------------------\n",49);
  }
  else{        //ak sa zavola funkcia zo standartneho vstupu, vypise sa na obrazovku
    printf("-------------------- INFO ----------------------\n");
    printf("| %s       |\n",time);
    printf("| Memory usage is: %ld                           |\n",usage.ru_maxrss);
    printf("| Processor time usage is: %lds %ldms              |\n",usage.ru_utime.tv_sec,usage.ru_utime.tv_usec);
    printf("------------------------------------------------\n");
  }
}

void get_help(int s,bool choice){  //funkcia vypisujuca/posielajuca help na obrazovku alebo na socket
  const int LENGTH=84;	           //konstanta udavajuca dlzku posielanych stringov
  
  if(choice){  //ak sa zavola funkcia zo strany socketov, vypise vystup spat na ne					                                          
  	write(s,"--------------------------------------- HELP -------------------------------------\n",LENGTH);
  	write(s,"| switches:                                                                      |\n",LENGTH);
  	write(s,"|                                                                                |\n",LENGTH);
  	write(s,"| zadanie2 -c               - run client                                         |\n",LENGTH);
  	write(s,"| zadanie2 -s               - run server                                         |\n",LENGTH);
  	write(s,"| zadanie2 (without switch) - run server anyway                                  |\n",LENGTH);
  	write(s,"|                                                                                |\n",LENGTH);
  	write(s,"| commands:                                                                      |\n",LENGTH);
  	write(s,"|                                                                                |\n",LENGTH);
  	write(s,"| help          - print manual to all switches and commands in the program       |\n",LENGTH);
  	write(s,"| info          - print actual date, time, processor time usage and memory usage |\n",LENGTH);
  	write(s,"| run -filename - for the given file print number of lines, words and characters |\n",LENGTH);
  	write(s,"| quit          - correct halt of connections                                    |\n",LENGTH);
  	write(s,"| halt          - correct halt of connections and the program                    |\n",LENGTH);
  	write(s,"----------------------------------------------------------------------------------\n",LENGTH);
   }
   else{  //ak sa zavola funkcia zo standartneho vstupu, vypise sa na obrazovku
  	printf("--------------------------------------- HELP -------------------------------------\n");
  	printf("| switches:                                                                      |\n");
  	printf("|                                                                                |\n");
  	printf("| zadanie2 -c               - run client                                         |\n");
  	printf("| zadanie2 -s               - run server                                         |\n");
  	printf("| zadanie2 (without switch) - run server anyway                                  |\n");
  	printf("|                                                                                |\n");
  	printf("| commands:                                                                      |\n");
  	printf("|                                                                                |\n");
  	printf("| help          - print manual to all switches and commands in the program       |\n");
  	printf("| info          - print actual date, time, processor time usage and memory usage |\n");
  	printf("| run -filename - for the given file print number of lines, words and characters |\n");
  	printf("| quit          - correct halt of connections                                    |\n");
  	printf("| halt          - correct halt of connections and the program                    |\n");
  	printf("----------------------------------------------------------------------------------\n");
   }
}

void get_file(int s,char *filename,bool choice){		//funkcia otvarajuca subor a vypisujuca pocet jeho riadkov slov znakov												                                       
  FILE *fr;
  char c='\n';
  int numbOfLines=1;
  int numbOfWords=1;
  int numbOfChars=0;
  char array[4]="";
  char output[100];
  
  filename[strlen(filename)-1]='\0';
  if((fr=fopen(filename,"r"))==NULL){
    if(choice)write(s,"\nFilename was not found.\n\n",26);
    else printf("\nFilename was not found.\n\n");
    return;
  }		
    									                                  
  c=numbOfLines+'0';
  if(choice){
    write(s,&c,sizeof(c));
    write(s," ",1);
  }
  else printf("\n%c ",c);
  numbOfLines++;

  while((c=getc(fr))!=EOF){									                                
    ++numbOfChars;
    
    if(c=='\n'){
      if(choice){
        write(s,&c,sizeof(c));
        sprintf(array,"%d",numbOfLines);
        write(s,&array,sizeof(array));
        write(s," ",1);
      }
      else{
        printf("%c",c);
        printf("%d",numbOfLines);
        printf(" ");
      }
      ++numbOfLines;
    }
    else {
      if(choice)write(s,&c,sizeof(c));
      else printf("%c",c);
    }
    if(c==' ' || c=='\n') ++numbOfWords;
  }
  if(choice){
    sprintf(output,"\n\nNumber of chars in file: %d \n",numbOfChars);
    write(s,output,strlen(output));
    sprintf(output,"Number of words in file: %d \n",numbOfWords);
    write(s,output,strlen(output));
    sprintf(output,"Number of lines in file: %d \n\n",numbOfLines-1);
    write(s,output,strlen(output));
  }
  else{
    printf("\n\nNumber of chars in file: %d\n",numbOfChars);
    printf("Number of words in file: %d\n",numbOfWords);
    printf("Number of lines in file: %d\n\n",numbOfLines-1);
  }
  fclose(fr);
}

int ls(int s, char *path, DIR *dir,bool choice){
  char buffer[MAXSIZE];
  struct dirent *dptr=NULL;
  unsigned int count=0;
  
  if(strlen(path)>1)path[strlen(path)-1]='\0';
	                           
  if(dir==NULL){
    if((dir=opendir((const char*)path))==NULL){
      if(choice)write(s,"\nCould not open the working directory\n\n",39);
		  else printf("\nCould not open the working directory\n\n");
   	  return;
    }
  }
  dptr=readdir(dir);
	                                                                                        
  if(dptr!=NULL){
    if(choice){
      sprintf(buffer,"%s  ",dptr->d_name);
      write(s,buffer,strlen(buffer));
      ls(s,path,dir,true);
    }
    else{
     printf("%s  ",dptr->d_name);
     ls(NULL,path,dir,false);
    }
  }
  else{
   if(choice)write(s,"\n",1);
   else printf("\n");
  }
}

void *connection_handler(void *socket_desc){
  int ns = *(int*)socket_desc;
  int r;
  char buffer[MAXSIZE];
     
  while((r=read(ns,&buffer,sizeof(buffer)))>0){		
    char *pch=(char *)malloc(MAXSIZE);
   	strcpy(pch,buffer);
    strtok(pch," ");
               							                            
    if(strcmp(buffer,"quit\n\0")==0) break;
    else if(strcmp(buffer,"halt\n\0")==0){
      close(ns);
      exit(0);
    }
    else if(strcmp(buffer,"info\n\0")==0) get_info(ns,true);
    else if(strcmp(buffer,"help\n\0")==0) get_help(ns,true);
    else if(strcmp(pch,"run")==0){
       while((pch=strtok(NULL," "))!=NULL){get_file(ns,pch,true);}
    }
    else if(strcmp(pch,"ls")==0 || strcmp(pch,"ls\n\0")==0){
      if(strcmp(buffer,"ls\n\0")==0)ls(ns,".",NULL,true);
      else while((pch=strtok(NULL," "))!=NULL){ls(ns,pch,NULL,true);}
    }
    else {
      system(buffer);
      write(ns,"Server: The message was succesfully sent.\n",42); 
    }                             
    bzero(buffer,sizeof(buffer));
  }
  if(r==0){
    puts("Client disconnected");
    fflush(stdout);
  }
  else if(r==-1)perror("recv failed");
  printf("Client #%d disconnected.\n",ns);
  free(socket_desc);
  close(ns);
  return 0;
}

void *input_handler(){
  char input[MAXSIZE];

  while(fgets(input,MAXSIZE,stdin)!=NULL){
    char *pch=(char *)malloc(MAXSIZE);		
   	strcpy(pch,input);
    strtok(pch," ");
               							                            
    if(strcmp(input,"quit\n\0")==0 || strcmp(input,"halt\n\0")==0){
      printf("Server disconnected.\n");
      exit(0);
    }
    else if(strcmp(input,"info\n\0")==0) get_info(NULL,false);
    else if(strcmp(input,"help\n\0")==0) get_help(NULL,false);
    else if(strcmp(pch,"run")==0){
       while((pch=strtok(NULL," "))!=NULL){get_file(NULL,pch,false);}
    }
    else if(strcmp(pch,"ls")==0 || strcmp(pch,"ls\n\0")==0){
      if(strcmp(input,"ls\n\0")==0)ls(NULL,".",NULL,false);
      else while((pch=strtok(NULL," "))!=NULL){ls(NULL,pch,NULL,false);}
    }
    else system(input); 
  }
  printf("Server disconnected.\n");
  return 0;
}

int server(){
  int s, ns, r, *newSock;
  char buffer[MAXSIZE];
  struct sockaddr_un ad;
  void *status;
  
  bzero(buffer,MAXSIZE);
  memset(&ad,0,sizeof(ad));										                                  
  ad.sun_family=AF_LOCAL;
  strcpy(ad.sun_path,"./sock");
  		                              
  if((s=socket(PF_LOCAL,SOCK_STREAM,0))==-1){
    printf("Socket not found!\n");
    return 1;
  }
  
  pthread_t sniffer_thread;
  
  if(pthread_create(&sniffer_thread ,NULL,input_handler,NULL)<0){
      perror("could not create thread");
      return 1;
  }
  unlink("./sock");
  bind(s,(struct sockaddr *)&ad,sizeof(ad));					                          
  listen(s,5);													                                          
  printf("Waiting for client...\n");
  while(ns=accept(s,NULL,NULL)){
    printf("Connection accepted!\n\n");
    
    pthread_t sniffer_thread;
    newSock=malloc(1);
    *newSock=ns;
    
    if(pthread_create(&sniffer_thread ,NULL,connection_handler,(void*)newSock)<0){
      perror("could not create thread");
      return 1;
    }
    printf("Client #%d connected.\n",ns);
    if(ns<0){
        perror("accept failed\n");
        return 1;
    }
  }
  close(s);
  close(ns);
  return 0;										                                  
}

int client(){
  int s, r, rv;
  char buffer[MAXSIZE];
  struct sockaddr_un ad;
  fd_set rs;
  
  memset(&ad,0,sizeof(ad));										                                  
  ad.sun_family=AF_LOCAL;
  strcpy(ad.sun_path,"./sock");
  
  if((s=socket(PF_LOCAL,SOCK_STREAM,0))==-1){
    printf("Socket not found!\n");							                              
    return 1;
  }
  if(connect(s,(struct sockaddr *)&ad,sizeof(ad))<0){
    printf("Connect failed. Error\n");
    return 1;
  }					                         
  FD_ZERO(&rs);
  FD_SET(0,&rs);
  FD_SET(s,&rs);
  printf("Connected!\n\n");
  rv=select(s+1,&rs,NULL,NULL,NULL);					                          

  while (1){
    if(rv==-1){
      printf("Error occured. Conection terminates.\n");
      break;
    }
    else if(rv==0){
      printf("Request timed out. Connection terminates.\n");
      break;
    }
    else{
  		if(FD_ISSET(0,&rs)){								                                        
        r=read(0,&buffer,sizeof(buffer));
  			if(write(s,&buffer,r)<0){
          printf("Server has been disconnected.\n");
          break;
        }
		  }
  		if (FD_ISSET(s,&rs)){								                                        
  			r=read(s,&buffer,sizeof(buffer));
  			if(write(1,&buffer,r)<0){
          printf("Server has been disconnected.\n");
          break;
        }
  		}
  		if(strcmp(buffer,"halt\n\0")==0){
  			close(s);
  			exit(0);
  		}
  		else if(strcmp(buffer,"quit\n\0")==0) break;
      
      bzero(buffer,strlen(buffer));
  		FD_ZERO(&rs);
  		FD_SET(0,&rs);
  		FD_SET(s,&rs);
  		rv = select(s+1,&rs,NULL,NULL,NULL);
    }
  }
  printf("You have been disconnected.\n");
  close(s);
}

int main (int argc, char *argv[]){						                                    
	if(argc==2){
		if(strcmp(argv[1],"-s")==0){
			printf("STARTING SERVER...\n");
			server();
		}
		else if(strcmp(argv[1],"-c")==0){
			printf("STARTING CLIENT...\n");
			client();
		}
		else printf("Program knows only -s and -c arguments.\n");
	}
	else if(argc>2) printf("Too much arguments. Program recognize only one.\n");
	else{
		printf("STARTING SERVER...\n");
		server();
	}
  return 0;
}