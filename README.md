# OpenSource Project

Team No.3  B289056 신경익 / B389002 고경규 / B389028 박현식

void* shared_memory = create_shared_memory(256);
pid_t pid = fork();

if(pid < 0){
            perror("fork error : ");
        exit(0);
}
else if(pid == 0){ /* IN 자식 프로세스 */

        sock_raw = socket(AF_INET , SOCK_RAW , IPPROTO_TCP);
        do{
                received_data_size_in_communication = recvfrom(sock_raw , socket_buffer , buffersize , 0 , &saddr , &saddr_size);
    
                ProcessPacket(socket_buffer, received_data_size_from_communication, shared_memory);

        }while(kill(Parent_pid, 0) == 0);

        close(sock_raw);

else{ /* IN 부모 프로세스 */

    while (SIGINT) {

        rx = calculate_received_data_from_/proc/net/dev (previnfo.rx, ifinfo.rx);
        tx = calculate_transmitted_data_from_/proc/net/dev (previnfo.tx, ifinfo.tx);

        /* rx: (받은 바이트 수), (받은 패킷 수)  tx: (보내는 바이트 수), (받은 패킷 수) */
        sprintf(received_buffer,  "rx: %s %d p/s", received_bytes, received_packet);
        sprintf(transmitted_buffer, "tx: %s %d p/s", transmitted_bytes, transmitted_packet);

        strcpy(received_buffer_from_shared_memory, shared_memory_data);

        total_buffer = received_buffer + transmitted_buffer + received_buffer_from_shared_memory;
        
        printf("%s", total_buffer);
    }
}
