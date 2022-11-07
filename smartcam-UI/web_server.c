#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<sys/wait.h>

#define BUFSIZE 8096

void gst_handler(int fd, char *buffer);
void act_smartcam();

struct {
    char *ext;
    char *filetype;
} extensions [] = {
    {"gif", "image/gif" },
    {"jpg", "image/jpeg"},
    {"jpeg","image/jpeg"},
    {"png", "image/png" },
    {"zip", "image/zip" },
    {"gz",  "image/gz"  },
    {"tar", "image/tar" },
    {"htm", "text/html" },
    {"html","text/html" },
    {"css","text/css"},
    {"js","text/javascript"},
    {"exe","text/plain" },
    {0,0} };

void handle_socket(int fd)
{
    int j, file_fd, buflen, len;
    long i, ret;
    char * fstr;
    static char buffer[BUFSIZE+1];
    char type = 0;

    ret = read(fd,buffer,BUFSIZE);   /* 讀取瀏覽器要求 */

    if (ret==0||ret==-1) {
    /* 網路連線有問題，所以結束行程 */
        exit(3);
    }

    /* 程式技巧：在讀取到的字串結尾補空字元，方便後續程式判斷結尾 */
    if (ret>0&&ret<BUFSIZE)
        buffer[ret] = 0;
    else
        buffer[0] = 0;

    /* 只接受 POST, GET 命令要求 */
    if(!strncmp(buffer,"POST ",5)||!strncmp(buffer,"post ",4)){
        gst_handler(fd, buffer);
    }
    else if (!strncmp(buffer,"GET ",4)|| !strncmp(buffer,"get ",4)){
    }
    else{
        exit(3);
    }

    /* 移除換行字元 */
    for (i=0;i<ret;i++) 
        if (buffer[i]=='\r'||buffer[i]=='\n')
            buffer[i] = 0;

    /* 我們要把 GET /index.html HTTP/1.0 後面的 HTTP/1.0 用空字元隔開 */
    for(i=4;i<BUFSIZE;i++) {
        if(buffer[i] == ' ') {
            buffer[i] = 0;
            break;
        
        }
    }

    /* 檔掉回上層目錄的路徑『..』 */
    for (j=0;j<i-1;j++)
        if (buffer[j]=='.'&&buffer[j+1]=='.')
            exit(3);

    /* 當客戶端要求根目錄時讀取 index.html */
    if (!strncmp(&buffer[0],"GET /\0",6)||!strncmp(&buffer[0],"get /\0",6) )
        strcpy(buffer,"GET /index.html\0");

    /* 檢查客戶端所要求的檔案格式 */
    buflen = strlen(buffer);
    fstr = (char *)0;

    for(i=0;extensions[i].ext!=0;i++) {
        len = strlen(extensions[i].ext);
        if(!strncmp(&buffer[buflen-len], extensions[i].ext, len)) {
            fstr = extensions[i].filetype;
            break;
        }
    }

    /* 檔案格式不支援 */
    if(fstr == 0) {
        fstr = extensions[i-1].filetype;
    }

    /* 開啟檔案 */
    if((file_fd=open(&buffer[5],O_RDONLY))==-1)
        write(fd, "Failed to open file", 19);

    /* 傳回瀏覽器成功碼 200 和內容的格式 */
    sprintf(buffer,"HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n", fstr);
    write(fd,buffer,strlen(buffer));


    /* 讀取檔案內容輸出到客戶端瀏覽器 */
    while ((ret=read(file_fd, buffer, BUFSIZE))>0) {
        write(fd,buffer,ret);
    }

    exit(1);
}

void gst_handler(int fd, char *buffer)
{
    int start = 0, len = 0, ret = 0;
    char *patten="GSTREAMER: \0";
    char *payload;

    // 找 gstreamer 指令位置
    payload = strstr(buffer, patten);
    
    if(payload == NULL){
        printf("Not a gst!\n");
        exit(1);
    }

    len = strlen(payload);

    // quit
    if(strncmp(&payload[11],"quit",4) == 0){
        pid_t pid = 0;
        
        // 拿取要 kill 的 target's pid
        FILE *shard_val_fd = fopen("./runtime/process.txt","r+");
        fscanf(shard_val_fd, "%d", &pid);

        printf("kill %d\n", pid);
        kill(-1*pid, SIGKILL);
        fclose(shard_val_fd);
        system("rm ./runtime/process.txt");
        exit(0);
    }
    
    FILE *shard_val_fd = fopen("./runtime/process.txt","w");
    fprintf(shard_val_fd, "%d", getpid());
    fclose(shard_val_fd);

    // 顯示指令並移除指令的換行符號
    printf("%s\n", &payload[11]);
    len = strlen(&payload[11]);
    payload[10+len] = 0;

    // 檢查是哪個應用
    // if(strncmp(&payload[11], "sudo smartcam", 13) == 0)
        // act_smartcam();

    setsid();

    // 執行程式
    system(&payload[11]);

    // 如果命令錯誤或執行完畢，會執行這行刪除自己的 pid
    system("rm ./runtime/process.txt");
}

void act_smartcam(){
    FILE *fd = fopen("./runtime/status.txt", "r");
    
    if(fd == NULL){
        // 啟動 smartcam
        system("sudo xmutil unloadapp");
        system("sudo xmutil loadapp kv260-smartcam");
        system("sudo xmutil listapps");
    }
    else{
        fclose(fd);
    }
}

// kill zombie child
void sig_chld(int sig_num){
    waitpid(-1, NULL, 0);
    return;
}

void main(int argc, char **argv)
{
    int i, pid, listenfd, socketfd;
    socklen_t length;
    static struct sockaddr_in cli_addr;
    static struct sockaddr_in serv_addr;

    /* 使用 /tmp 當網站根目錄 */
    if(chdir("./") == -1){ 
        printf("ERROR: Can't Change to directory %s\n",argv[2]);
        exit(4);
    }

    /* 背景繼續執行 */
    // if(fork() != 0)
    //     return 0;

    /* 讓父行程不必等待子行程結束 */
    // signal(SIGCLD, SIG_IGN);

    /* 開啟網路 Socket */
    if ((listenfd=socket(AF_INET, SOCK_STREAM,0))<0)
        exit(3);

    /* 網路連線設定 */
    serv_addr.sin_family = AF_INET;
    /* 使用任何在本機的對外 IP */
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    /* 使用 80 Port */
    serv_addr.sin_port = htons(8080);

    if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &(int){1} , sizeof(int)) < 0)
        perror("setsockopt");

    /* 開啟網路監聽器 */
    if (bind(listenfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr))<0)
        exit(3);

    /* 開始監聽網路 */
    if (listen(listenfd,64)<0)
        exit(3);

    length = sizeof(cli_addr);
    /* 等待客戶端連線 */
    while(1){
        if ((socketfd = accept(listenfd, (struct sockaddr *)&cli_addr, &length))<0){
            perror("accept error\n");
            exit(3);
        }

        /* 分出子行程處理要求 */
        if ((pid = fork()) < 0) {
            perror("fork error\n");
            exit(3);
        } else {
            if (pid == 0) {  /* 子行程 */
                close(listenfd);
                handle_socket(socketfd);
            } else { /* 父行程 */
                close(socketfd);
            }
        }
    }
}