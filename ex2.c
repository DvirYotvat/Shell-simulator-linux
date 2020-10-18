// Dvir Yotvat 311212823

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include<sys/stat.h>
#include<fcntl.h>

#define SIZE 30
#define UserType 510

// flags for pipe and redirection
int flag_output=0, flag_input=0, flag_err=0, redirection_flag=0, flag_pipe=0;
// save the name of file and the rest of the commend
char fileName[UserType], the_command[UserType];

// count redirections
int cou_redirections=0;
int cou_right=0, cou_left=0;

char **left;
char **right;



// function that make 2 dimensional array of words frome 1 dimensional array of chars (when you have space between "words")
char** array_to_2D_array(char* oneDarray, char** towDarr, int size){

    int chars_in_word=0;
    int i=0;

    // token take word frome the 1D array
    char* token=strtok(oneDarray, " ");
    //run on the 1D array and take words
    while(token!=NULL){
        chars_in_word=strlen(token);
        //place the words in the 2D array
        towDarr[i]=(char*)malloc((chars_in_word+1)*sizeof(char));
        if(towDarr[i]==NULL){
            perror("ERROR: could not allocate memory!\n ");
            exit(1);
        }
        strcpy(towDarr[i], token);
        token=strtok(NULL, " ");
        i++;
    }
    //last place in the array is NULL
    towDarr[chars_in_word]=NULL;   

    return towDarr;
}

// if the user want to open file and etc the function divide the file name and the commend
void divide_the_command_and_file_name(char* command ){

    char* command_temp;
    char* file_name_temp;
    char command_copy[UserType], command_copy2[UserType];
    strcpy(command_copy, command);
    strcpy(command_copy2, command);

    if(strstr(command, ">>")!=NULL){
        flag_output=2;
        command_temp=strtok(command_copy, ">");  
        file_name_temp=strstr(command_copy2, ">"); 
        file_name_temp++;
        file_name_temp++;
    }

    else if(strstr(command, ">")!=NULL){             
        flag_output=1;
        command_temp=strtok(command_copy, ">");
        file_name_temp=strstr(command_copy2, ">");
        file_name_temp++;
    }
    else if(strstr(command, "2>")!=NULL){
        flag_err=1;
        command_temp=strtok(command_copy, "2");
        file_name_temp=strstr(command_copy2, ">");
        file_name_temp++;
    }
    
    else if(strstr(command, "<")!=NULL){
        flag_input=1;
        command_temp=strtok(command_copy, "<");
        file_name_temp=strstr(command_copy2, "<");
        file_name_temp++;
    }

    
    if(flag_output==1 || flag_err==1 || flag_input==1 || flag_output==2){
        redirection_flag=1;
        cou_redirections++;
        // delet the space in the name of the file
        while(file_name_temp[0]==' '){
        file_name_temp++;
        }   
        strcpy(fileName, file_name_temp);
        strcpy(the_command, command_temp); 
    }
    if(redirection_flag==0)
         strcpy(the_command, command);


} 


void divide_pipe(char *command){

    char copy[UserType], copy2[UserType];
    cou_left=0, cou_right=0;
    strcpy(copy, command);
    char *cut;

    // check how many words there is in the left side of the order before the "|" (pipe)
    char *leftCommand=strtok(copy, "|");
    char *rightCommand=strtok(NULL, "\n");

    char *temp2=strtok(rightCommand, " ");
    while(temp2!=NULL){                         
        temp2=strtok(NULL, " ");     
        cou_right++;
    }
    right=(char**)malloc((cou_right+1)*(sizeof (char*)));
    if(right==NULL){
        perror("error: could not allocate memory!\n ");
        exit(EXIT_FAILURE);
    }

    char *temp=strtok(leftCommand, " ");
    while(temp!=NULL){                         
        temp=strtok(NULL, " ");     
        cou_left++;
    }

    left=(char**)malloc((cou_left+1)*(sizeof (char*)));
    if(left==NULL){
        perror("error: could not allocate memory!\n ");
        exit(EXIT_FAILURE);
    }

    leftCommand=strtok(copy2, "|");
    rightCommand=strtok(NULL, "\n"); 

    if(flag_output>0){
        cut=strtok(rightCommand, ">");
    }
    else if(flag_err==1){
        cut=strtok(rightCommand, "2");
    }
    else if(flag_input==1){
        cut=strtok(rightCommand, "<");
    }
    if(redirection_flag==1){
        while(cut[0]==' '){
            cut++;
        }
        right=array_to_2D_array(cut, right, cou_right+1);
    }
    else if(redirection_flag==0){
        while(rightCommand[0]==' '){
            rightCommand++;
        }
        right=array_to_2D_array(rightCommand, right, cou_right+1);
    }

    
    left=array_to_2D_array(leftCommand, left, cou_left+1);

}



