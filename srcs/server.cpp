/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/30 18:33:29 by yle-jaou          #+#    #+#             */
/*   Updated: 2025/10/01 18:02:20 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incl/struct_class.hpp"
#include "../incl/server.hpp"
#include <iostream>
#include <string>
#include <cstring>     // memset
#include <unistd.h>    // close
#include <netinet/in.h> // sockaddr_in
#include <sys/socket.h> // socket, bind, listen, accept
#include <cstdio>

/*
    fonction socket(domain, type, protocol);
    cree un socket (fd) pour le serveur
    Domain = ipv4/ipv6(1/2)
    Type = SOCK_STREAM/SOCK_DGRAM (TCP/UDP)
    Protocol = 0 (le systeme decide)

    fonction bind(sockfd, sockaddr *addr, addrlen);
    attache le socket a une adresse locale (ip:port)
    sockfd = server_fd
    addr = struct decrivant une ip
    addrlen = sizeof(addr)

    fonction listen(sockfd, backlog);
    transforme le socket en "passif" pret a recevoir des connexions
    sockfd = server fd
    backlog = nb de connections en attente

    fonction accept(sockfd, sockaddr *addr, *addrlen)
    Prend une connection presente dans la file d'attente de listen et
    cree un nouveau socket pour le client
    sockfd = server fd
    * addr = struct decrivant une ip
    addrlen = sizeof(addr)

    Ce bout de code ne gere qu'une connection et se ferme apres
    (pas l'objectif d'irc de base c un exemple afin de comprendre
    le fonctionnement de ce 4 fonctions)
*/

int start_server(int port, std::string pass){
    (void)pass;
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket");
        return (1);
    }

    int opt = 1;
    //mettre le socket en reuse pour pouvoir reutiliser directement la
    //meme adresse car tcp peut parfois la bloquer environ 1 minute sans
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY; //accepte toute ip locales
    addr.sin_port = htons(port);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0)
        return (perror("bind"), 1);
    
    if (listen(server_fd, 1) < 0)
    {
        perror("listen");
        return 1;
    }
    
    std::cout << "Server listening on port "<< port << std::endl;

    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
    if (client_fd < 0) {
        perror("accept");
        return 1;
    }

    std::cout << "a client connected" << std::endl;

    char buffer[1024];
    int n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (n > 0)
    {
        buffer[n] = '\0';
        std::cout << "Reçu: " << buffer << std::flush;
    }

    close(client_fd);
    close(server_fd);

    return 0;
}
