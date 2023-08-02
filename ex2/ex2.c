
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>
#include <fcntl.h>

#define MAX_COMMAND_LENGTH 510
#define MAX_ARGUMENTS 10
#define PROMPT "#cmd:%d|#args:%d@%s>"
#define FAILURE (1)
#define SUCCESS (0)
pid_t pid;
pid_t pid2;
int RDFlag , MpsFlag;
typedef enum  {
    REGULAR_COMMAND,
    CD_COMMAND
    ,VER_COMMAND,
    ENV_VAR,
    READ_DIRECT,
    PIPE_COMMAND,
    BG_COMMAND,
}CommandStatus;

typedef struct node {
    char *key;
    char *data;
    struct node *next;
} Node;

Node *head = NULL;

void insert(char *key, char *data);
void print_list();
char *find_element(char *key);
void deleteList();
int printVar (char* str);


/* service functions to handle the system */
static char *PrepareBuffForExecution(char** args,char *singleCommand);
static void ExecuteCommand(char **arguments, char *filename , int numOfPipes);
static CommandStatus GetCommandStatus(char *singleCommand);
static int IsCdCommand(char *singleCommand);
static void HandleCommandLine(CommandStatus command, char* singleCommand);
static int HasDollarSign(char *singleCommand);
static void RunCommand(char **args,char* singleCommand);
static void ExecuteDollarSignCommand(char** args, char*  singleCommand);
int ConvertToEnvPattern(char *dest, char* src);
static int countSpaces(char *cmd);
static int IsReadDirect(char *singleCommand);



static int equalCheck (char *str)
{
    for (int i = 0 ; str[i] != '\0'&& i < 510; i++)
        if(str[i] == '=')
            return i;
}


int numOfSpace(char *str)
{

}
//Accepts an array with the valid command and creates a process and dispatches with the execvp function
void ExecuteCommand(char **arguments , char *fileName , int numOfPipes ) {
    char pipeOrders[510];
    char *commandPipes[numOfPipes + 1];
    int **pipes;
    int *currPiP;
    int *prevPiP;
    if (numOfPipes > 0) {
        pipes = (int **) malloc(sizeof(int *) * numOfPipes);
        for (int i = 0; i < numOfPipes; ++i) {
            pipes[i] = (int *) malloc(sizeof(int) * 2);

        }
        for (int i = 0; i < numOfPipes; ++i) {
            if (pipe((pipes[i])) == -1) {
                perror("pipe");
                exit(1);
            }
        }
        currPiP = pipes[0];
        prevPiP = pipes[0];

        strcpy(pipeOrders, arguments[0]);
        int i = 0;
        commandPipes[i] = strtok(pipeOrders, "|");
        while (i <= numOfPipes) {
            i++;
            commandPipes[i] = strtok(NULL, "|");

        }


    }

    for (int i = 0; i <= numOfPipes; i++) {
        if (numOfPipes > 0) {
            int j = 0;
            int numSpace = countSpaces(commandPipes[i]);
            arguments[j] = strtok_r(commandPipes[i], " ", &commandPipes[i]);
            while (j < numSpace) {
                j++;
                arguments[j] = strtok_r(NULL, " ", &commandPipes[i]);
            }
            arguments[j + 1] = NULL;
        }
        pid = fork();

        if (pid == -1) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {    // child process
            // for ctrl Z
            signal(SIGTSTP, SIG_DFL);

// read direct care:
            if (fileName != NULL) {
                fileName = strtok(fileName, "\n");
                int fd = open(fileName, O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU);
                if (fd < 0) {
                    perror("open");
                }
                if (dup2(fd, STDOUT_FILENO) == -1) {
                    perror("dup2");
                }

            }

            if (numOfPipes > 0) {
                if (i == 0) {
                    close(currPiP[0]);
                    if ((dup2(currPiP[1], STDOUT_FILENO)) == -1) {
                        perror("dup2");
                        exit(1);
                    }


                } else if (i == numOfPipes) {
                    prevPiP = pipes[i - 1];
                    close(prevPiP[1]);
                    if ((dup2(prevPiP[0], STDIN_FILENO)) == -1) {
                        perror("dup2");
                        exit(1);
                    }

                }
                if (i > 0 && i < numOfPipes) {
                    currPiP = pipes[i];
                    prevPiP = pipes[i - 1];
                    close(prevPiP[1]);
                    if ((dup2(prevPiP[0], STDIN_FILENO)) == -1) {
                        perror("dup2");
                        exit(1);
                    }


                    close(currPiP[0]);
                    if ((dup2(currPiP[1], STDOUT_FILENO)) == -1) {
                        perror("dup2");
                        exit(1);
                    }

                }
            }
            if ((execvp(arguments[0], arguments)) == -1) {
                perror("execvp failed");
                exit(EXIT_FAILURE);
            }


        } else {
            if (numOfPipes > 0) {
                currPiP = pipes[i];
                prevPiP = pipes[i - 1];
                if (i == numOfPipes) {
                    close(prevPiP[0]);
                } else if (i == 0) {
                    close(currPiP[1]);
                }
                if ((i > 0) && (i < numOfPipes)) {
                    close(prevPiP[0]);
                    close(currPiP[1]);
                }
            }
            if (MpsFlag == 0) { waitpid(pid, NULL, WUNTRACED); }
            MpsFlag = 0;
        }


    }
    if (numOfPipes > 0) {
    for (int j = 0; j < numOfPipes; ++j) {
        free(pipes[j]);
    }
    free(pipes);
}
}