void make_only_one_command(char *command){

    char copy[UserType];
    int cou_words=0;
    char **exe;
    strcpy(copy, command);
    //count how much words the user type
    char *token = strtok(copy, " ");
    // if the user enter the commend cd
    if(token != NULL){
        cou_words++;
        while (token != NULL){
            token = strtok(NULL, " ");
            cou_words++;
        }
    }
    /* new 2d array that save what the user type */
    exe=(char**)malloc((cou_words+1)*(sizeof (char*)));
    if(exe==NULL){
        perror("ERROR: could not allocate memory!\n");
        exit(1);
    }
    // make 2D array frome 1D array
    exe=array_to_2D_array(command, exe, cou_words+1);
    pid_t son=fork();
    if(son<0){
        perror("ERROR: could not allocate memory!\n ");
        exit(1);
    }
    if(son==0){
        execvp(exe[0],exe);
        //printf("%s: coomand not found\n", exe[0]);
        exit(0);
    }
    else{
        wait(NULL);
        // free al mallocs
        for(int i=0; i<cou_words; i++)
            free(exe[i]);
        free(exe);
    }

}



int main(){

    // str that say how much he can type
    char userStr[UserType];  
    // get the name user and the folder
    char name[SIZE];   
    struct passwd *pointer;    
    pointer=getpwuid(getuid());  
    
        
    char copy[UserType];
    char **arr_for_exec;

    //counter how much words we have in what the user type
    int couWord=0;
    //conter how much chars we have in token
    int charCount=0;

    /*              summary counters          */
    // how much commend the user enter to the program 
    int commends=0;
    // how much chars in total the user enter to the program
    int len_of_chars=0;
    //average of chars for commend
    double average=0;
    // count pipes
    int cou_pipe=0;

    /*               end of summary            */

    int fileDiscriptor=0;

    // loop that get and executing orders from user
    while (strcmp(userStr, "done")!=0){

        couWord=0, charCount=0, cou_left=0, cou_right=0;
        
        flag_output=0, flag_input=0, flag_err=0, redirection_flag=0, flag_pipe=0;

        //print the user name and the folder
        printf("%s@%s>", pointer->pw_name, getcwd(name, SIZE));
        // get order frome user
        fgets(userStr, UserType, stdin);
        // run over the "\n" and replace to "\0"
        userStr[strlen(userStr)-1]= '\0'; 
        // if the user enter cd 
        if(strcmp(userStr, "cd")==0){
            printf("command not supported (Yet) \n");
        }



        // count the total length of all chras the user enter to the program
        len_of_chars+=strlen(userStr);

        // check if the user want to open file and ech and if yes divide the command and the name of file
        divide_the_command_and_file_name(userStr);

        //copy the array of the user after we cut the name of the file if there is redirection
        strcpy(copy, the_command);

        // check if the commands the user enter use pipe
        if(strstr(the_command, "|")!=NULL){
            flag_pipe=1;
            cou_pipe++;
            //divide the commend before the pipe and after the pipe and enter ech one to 2d array
            divide_pipe(the_command);           

            //execvp 2 sons 
            pid_t son_left, son_right;

            int pipe_fd[2];
            if(pipe(pipe_fd)==-1){
                perror("Error, pipe:");
                exit(EXIT_FAILURE);
            }

            son_left=fork();
            if(son_left==-1){
                perror("Error, fork:");
                exit(EXIT_FAILURE);
            }
            if(son_left==0){ 
                close (pipe_fd[0]);
                dup2(pipe_fd[1], STDOUT_FILENO);
                close(pipe_fd[1]);
                execvp(left[0], left);
                exit(1);
            }
            close(pipe_fd[1]); 
            wait(NULL);
            // free mallocs of cpmmand right
            for(int i=0; i<cou_left; i++){
                free(left[i]);
            }
            free(left);

            son_right=fork();
            if(son_right==-1){
                perror("Error, fork:");
                exit(1);
            }
            if(son_right==0){ 
                dup2(pipe_fd[0], STDIN_FILENO);
                close(pipe_fd[0]);

                if(redirection_flag==1){
                    if(flag_output==1){
                       fileDiscriptor=open(fileName,O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRGRP | S_IROTH);
                        if(fileDiscriptor==-1){
                            perror("Error, open file:");
                            exit(1);
                        }
                        dup2(fileDiscriptor, STDOUT_FILENO);
                        close(fileDiscriptor);
                    }

                    else if(flag_err==1){
                        fileDiscriptor=open(fileName,O_WRONLY | O_CREAT, S_IRWXU | S_IRGRP | S_IROTH);
                        if(fileDiscriptor==-1){
                            perror("Error, open file:");
                            exit(1);
                        }
                        dup2(fileDiscriptor, STDERR_FILENO);
                        close(fileDiscriptor);
                    }

                    else{
                        fileDiscriptor=open(fileName,O_WRONLY | O_CREAT | O_APPEND, S_IRWXU | S_IRGRP | S_IROTH);
                        if(fileDiscriptor==-1){
                            perror("Error, open file:");
                            exit(1);
                        }
                        dup2(fileDiscriptor, STDOUT_FILENO);
                        close(fileDiscriptor);
                    } 
                }
                execvp(right[0], right);
                exit(1);
            }
            close(pipe_fd[0]); 
            wait(NULL);
            // free mallocs of cpmmand right
            for(int i=0; i<cou_right; i++){
                free(right[i]);
            }
            free(right);
        }
    


        if(flag_pipe==0){

            if (redirection_flag==0) {
                make_only_one_command(the_command);
            }

            else{

                //count how much words the user type
                char *token=strtok(copy, " ");
                while(token!=NULL){                         
                    token=strtok(NULL, " ");     
                    couWord++;
                }

                /* new 2d array that save what the user type */
                arr_for_exec=(char**)malloc((couWord+1)*(sizeof (char*)));
                if(arr_for_exec==NULL){
                    perror("ERROR: could not allocate memory!\n");
                    exit(1);
                }

                // make 2D array frome 1D array
                arr_for_exec=array_to_2D_array(the_command, arr_for_exec, couWord+1);

                pid_t son=fork();
                if(son<0){
                    perror("ERROR: could not allocate memory!\n ");
                    exit(1);
                }
                if(son==0){
                    
                    // case redirection
                    if(redirection_flag==1){

                        // case ">"
                        if(flag_output==1){
                           fileDiscriptor=open(fileName,O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRGRP | S_IROTH);
                            if(fileDiscriptor==-1){
                                perror("Error, open file:");
                                exit(1);
                            }
                            dup2(fileDiscriptor, STDOUT_FILENO);
                            close(fileDiscriptor);
                        }
                        // case "2>"
                        else if(flag_err==1){
                            fileDiscriptor=open(fileName,O_WRONLY | O_CREAT, S_IRWXU | S_IRGRP | S_IROTH);
                            if(fileDiscriptor==-1){
                                perror("Error, open file:");
                                exit(1);
                            }
                            dup2(fileDiscriptor, STDERR_FILENO);
                            close(fileDiscriptor);
                        }

                        // case ">>"
                        else if(flag_output==2){
                            fileDiscriptor=open(fileName,O_WRONLY | O_CREAT | O_APPEND, S_IRWXU | S_IRGRP | S_IROTH);
                            if(fileDiscriptor==-1){
                                perror("Error, open file:");
                                exit(1);
                            }
                            dup2(fileDiscriptor, STDOUT_FILENO);
                            close(fileDiscriptor);
                        }
                        // case "<"
                        else{
                            fileDiscriptor=open(fileName,O_WRONLY | O_CREAT | O_APPEND, S_IRWXU | S_IRGRP | S_IROTH);
                            if(fileDiscriptor==-1){
                                perror("Error, open file:");
                                exit(1);
                            }
                            dup2(fileDiscriptor, STDOUT_FILENO);
                            close(fileDiscriptor);
                        }
                        
                    }
                    execvp(arr_for_exec[0], arr_for_exec);
                    exit(1);
                }
                else{
                    wait(NULL);
                    // free al mallocs
                    for(int i=0; i<couWord; i++)
                        free(arr_for_exec[i]);
                    free(arr_for_exec);
                }
            }
        }
            
            
        

        commends++;
    
    }
    printf("Num of commends: %d\n", commends);
    // +4 is the lengt of the commend "done"
    printf("Total length of all commends: %d \n", len_of_chars);
    average=(double)((double)len_of_chars/(double)commends);
    printf("Average length of all commends: %f,\n", average);
    printf("Number of command tht include pipe: %d \n", cou_pipe);
    printf("Number of command tht include redirection: %d \n", cou_redirections);
    printf("See you Next time !");


    return 0;   
}



                //// case "<"
                //else{
//
                //    fileDiscriptor=open(fileName,O_WRONLY | O_CREAT | O_APPEND, S_IRWXU | S_IRGRP | S_IROTH);
                //    if(fileDiscriptor==-1){
                //        perror("Error, open():");
                //        exit(1);
                //    }
                //    dup2(fileDiscriptor, STDOUT_FILENO);
                //    close(fileDiscriptor);
                //}      




