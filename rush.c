#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define READ_END 0
#define WRITE_END 1

int cortacar(char* cmd, char **argv, char car[]){
    char * token;
    int i=0;
    token = strtok(cmd,car);
    do{
        argv[i++] = token;
        token = strtok(NULL,car);
    }while(token!=NULL);
    argv[i] = '\0';
    return i;

}

int handlearq(char *argv){//lida com criacao de arquivo
    char *aux[100], *aux2[100], a[100];
    int tam,tam2;
    strcpy(a,argv);
    //fprintf(stderr,"cmd: %s\n", argv);
    tam = cortacar(argv,aux,">");//separa por > e coloca em **aux
    //fprintf(stderr,"tam: %d\n", tam);
    if(tam == 1){
        tam = cortacar(a,aux,"<");//separa por > e coloca em **aux
        //fprintf(stderr,"tam: %d\n", tam);
        if(tam == 1)
            return 1;
        
        if(fork()==0){
            tam2 = cortacar(aux[1],aux2," ");
            aux2[tam2] = NULL;
            freopen(aux2[0],"r",stdin);
            tam2 = cortacar(aux[0],aux2," ");
            execvp(aux2[0],aux2);
        }

        wait(NULL);
        return 0;
    }
    if(fork()==0){
        tam2 = cortacar(aux[1],aux2," ");
        aux2[tam2] = NULL;
        freopen(aux2[0],"w",stdout);
        tam2 = cortacar(aux[0],aux2," ");
        execvp(aux2[0],aux2);
    }

    wait(NULL);

    return 0;
}

int handlearqent(char *argv){//lida com a leitura de arquivos
    char *aux[100], *aux2[100];
    int tam,tam2;
    fprintf(stderr,"cmd: %s\n", argv);
    tam = cortacar(argv,aux,"<");//separa por > e coloca em **aux
    //fprintf(stderr,"tam: %d\n", tam);
    if(tam == 1)
        return 1;
    
    if(fork()==0){
        tam2 = cortacar(aux[1],aux2," ");
        aux2[tam2] = NULL;
        freopen(aux2[0],"r",stdin);
        tam2 = cortacar(aux[0],aux2," ");
        execvp(aux2[0],aux2);
    }

    wait(NULL);

    return 0;
}

void separg(char *cmd,char **argv){//separa argumentos
    char *argfim[100]; //argumentos finais, argv = 
    char auxiliar[100];
    int p1[2],p2[2];
    int *aux;
    int tam = cortacar(cmd,argfim,"|");
    int tam2;
    for(int i=0;i<tam;i++){
        //fprintf(stderr,"cmd antes: %s\n", argfim[i]);
        strcpy(auxiliar,argfim[i]);
        tam2 = cortacar(argfim[i],argv," ");
        argv[tam2] = NULL;
        if(i%2==0)
            aux = p1;
        else
            aux = p2;
        if(pipe(aux)==-1){
            perror("Falha ao criar pipe");
            exit(1);
        }
        if(fork() == 0){
            if(i%2==0 && i < (tam-1)){   
                dup2(p1[WRITE_END],STDOUT_FILENO);
                close(p1[WRITE_END]);
                close(p1[READ_END]);
            }
            if(i > 0 && i<(tam-1) && i%2 == 1){
                dup2(p2[WRITE_END],STDOUT_FILENO);
                close(p2[WRITE_END]);
                close(p2[READ_END]);
            }
            if(i > 0 && i%2 == 0){
                dup2(p2[READ_END],STDIN_FILENO);
                close(p2[READ_END]);
                close(p2[WRITE_END]);
            }
            if(i > 0 && i%2 == 1){
                dup2(p1[READ_END],STDIN_FILENO);
                close(p1[READ_END]);
                close(p1[WRITE_END]);
            }
            //fprintf(stderr,"cmd: %s\n", argfim[i]);
            if(handlearq(auxiliar)==1)
                execvp(argv[0],argv);
            else
                exit(0);
        }
        if(i%2==1){
            close(p1[WRITE_END]);
            close(p1[READ_END]);
        }else if(i > 0){
            close(p2[WRITE_END]);
            close(p2[READ_END]);
        }
    }
    
    for(int i=0;i<tam;i++)    
        wait(NULL);

}

int main(void){

    char cmd[100];
    char *argv[100];

    while(strcmp(cmd,"exit") != 0){
        printf("rush ~$ ");

        fgets(cmd,100,stdin);
        cmd[strlen(cmd)-1]='\0';

        printf("\n");
        separg(cmd,argv);
        if(strcmp(cmd,"exit") == 0){
            printf("bye bye!\n");
        }
    }
    return 0;
}