//Prepare the Buffer For Execution
static char *PrepareBuffForExecution(char** args,char *singleCommand)
{
    int num_args = 0;
    char *arg;
    arg = strtok(singleCommand, " \t\n\r\f\v");

    while (arg != NULL && num_args < MAX_COMMAND_LENGTH / 2)
    {
        args[num_args++] = arg;
        arg = strtok(NULL, " \t\n\r\f\v");
    }
    args[num_args] = NULL;
}

static int IsPipeCommand(char *singleCommand)
{
   int countPipes = 0;
    for (int i = 0; singleCommand[i] != '\0'; ++i) {
        if( singleCommand[i] == '|')
          countPipes++;
    }
    return countPipes;
}

static int IsCdCommand(char *singleCommand)
{
    /* 0 for true, non zero for false */
    return !strncmp(singleCommand,"cd",2);
}

static int IsBGCommand(char *singleCommand)
{
    /* 0 for true, non zero for false */
    return !strncmp(singleCommand,"bg",2);
}
static int HasDollarSign(char *singleCommand)
{
    return strchr(singleCommand, '$') != NULL;
}

static int IsReadDirect(char *singleCommand)
{
    return strchr(singleCommand, '>') != NULL;
}
static int IsEchoCommand(char *singleCommand)
{
    /* 0 for true, non zero for false */
    return !strncmp(singleCommand,"echo",4);
}


void deleteQuots(char *str) {
    int len = strlen(str);
    int i, j;
    for (i = 0, j = 0; i < len; i++)
        if (str[i] != '\"')
            str[j++] = str[i];
    str[j] = '\0';
}


void insertVerToTheList (int index,char *str)
{
    char Key[index + 1];
    char data[strlen(str) - index];

    memcpy(Key, &str[0], index * sizeof(char));
    Key[index]= '\0';
    memcpy(data, &str[index+1], (strlen(str)-index) * sizeof(char));
    data[(strlen(str)-index)] = '\0';

    insert(Key,&data[0]);
}



/* return 0 in case of non-special command or the required command */
static CommandStatus GetCommandStatus(char *singleCommand)
{
    if (IsPipeCommand(singleCommand) > 0)
    {
        return PIPE_COMMAND;
    }
    if (IsCdCommand(singleCommand))
    {
        return CD_COMMAND;
    }
    if (IsBGCommand(singleCommand))
    {
        return BG_COMMAND;
    }
    else if(IsReadDirect(singleCommand))
    {
        RDFlag = 1;
       return READ_DIRECT;
    }

    else if (HasDollarSign(singleCommand))
    {
        return ENV_VAR;
    }

    else if(IsEchoCommand(singleCommand))
    {
        deleteQuots(singleCommand);
    }

    else if (equalCheck(singleCommand) > 0 ){
        int equalInd = equalCheck(singleCommand);
        insertVerToTheList(equalInd,singleCommand);
        return VER_COMMAND;
    }
    return REGULAR_COMMAND;
}



static void RunCommand(char **args,char* singleCommand)
{
    PrepareBuffForExecution(args,singleCommand);
    ExecuteCommand(args , NULL, 0);
}
static void RunRDCommand(char **args,char* singleCommand, char* fileName)
{
    PrepareBuffForExecution(args,singleCommand);
    ExecuteCommand(args , fileName, 0);
}

int ConvertToEnvPattern(char *dest, char* src)
{
    /* str tok changes the string.... */
    char srcCpy[MAX_COMMAND_LENGTH + 1];
    strcpy(srcCpy,src);

    char* p = strchr(srcCpy, '$');

    size_t dollarIdx = p - srcCpy;

    strncpy(dest,src,dollarIdx);

    char* envName = strtok(p + 1, " \t\n\r\f\v");

    char *data = find_element(envName);

    if (data == NULL)
    {
        return FAILURE;
    }

    strcpy(dest + dollarIdx, data);
    strcpy(dest + dollarIdx + strlen(envName) + 1, src + dollarIdx + strlen(envName) + 1);

    return SUCCESS;
}


static void ExecutePipeCommand(char** args, char*  singleCommand)
{
    args[0] = singleCommand;
    ExecuteCommand(args,NULL, IsPipeCommand(singleCommand)) ;//TODO
}
static void ExecuteReadDirectCommand(char** args, char*  singleCommand)
{

    char* fileName;
    singleCommand = strtok(singleCommand, ">");
    fileName =  strtok(NULL, ">");
    fileName =  strtok(fileName, " ");
    RunRDCommand(args,singleCommand, fileName);
}

