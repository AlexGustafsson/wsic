#ifndef SERVER_H
#define SERVER_H

void serverListen(int port);
void acceptConnection();
void request();
void respons();
void closeSocket();

#endif /* SERVER_H */
