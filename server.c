#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <errno.h>
#include <arpa/inet.h>

int main(){
  struct sockaddr_in server, client;
  int sock;
  // создание сокета
  if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP)) == -1){
    printf("Error create socket!\n");
    return -1;
  }
  // структура для сервера
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl(INADDR_ANY); // локальный адрес
  server.sin_port = htons(2016); // порт сервера
  // связка
  if(bind(sock, (struct sockaddr*)&server, sizeof(server)) == -1){
    printf("Error bind!\n");
    return -1;
  }
  // настройка канала
  struct sctp_initmsg initmsg;
  initmsg.sinit_num_ostreams = 3; // входные потоки
  initmsg.sinit_max_instreams = 3; // выходные потоки
  initmsg.sinit_max_attempts = 2; // максимальное количество попыток
  setsockopt(sock, IPPROTO_SCTP, SCTP_INITMSG, &initmsg, sizeof(initmsg));

  // объявляем очередь
  if(listen(sock,5)==-1){
    printf("Error listen!\n");
    return -1;
  }
  int newsock;
  char buffer[512];
  int clnlen = sizeof(client), flags;
  while(1){
    if((newsock = accept(sock, (struct sockaddr*)&client, &clnlen)) == -1){
      printf("Error accept!\n");
      return -1;
    }
    printf("Новый клиент!\n");
    struct sctp_sndrcvinfo sndrcvinfo; // информация о пересылке
    if(sctp_recvmsg(newsock, (void*)buffer, sizeof(buffer), (struct sockaddr*)NULL, 0, &sndrcvinfo, &flags) == -1){ // принимаем сообщение от клиента
      printf("Error sctp_recvmsg!\n");
      return -1;
    }
    int i;
    for(i = 0; i < 3; i++){ // по всем 3-м потокам отправляем информацию
      if(sctp_sendmsg(newsock, (void*)buffer, (size_t)strlen(buffer), NULL, 0, 0, 0, i, 0, 0) == -1){ // отправляем клиенту сообщение
        printf("Error sctp_sendmsg! %s\n", strerror(errno));
        return -1;
      }
    }
    close(newsock); // закрываем связь с клиентом
  }
  close(sock); // закрываем сокет сервера
  return 0;
}
