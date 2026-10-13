#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

int main(int argc, char *argv[]) {

    int n;
    int max_rounds;
    int voter_id = -1;
    int vote = 0; 

    //Step 1: Ask user to input # of voters and max # of rounds
    //N is specified by the user (should be equal or greater than 3)
    printf("Enter number of voters: ");
    scanf("%d", &n);
    if(n < 3)
    {
        printf("Number of voters should be at least 3\n");
        return 1;
    }

    printf("Enter max number of rounds: ");
    scanf("%d", &max_rounds);
    if(max_rounds < 1)
    {
        printf("Max rounds should be at least 1\n");
        return 1;
    }

    //Step 2: Judge creates N pipes using pipe() system call
    setbuf(stdout, NULL);
    
    int pipefd[n][2];

    for(size_t i = 0; i < n; i++)
    {
        if (pipe(pipefd[i]) == -1) {
            perror("pipe");
            exit(1);
        }
    }

    //Step 3: Judge creates N child processes using fork()
    //Judge is parent process, voters are children processes
    if(voter_id < 0)
    {
        for(size_t i = 0; i < n; i++)
        {
            //create voter process
            pid_t pid = fork();
            if(pid < 0)
            {
                perror("fork");
                exit(1);
            }

            if(pid == 0)
            {
                //child process
                voter_id = i;

                //initializes the rand function with seed
                srand(getpid());

                //Child does not need the read side
                close(pipefd[i][0]);

                break;
            }
        }
    }

    //Step 4: At each round, judge collects votes, each voter votes
    //voting continues until it reaches max # of rounds
    for(int i = 0; i < max_rounds; i++)
    {
        int total_votes_yes = 0;
        int total_votes_no = 0;

        //judge process
        if(voter_id < 0)
        { 
            printf("------ Round %d -------\n", i + 1);
            //4th for loop - judge process collects votes from each voter
            for(int j = 0; j < n; j++)
            {
                if (read(pipefd[j][0], &vote, sizeof(int)) > 0)
                {
                    if(vote == 1)
                    {
                        total_votes_yes++;
                    }
                    else if (vote == 0)
                    {
                        total_votes_no++;
                    }
                }  
                
                //display each voter's vote
                printf("--- Round %d: Voter %d votes: ", i + 1, j);
                if(vote > 0)
                {
                    printf("YES.\n");
                }
                else
                {
                    printf("NO.\n");
                }

            }

            //display results from each round
            printf("--- Round %d: The voting result is a ", i + 1);
            if(total_votes_yes > total_votes_no)
            {
                printf("YES.\n");
            }
            else if(total_votes_no > total_votes_yes)
            {
                printf("NO.\n");
            }
            else
            {
                printf("TIE.\n");
            }
        }

        //child process
        if(voter_id >= 0 && voter_id < n)
        {
            int v = rand() % 2;

            write(pipefd[voter_id][1], &v, sizeof(int));

            sleep(1);
        } 
    }
    
    //all processes should terminate properly before exiting
    if (voter_id == -1)
    {
        for (int i = 0; i < n; i++)
        {
            wait(NULL);
        }
    }

    return 0;
}