static void ExecuteDollarSignCommand(char** args, char*  singleCommand)
{
    deleteQuots(singleCommand);
    char converedBuff[MAX_COMMAND_LENGTH + 1];

    ConvertToEnvPattern(converedBuff,singleCommand);
    RunCommand(args,converedBuff);
}

static void HandleCommandLine(CommandStatus command, char* singleCommand)
{
    char *args[MAX_COMMAND_LENGTH / 2 + 1];

    switch (command)
    {
        case BG_COMMAND:
            kill(pid2,SIGCONT);
            break;
        case CD_COMMAND:
            printf("cd not supported\n");
            break;
        case ENV_VAR:
            ExecuteDollarSignCommand(args,singleCommand);
            break;
        case VER_COMMAND:
            break;
        case READ_DIRECT:
            ExecuteReadDirectCommand(args,singleCommand);
            break;
        case PIPE_COMMAND:
            ExecutePipeCommand(args,singleCommand);
            break;
        default:
            RunCommand(args,singleCommand);


            break;
    }
}


static void ExecuteComment(int *sum_args, char *singleCommand)
{
    singleCommand[strcspn(singleCommand,"\n")] = '\0';
    CommandStatus commandStatus = GetCommandStatus(singleCommand);

    HandleCommandLine(commandStatus,singleCommand);
}

static int countSpaces(char *cmd)
{
    int counter = 0;

    while (*cmd != '\0')
    {
        if(*cmd =='\"')
        {
            ++cmd;
            ++counter;
            while (*cmd !='\"')
            {
                ++cmd;
            }
        }
        if (*cmd == ' ')
        {
            ++counter;
        }

        ++cmd;
    }

    return counter;
}
void signalHandler(int signum)
{
    if(signum == SIGTSTP)
    {
        pid2 = pid;
    }
    waitpid(-1,NULL,WNOHANG);
}

int main() {
    char command[MAX_COMMAND_LENGTH + 1];  // +1 for null terminator
    char* commands[MAX_COMMAND_LENGTH / 2 + 1];  // +1 for NULL sentinel
    char cwd[1024];
    char* token;
    int sum_commands = 0;
    int num_commands = 0;
    int sum_args = 0;
    int i;


    signal(SIGTSTP,signalHandler); // ignore ctrl Z //TODO signal handler

    while (1)
    {
        // Print prompt
        getcwd(cwd, sizeof(cwd));
        printf(PROMPT, sum_commands, sum_args, cwd);

        // Read input
        if (!fgets(command, sizeof(command), stdin)) {
            perror("fgets failed");
            exit(EXIT_FAILURE);
        }

        // Exit if user presses enter 3 times
        if (strcmp(command, "\n") == 0) {
            if (++num_commands == 3) {
                printf("Exiting shell...\n");
                break;
            }
            continue;
        } else {
            num_commands = 0;
        }

        MpsFlag = 0;

i =0;
        char tempComand[520];
        int quotFlag = 0;
        for (int j = 0 , k = 0; j <= strlen(command); ++j) {

            if(command[j] == '\"')
            {
                if(quotFlag == 0)
                    quotFlag++;
                else
                    quotFlag--;
            }

            if((command[j] == ';'|| command[j] == '\0') && quotFlag == 0)
            {
                tempComand[k] = '\0';
//                strcpy(commands[i],tempComand);
                ExecuteComment(&sum_args,tempComand);
              sum_args = sum_args + countSpaces(command) + 1;
                i++;
                sum_commands += i;
                k = 0;
            } else
            {
                if(command[j] == '&')
                {
                    MpsFlag = 1;
                    continue;
                }
                tempComand[k] = command[j];
                k++;
            }
        }

    }


    deleteList();

    return 0;
}

void insert(char *key, char *data) {
    Node *new_node = (Node *)malloc(sizeof(Node));
    new_node->key = (char *)malloc(strlen(strtok(key," ")) + 1);
    strcpy(new_node->key,  key);
    new_node->data = (char *)malloc(strlen(strtok(data, "\n")) + 1);
    strcpy(new_node->data, strtok(data," \n"));
    new_node->next = NULL;

    if (head == NULL) {
        head = new_node;
    } else {
        Node *current = head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }
}

void print_list() {
    Node *current = head;
    while (current != NULL) {
        printf("Key: %s, Data: %s\n", current->key, current->data);
        current = current->next;
    }
}

char *find_element(char *key) {
    Node *current = head;
    while (current != NULL) {
        if (strcmp(current->key , key) == 0) {
            return current->data;
        }
        current = current->next;
    }
    return NULL;
}



void deleteList()
{
    Node *current = head;
    Node *next;
    while (current != NULL) {
        next = current->next;
        free(current->key);
        free(current->data);
        free(current);
        current = next;
    }
    for (int i = 0;; ++i) {

    }
}






