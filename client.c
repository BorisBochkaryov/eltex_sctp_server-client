#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <netdb.h>

int main(){
  // настройка структуры для сервера
  struct sockaddr_in server;
  server.sin_addr.s_addr = inet_addr("192.168.0.105");
  server.sin_family = AF_INET;
  server.sin_port = htons(2016);
  // создаем сокет
  int sock;
  if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP)) == -1){
    printf("Error create!\n");
    return -1;
  }
  // настройка канала
  struct sctp_initmsg initmsg;
  memset(&initmsg, 0, sizeof(initmsg));
  initmsg.sinit_num_ostreams = 3; // выходные потоки
  initmsg.sinit_max_instreams = 3; // входные потоки
  initmsg.sinit_max_attempts = 2; // количество попыток
  setsockopt(sock, IPPROTO_SCTP, SCTP_INITMSG, &initmsg, sizeof(initmsg));
  // соединяемся с сервером
  if(connect(sock, (struct sockaddr*)&server, sizeof(server)) == -1){
    printf("Error connect!\n");
  }
  // отправка и прием сообщения 
  char buf[512] = "Hello!";
  sctp_sendmsg(sock, (void*)buf, (size_t)strlen(buf), NULL, 0, 0, 0, 1 /* номер потока */, 0, 0);
  struct sctp_sndrcvinfo sndrcvinfo;
  int i, flags;
  char buff[512];
  for(i = 0; i < 3; i++){ // по 3-м каналам принимаем сообщения
    bzero((void *)&buff, sizeof(buff));
    sctp_recvmsg(sock, (void*)buff, sizeof(buff), (struct sockaddr*)NULL, 0, &sndrcvinfo, &flags);
    printf("Сервер: %s\n",buff);
  }
  close(sock);
  return 0;
}
