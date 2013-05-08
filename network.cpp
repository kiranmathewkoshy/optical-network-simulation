#include<windows.h>
#include<stdio.h>
#include<iostream>
#include<stdlib.h>
#include<process.h>
#include<time.h>
#include<conio.h>
#include<math.h>
#include<string.h>
#define NO_OF_NODES 14
#define NO_OF_CLIENT_NODES 11
#define NO_OF_DATA_CENTERS 3
#define NO_OF_REGENERATORS 5
#define NO_OF_REGENERATOR_WAVELENGTHS 10
#define NO_OF_AVAILABLE_WAVELENGTHS 140
#define MAXIMUM_DURATION_OF_A_CALL 10
#define MAXIMUM_NO_OF_CALLS_TO_PROCESS 100000
#define FILE_INTERVAL 10000
#define OUTPUT_FILENAME "output.txt"
#define INPUT_FILENAME "a.txt"
#define NOT_INITIALISED  INT_MIN
#define CALL_FREQUENCY 150
#define ENABLE_PROGRESS_BAR 1
#define REGENERATOR_ON 1
#define REGENERATOR_LOGIC_LAMBDA 0.2000
#define REGENERATOR_LOGIC_ALPHA 0.3000
#define REGENERATOR_LOGIC_THRESHOLD_SIGNAL_STRENGTH 0.25
#define REGENERATOR_LOGIC_INITIAL_SIGNAL_STRENGTH 1.00
#define REGENERATOR_NOISE_THRESHOLD 600.00
#define RANDOMIZE_CALL_TIMING 0
#define LAMBDA_EXP 0.6
#define ENABLE_RANDOM_NODE_FAILURE 0
#define ENABLE_DATACENTER_FAILURE 1
#define ENABLE_CLIENT_NODE_FAILURE 1
#define NODE_FAILURE_POINT 50000
using namespace std;
int *POISSONS_RATIO;
int SIGMA_N;
CRITICAL_SECTION cs1,cs2,TIME_CRITICAL;
int *data_centers;
int *nodes;
int *regenerators;
int STOP_PROCESS;
unsigned NO_OF_ELEMENTS_IN_PATH;
unsigned long long CALL_NO;
unsigned long long SYSTEM_TIME;
long double n_total;
long NO_OF_ELEMENTS_IN_STACK_1;
long NO_OF_ELEMENTS_IN_STACK_2;
long NO_OF_ELEMENTS_IN_STACK_3;
long NO_OF_ELEMENTS_IN_MEMORY_REUSE_STACK;
unsigned long long NO_OF_CALLS_ALLOCATED;
unsigned long long NO_OF_CALLS_BLOCKED;
unsigned long long delayVariable;
FILE *fp;
void initialise();
void initialise_connection_settings();
void display_connection_status();
void display_graph_status();
void initialise_graph_weight();
int random_node_failure();
struct call_data
{
    unsigned int source;
    unsigned int destination;
    unsigned long long time;
    unsigned long long time_of_origin;
    int path_info[NO_OF_NODES];
    int wavelength[NO_OF_NODES];
    int NO_OF_NODES_IN_PATH;
    bool Allocated;
    unsigned long long  CALL_ID;
    call_data *next;
    double n;
};
call_data *call_data_top,*call_data_end,*call_data_p,*call_data_temp;
call_data *path_1,*path_2;
call_data *path_top,*path_end,*path_p,*path_temp;
call_data *main_queue_top,*main_queue_end,*main_queue_p,*main_queue_temp;
call_data *op_data_top,*op_data_end,*op_data_p,*op_data_temp;
call_data *memoryReuseQueue_top,*memoryReuseQueue_end,*memoryReuseQueue_p,*memoryReuseQueue_temp;
struct nod
{
    int status;
    int predecessor;
    int pdc;
    int bdc;
};
int graph1[NO_OF_NODES][NO_OF_NODES];
int path_data1[NO_OF_NODES];
int graph2[NO_OF_NODES][NO_OF_NODES];
int path_data2[NO_OF_NODES];
float graph_weight[NO_OF_NODES][NO_OF_NODES];
unsigned long long _network_status[NO_OF_NODES][NO_OF_NODES][NO_OF_AVAILABLE_WAVELENGTHS];
unsigned long long _regenerator_status[NO_OF_NODES][NO_OF_REGENERATOR_WAVELENGTHS];
struct connection
{
    int node1;
    int node2;
};
connection connection_list[NO_OF_NODES][NO_OF_NODES];
int connection_list_size[NO_OF_NODES];
class progress_bar
{
private:
    char icon;
    int max_icons;
    unsigned long long max_no;
    int no;
    unsigned long long count;
    int is_initialised;
    double displayed,count_max;
public:
    progress_bar(char icon_,unsigned long long max_n)
    {
        if(max_n<1)
        {
            is_initialised=-1;
            return;
        }
        if(max_n<1)
        {
            is_initialised=-1;
            return;
        }
        is_initialised=1;
        max_icons=80;
        icon=icon_;
        max_no=max_n;
        no=0;
        count=0;
        count_max=0.0;
        displayed=0.0;
        return;
    }
    void report()
    {
        if(is_initialised!=1)
            return;
        count++;
        displayed=(no+1)/80.00;
        count_max=(count*1.0)/max_no;
        if(count_max>displayed)
        {
            cout<<icon;
            no++;
        }
        if(no>=80)
            is_initialised=-1;
        return;
    }
    void report_value(unsigned long long value)
    {
        if(is_initialised!=1)
            return;
        count=value;
        displayed=(no+1)/80.00;
        count_max=(count*1.0)/max_no;
        if(count_max>displayed)
        {
            cout<<icon;
            no++;
        }
        if(no>=80)
            is_initialised=-1;
        return;
    }
    void initialise(char icon_,int max_n)
    {
        if(max_n<1)
        {
            is_initialised=-1;
            return;
        }
        if(max_n<1)
        {
            is_initialised=-1;
            return;
        }
        is_initialised=1;
        max_icons=80;
        icon=icon_;
        max_no=max_n;
        no=0;
        count=0;
        count_max=0.0;
        displayed=0.0;
        return;
    }
};
progress_bar progress('#',MAXIMUM_NO_OF_CALLS_TO_PROCESS);
class Queue
{
    int queue[NO_OF_NODES],front,rear,no_of_elements;
public:
    Queue()
    {
        front=rear=-1;
        no_of_elements=0;
    }
    void initialise()
    {
        front=rear=-1;
        no_of_elements=0;
    }
    int enqueue(int d)
    {
        if ((front==0&&rear==NO_OF_NODES)||(front==(rear+1)))
            return -1;
        else
        {
            if(front==-1&&rear==-1)
            {
                rear=0;
                front=0;
            }
            else if(front!=0&&rear==NO_OF_NODES)
                rear=0;
            else
                rear++;
            queue[rear]=d;
        }
        no_of_elements++;
        return 1;
    }
    int dequeue()
    {
        if(front==-1&&rear==-1)
            return -1;
        else
        {
            if(front==rear)
                front=rear=-1;
            else if(front==NO_OF_NODES&&rear==NO_OF_NODES)
                front=0;
            else
                front++;
        }
        no_of_elements--;
        return 1;
    }
    int get_top()
    {
        return queue[front];
    }
    bool is_empty()
    {
        if(no_of_elements==0)
            return true;
        else
            return false;
    }
} q1,q2;
inline bool is_data_center(int node)
{
    for(int i=0; i<NO_OF_DATA_CENTERS; i++)
    {
        if(data_centers[i]==node)
            return true;
    }
    return false;
}
inline bool is_regenerator(int node)
{
    for(int i=0; i<NO_OF_REGENERATORS; i++)
    {
        if(regenerators[i]==node)
            return true;
    }
    return false;
}
inline int* path1(int client_node)
{
    nod node[NO_OF_NODES];
    int adj[NO_OF_NODES];
    int found_primary;
    int last=-1;
    int no_of_adj;
    int i,j;
    for(i=0; i<NO_OF_NODES; i++)
    {
        adj[i]=-1;
        path_data1[i]=-1;
    }
    found_primary=-1;
    for(i=0; i<NO_OF_NODES; i++)
    {
        node[i].status=0;
        node[i].predecessor=-1;
    }
    q1.initialise();
    q1.enqueue(client_node);
    node[client_node].status=1;
    if(is_data_center(client_node)==true)
        return NULL;
    last=client_node;
    while(q1.is_empty()==false)
    {
        node[client_node].status=1;
        while(found_primary==-1)
        {
            {
                int i,n,j,tmp;
                n=q1.get_top();
                i=0;
                j=0;
                for(i=0; i<NO_OF_NODES; i++)
                {
                    if(graph1[n][i]>0)
                    {
                        if(node[i].status==0)
                        {
                            adj[j]=i;
                            j++;
                        }
                    }
                }
                no_of_adj=j;
                for(i=0; i<no_of_adj; i++)
                {
                    for(j=0; j<(no_of_adj-1); j++)
                    {
                        if(graph1[n][adj[j]]>graph1[n][adj[j+1]])
                        {
                            tmp=adj[j];
                            adj[j]=adj[j+1];
                            adj[j+1]=tmp;
                        }
                    }
                }
            }
            last=q1.get_top();
            for(i=0; i<no_of_adj; i++)
            {
                node[adj[i]].status=1;
                q1.enqueue(adj[i]);
                node[adj[i]].predecessor=last;
                if(is_data_center(adj[i])==true&&found_primary==-1)
                {
                    node[client_node].pdc=adj[i];
                    found_primary=1;
                    for(j=0; j<NO_OF_NODES; j++)
                        node[j].status=0;
                    int temp=adj[i],k;
                    k=0;
                    while(temp!=client_node)
                    {
                        path_data1[k]=temp;
                        k++;
                        if(node[temp].predecessor==-1)
                            return &path_data1[0];
                        node[temp].status=1;
                        temp=node[temp].predecessor;
                    }
                    path_data1[k]=client_node;
                    NO_OF_ELEMENTS_IN_PATH=++k;
                    node[client_node].status=1;
                    return &path_data1[0];
                }
            }
            q1.dequeue();
        }
    }
    return &path_data1[0];
}
inline void Allocate(call_data *p)
{
    int x,y,i,j,k;
    unsigned long long t;
    int samplePath[NO_OF_NODES];
    int no_Of_Elements_In_SamplePath=0;
    int regenerator_wavelength;
    int wavelength_list[NO_OF_NODES];
    bool flag=false;
    bool allocated=false;
    y=1;
    int wavelength[NO_OF_NODES];
    for(i=0; i<NO_OF_AVAILABLE_WAVELENGTHS; i++)
    {
        if(_network_status[p->path_info[0]][p->path_info[1]][i]<SYSTEM_TIME)
        {
            for(j=1; j<p->NO_OF_NODES_IN_PATH-1; j++)
            {
                t=SYSTEM_TIME;
                if(_network_status[p->path_info[j]][p->path_info[j-1]][i]>t)
                {
                    goto next_wavelength;
                }
            }
            t=(SYSTEM_TIME+(p->time));
            for(k=1; k<(p->NO_OF_NODES_IN_PATH); k++)
            {
                _network_status[p->path_info[k]][p->path_info[k-1]][i]=t;
                _network_status[p->path_info[k-1]][p->path_info[k]][i]=t;
                p->n+=REGENERATOR_LOGIC_LAMBDA+(REGENERATOR_LOGIC_ALPHA*graph_weight[p->path_info[k]][p->path_info[k-1]]);
                if((p->n)>REGENERATOR_NOISE_THRESHOLD)
                {
                    allocated=false;
                    for(int l=0;l<NO_OF_REGENERATORS;l++)
                    {
                        if(_regenerator_status[p->path_info[k-1]][l]<SYSTEM_TIME)
                        {
                            allocated=true;
                            _regenerator_status[p->path_info[k-1]][l]=SYSTEM_TIME;
                            p->n=2;
                            break;
                        }
                    }
                    if(!allocated)
                        goto next_wavelength;
                }

            }
            n_total+=p->n;
            p->wavelength[0]=i;
            NO_OF_CALLS_ALLOCATED++;
            p->Allocated=true;
            return;
        }
next_wavelength:
        ;
    }
    if(!REGENERATOR_ON)
        goto block_call;
    if(p->NO_OF_NODES_IN_PATH<=2)
        goto block_call;
    if(p->NO_OF_NODES_IN_PATH==3)
    {
        if(is_regenerator(p->path_info[1])==false)
            goto block_call;
        flag=false;
        for(i=0; i<NO_OF_REGENERATOR_WAVELENGTHS; i++)
        {
            if(_regenerator_status[p->path_info[1]][i]<SYSTEM_TIME)
            {
                regenerator_wavelength=i;
                break;
            }
        }
        for(i=0; i<NO_OF_AVAILABLE_WAVELENGTHS; i++)
        {
            if(_network_status[p->path_info[0]][p->path_info[1]][i]<SYSTEM_TIME)
            {
                wavelength_list[0]=i;
                flag=true;
                break;
            }
        }
        if(flag==false)
            goto block_call;
        flag=false;
        for(i=0; i<NO_OF_AVAILABLE_WAVELENGTHS; i++)
        {
            if(_network_status[p->path_info[1]][p->path_info[2]][i]<SYSTEM_TIME)
            {
                wavelength_list[1]=i;
                flag=true;
                break;
            }
        }
        if(flag==false)
            goto block_call;
        t=(SYSTEM_TIME+(p->time));
        _network_status[p->path_info[0]][p->path_info[1]][i]=t;
        _network_status[p->path_info[1]][p->path_info[0]][i]=t;
        _regenerator_status[p->path_info[1]][regenerator_wavelength]=SYSTEM_TIME;
        NO_OF_CALLS_ALLOCATED++;
        p->wavelength[0]=wavelength_list[0];
        p->wavelength[1]=wavelength_list[1];
        p->Allocated=true;
        return;
    }
block_call:
    NO_OF_CALLS_BLOCKED++;
    p->Allocated=false;
}
void displayReport()
{
    cout<<"\n";
    cout<<"Simulation Complete";
    cout<<"\nBrief result of Calls Processed:\n";
    cout<<(NO_OF_CALLS_ALLOCATED+NO_OF_CALLS_BLOCKED)<<" Calls generated ";
    double blocking=(NO_OF_CALLS_BLOCKED*1.0)/MAXIMUM_NO_OF_CALLS_TO_PROCESS;
    cout<<"\nBLocking Probability= "<<blocking;
    cout<<"\n"<<NO_OF_CALLS_ALLOCATED<<" Calls Allocated to network ";
    cout<<"\n"<<NO_OF_CALLS_BLOCKED<<" Calls Blocked from network";

    //cout<<"\nTotal value of n= "<<n_total;
    //long double av=n_total/(MAXIMUM_NO_OF_CALLS_TO_PROCESS*1.0);
    //cout<<"\nAverage value of n= "<<av;
    char st[100];
    strcpy(st,"notepad.exe ");
    strcat(st,OUTPUT_FILENAME);
    //display_graph_status();
    //system(st);
    //getch();
}
void callCreation(void* pParams)
{
    int i,j,k;
    if(RANDOMIZE_CALL_TIMING==1)
        srand(time(NULL));
    int counter,SYS_TIME;
    float xx,temp_exp_no, yy;
    NO_OF_ELEMENTS_IN_STACK_1=0;
    call_data_p=new call_data;
    if(call_data_p==NULL)
        exit(2);
    i=rand()%NO_OF_DATA_CENTERS;
    call_data_p->source=data_centers[i];
    i=rand()%NO_OF_CLIENT_NODES;
    call_data_p->destination=nodes[i];
    k=rand()%SIGMA_N;
    call_data_p->time=POISSONS_RATIO[k];
    call_data_p->next=NULL;
    call_data_p->CALL_ID=CALL_NO;
    call_data_p->time_of_origin=SYSTEM_TIME;
    call_data_p->n=2.00;
    CALL_NO++;
    for(i=0; i<NO_OF_NODES; i++)
    {
        call_data_p->path_info[i]=-1;
        call_data_p->wavelength[i]=-1;
    }
    call_data_top=call_data_p;
    EnterCriticalSection(&cs1);
    call_data_end=call_data_p;
    NO_OF_ELEMENTS_IN_STACK_1++;
    LeaveCriticalSection(&cs1);
    SYS_TIME=SYSTEM_TIME;
    counter=0;
    while(1)
    {
call_loc:
        k=NO_OF_ELEMENTS_IN_STACK_1;
        if(SYS_TIME<SYSTEM_TIME)
        {
            counter=0;
            SYS_TIME=SYSTEM_TIME;
        }
        if(counter>CALL_FREQUENCY)
            goto call_loc;
        if(STOP_PROCESS==1)
        {
            j=NO_OF_ELEMENTS_IN_MEMORY_REUSE_STACK;
            if(j>5)
            {
                call_data_p= memoryReuseQueue_top;
                memoryReuseQueue_top=memoryReuseQueue_top->next;
                InterlockedDecrement(&NO_OF_ELEMENTS_IN_MEMORY_REUSE_STACK);
                delete call_data_p;
            }
            goto call_loc;
        }
        if(k>5)
        {
            j=NO_OF_ELEMENTS_IN_MEMORY_REUSE_STACK;
            if(j<5)
                call_data_p=new call_data;
            else
            {
                call_data_p= memoryReuseQueue_top;
                memoryReuseQueue_top=memoryReuseQueue_top->next;
                InterlockedDecrement(&NO_OF_ELEMENTS_IN_MEMORY_REUSE_STACK);
            }
            if(call_data_p==NULL)
                exit(2);
            i=rand()%NO_OF_DATA_CENTERS;
            call_data_p->source=data_centers[i];
            i=rand()%NO_OF_CLIENT_NODES;
            call_data_p->destination=nodes[i];
            k=rand()%SIGMA_N;
            call_data_p->time=POISSONS_RATIO[k];
            call_data_p->next=NULL;
            call_data_p->CALL_ID=CALL_NO;
            call_data_end->next=call_data_p;
            call_data_end=call_data_p;
            call_data_p->time_of_origin=SYSTEM_TIME;
            call_data_p->n=2.00;
            InterlockedIncrement(&NO_OF_ELEMENTS_IN_STACK_1);
            CALL_NO++;
        }
        else
        {
            j=NO_OF_ELEMENTS_IN_MEMORY_REUSE_STACK;
            if(j<5)
                call_data_p=new call_data;
            else
            {
                call_data_p= memoryReuseQueue_top;
                memoryReuseQueue_top=memoryReuseQueue_top->next;
                InterlockedDecrement(&NO_OF_ELEMENTS_IN_MEMORY_REUSE_STACK);
            }
            if(call_data_p==NULL)
                exit(2);
            i=rand()%NO_OF_DATA_CENTERS;
            call_data_p->source=data_centers[i];
            i=rand()%NO_OF_CLIENT_NODES;
            call_data_p->destination=nodes[i];
            k=rand()%SIGMA_N;
            call_data_p->time=POISSONS_RATIO[k];
            call_data_p->next=NULL;
            call_data_p->CALL_ID=CALL_NO;
            call_data_p->time_of_origin=SYSTEM_TIME;
            call_data_p->n=2.00;
            EnterCriticalSection(&cs1);
            if(call_data_top==NULL)
                call_data_top=call_data_p;
            call_data_end->next=call_data_p;
            call_data_end=call_data_p;
            NO_OF_ELEMENTS_IN_STACK_1++;
            LeaveCriticalSection(&cs1);
            CALL_NO++;
        }
        counter++;
    }
}
void pathFinder1(void* pParams)
{
    int *path_op,i,j,k;
    while(1)
    {
path1_point:
        k=NO_OF_ELEMENTS_IN_STACK_1;
        if(k<10)
            goto path1_point;
        if(STOP_PROCESS==1)
        {
            Sleep(50);
            goto path1_point;
        }
        path_1=call_data_top;
        call_data_top=call_data_top->next;
        InterlockedDecrement(&NO_OF_ELEMENTS_IN_STACK_1);
        //cout<<"\nCall Processed By Path_Finder "<<path_1->CALL_ID;
        path_op=path1(path_1->destination);
        j=0;
        path_1->source=path_op[0];
        for(i=0; i<NO_OF_NODES; i++)
        {
            path_1->path_info[i]=path_op[i];
        }
        path_1->NO_OF_NODES_IN_PATH=NO_OF_ELEMENTS_IN_PATH;
        if(path_top==NULL)
        {
            path_top=path_1;
        }
        else
            path_end->next=path_1;
        path_end=path_1;
        InterlockedIncrement(&NO_OF_ELEMENTS_IN_STACK_2);
        //cout<<"\nPath Calculated";
    }
}
void Allocator(void *pParams)
{
    int k;
    srand(time(NULL));
    while(1)
    {
Allocator_loc:
        k=NO_OF_ELEMENTS_IN_STACK_2;
        if(k<5)
            goto Allocator_loc;
        if(2==1)
        {
            Sleep(50);
            goto Allocator_loc;
        }
        path_temp=path_top;
        if(path_top!=NULL)
            path_top=path_top->next;
        InterlockedDecrement(&NO_OF_ELEMENTS_IN_STACK_2);
        if(path_temp->CALL_ID>MAXIMUM_NO_OF_CALLS_TO_PROCESS)
        {
            cout<<"\nComputation Completed\nSaving Data to File....\n";
            STOP_PROCESS=1;
            fclose(fp);
            cout<<"Output data saved to file.\nFilename: "<<OUTPUT_FILENAME;
            displayReport();
            exit(0);
        }
        if(path_temp->CALL_ID==NODE_FAILURE_POINT)
        {
            random_node_failure();
        }
        Allocate(path_temp);
        if(ENABLE_PROGRESS_BAR==1)
            progress.report();
        if(main_queue_top==NULL)
        {
            main_queue_top=path_temp;
        }
        else
            main_queue_end->next=path_temp;
        main_queue_end=path_temp;
        InterlockedIncrement(&NO_OF_ELEMENTS_IN_STACK_3);
    }
}
void systemTimeKeeper(void *pParams)
{
    while(1)
    {
        //EnterCriticalSection(&TIME_CRITICAL);
        SYSTEM_TIME++;
        Sleep(1);
        //if(SYSTEM_TIME%1000==0)
        //Sleep(1);
        //delayVariable*=2;
        //delayVariable*=3;
        //LeaveCriticalSection(&TIME_CRITICAL);
    }
}
void memoryReuse(void *pParams)
{
    int i,j,k;
    while(1)
    {
memoryReuse_start:
        k=NO_OF_ELEMENTS_IN_STACK_3;
        if(k<5)
            goto memoryReuse_start;
        main_queue_p=main_queue_top;
        main_queue_top=main_queue_top->next;
        InterlockedDecrement(&NO_OF_ELEMENTS_IN_STACK_3);
        if(main_queue_p->CALL_ID%FILE_INTERVAL==0)
            fflush(fp);
        fprintf(fp,"\n\nCALL ID: %llu",(main_queue_p->CALL_ID));
        fprintf(fp,"\nTime of origination of Call: %llu",(main_queue_p->time_of_origin));
        fprintf(fp,"\nSource(Node): %u",(main_queue_p->source+1));
        fprintf(fp,"\nDuration: %u",main_queue_p->time);
        fprintf(fp,"\nDestination: %u",(main_queue_p->destination+1));
        fprintf(fp,"\nValue of n= %f",(main_queue_p->n));
        fprintf(fp,"\nPath details: ");
        fprintf(fp,"No of Elements in path: %d",main_queue_p->NO_OF_NODES_IN_PATH);
        fprintf(fp,"\nNodes: ");
        for(int i=0; i<(main_queue_p->NO_OF_NODES_IN_PATH); i++)
            fprintf(fp,"%d ",(main_queue_p->path_info[i]+1));
        fprintf(fp,"\nStatus: ");
        if(main_queue_p->Allocated==true)
            fprintf(fp,"Allocated");
        else
            fprintf(fp,"Blocked");
        fprintf(fp,"\nWavelength allocated = %d",main_queue_p->wavelength[0]+1);
skip:
        ;
        if(memoryReuseQueue_top==NULL)
            memoryReuseQueue_top=main_queue_p;
        else
            memoryReuseQueue_end->next=main_queue_p;
        memoryReuseQueue_end=main_queue_p;
        InterlockedIncrement(&NO_OF_ELEMENTS_IN_MEMORY_REUSE_STACK);
    }
}
int random_node_failure()
{
    int node=-1;
    if(ENABLE_RANDOM_NODE_FAILURE!=1)
        return -1;
    if(ENABLE_CLIENT_NODE_FAILURE==1&&ENABLE_DATACENTER_FAILURE==1)
    {
        node= rand()%NO_OF_NODES;
    }
    else
    {
        if(ENABLE_CLIENT_NODE_FAILURE==1)
        {
            node=nodes[rand()%NO_OF_CLIENT_NODES];
        }
        if(ENABLE_DATACENTER_FAILURE==1)
        {
            node=data_centers[rand()%NO_OF_DATA_CENTERS];
        }
    }
    if(node==-1)
        return -1;
    for(int i=0; i<connection_list_size[node]; i++)
    {
        graph1[connection_list[node][i].node1][connection_list[node][i].node2]=0;
        graph1[connection_list[node][i].node2][connection_list[node][i].node1]=0;
    }
    cout<<"\nRandom Node Failure : activated\nNode number "<<(node+1)<<" terminated from network.\n";
    return 1;
}
int main()
{
    cout<<"Initialising...";
    initialise();
    cout<<"\nComputation Started\n";
    cout<<"Starting thread 1: Timekeeper\n";
    _beginthread(systemTimeKeeper,0,NULL);
    cout<<"Starting Thread 2: Call creation\n";
    _beginthread(callCreation,0,NULL);
    //Sleep(1);
    cout<<"Starting Thread 3: Path Finder\n";
    _beginthread(pathFinder1,0,NULL);
    //Sleep(1);
    cout<<"Starting Thread 4: Wavelength allocator\n";
    _beginthread(Allocator,0,NULL);
    //Sleep(1);
    cout<<"Starting Thread 5: Memory Reuser\n";
    _beginthread(memoryReuse,0,NULL);
    //getch();
    cout<<"Simulating....\n";
    Sleep(1000000);
    return 0;
}
int poisson_time(int i)
{
    float yy=i*1.0/MAXIMUM_DURATION_OF_A_CALL;
    float temp_exp_no=(-1.0*(1.0/LAMBDA_EXP)*log(1.00-yy));
    if (temp_exp_no==0.0)
        return 0;
    else
        return ceil(temp_exp_no);
}
void calculate_poissons_ratio()
{
    int n;
    n=0;
    for(int i=1; i<MAXIMUM_DURATION_OF_A_CALL; i++)
    {
        n+=poisson_time(i);
    }
    POISSONS_RATIO=new int[n];
    int *temp=new int[n];
    SIGMA_N=n;
    int count=0;
    for(int i=0; i<MAXIMUM_DURATION_OF_A_CALL; i++)
    {
        n=poisson_time(MAXIMUM_DURATION_OF_A_CALL-i);
        for(int j=0; j<n; j++)
        {
            POISSONS_RATIO[count]=i;
            count++;
        }
    }
    delete temp;
    return;
}
void initialise()
{
    int i,j,k;
    if(NO_OF_REGENERATORS>NO_OF_NODES)
    {
        cout<<"\nError in number of Regenerators/Nodes";
        abort();
    }
    InitializeCriticalSection(&cs1);
    InitializeCriticalSection(&cs2);
    InitializeCriticalSection(&TIME_CRITICAL);
    data_centers=new int[NO_OF_DATA_CENTERS];
    nodes=new int[NO_OF_CLIENT_NODES];
    regenerators= new int[NO_OF_REGENERATORS];
    srand(time(NULL));
    call_data_end=NULL;
    call_data_p=NULL;
    call_data_temp=NULL;
    call_data_top=NULL;
    path_top=NULL;
    path_end=NULL;
    path_p=NULL;
    path_temp=NULL;
    path_1=NULL;
    path_2=NULL;
    main_queue_top=NULL;
    main_queue_end=NULL;
    main_queue_p=NULL;
    main_queue_temp=NULL;
    op_data_top=NULL;
    op_data_end=NULL;
    op_data_p=NULL;
    op_data_temp=NULL;
    memoryReuseQueue_top=NULL;
    memoryReuseQueue_end=NULL;
    memoryReuseQueue_p=NULL;
    memoryReuseQueue_temp=NULL;
    NO_OF_ELEMENTS_IN_PATH=0;
    NO_OF_ELEMENTS_IN_STACK_1=0;
    NO_OF_ELEMENTS_IN_STACK_2=0;
    NO_OF_ELEMENTS_IN_STACK_3=0;
    NO_OF_ELEMENTS_IN_MEMORY_REUSE_STACK=0;
    NO_OF_CALLS_ALLOCATED=0;
    NO_OF_CALLS_BLOCKED=0;
    progress.initialise('#',MAXIMUM_NO_OF_CALLS_TO_PROCESS);
    fp=fopen(INPUT_FILENAME,"r");
    if(fp==NULL)
    {
        cout<<"\nFile Opening Failed";
        abort();
    }
    for(i=0; i<NO_OF_NODES; i++)
    {
        for (j=0; j<NO_OF_NODES; j++)
        {
            fscanf(fp,"%d",&graph1[i][j]);
        }
    }
    fclose(fp);
    for(i=0; i<NO_OF_NODES; i++)
    {
        for (j=0; j<NO_OF_NODES; j++)
        {
            graph2[i][j]=graph1[i][j];
        }
    }
    for(i=0; i<NO_OF_NODES; i++)
    {
        for (j=0; j<NO_OF_NODES; j++)
        {
            for(k=0; k<NO_OF_AVAILABLE_WAVELENGTHS; k++)
            {
                _network_status[i][j][k]=1;
            }
        }
    }
    for(i=0; i<NO_OF_NODES; i++)
    {
        for(j=0; j<NO_OF_REGENERATOR_WAVELENGTHS; j++)
        {
            _regenerator_status[i][j]=1;
        }
    }
    delayVariable=2;
    CALL_NO=1;
    SYSTEM_TIME=2;
    STOP_PROCESS=0;
    n_total=0.0;
    data_centers[0]=0;
    data_centers[1]=5;
    data_centers[2]=8;
    nodes[0]=1;
    nodes[1]=2;
    nodes[2]=3;
    nodes[3]=4;
    nodes[4]=6;
    nodes[5]=7;
    nodes[6]=9;
    nodes[7]=10;
    nodes[8]=11;
    nodes[9]=12;
    nodes[10]=13;
    regenerators[0]=1;
    regenerators[1]=3;
    regenerators[2]=4;
    regenerators[3]=9;
    regenerators[4]=13;
    regenerators[5]=6;
    for(i=0; i<NO_OF_NODES; i++)
    {
        for(j=0; j<NO_OF_NODES; j++)
        {
            connection_list[i][j].node1=-1;
            connection_list[i][j].node2=-1;
        }
    }
    for(i=0; i<NO_OF_NODES; i++)
    {
        connection_list_size[i]=0;
    }
    fp=NULL;
    fp=fopen(OUTPUT_FILENAME,"w");
    if(fp==NULL)
    {
        cout<<"\nFile Opening Failed";
        abort();
    }
    initialise_connection_settings();
    //display_connection_status();
    //display_graph_status();
    initialise_graph_weight();
    calculate_poissons_ratio();
    return;
}
void initialise_connection_settings()
{
    connection_list_size[0]=3;
    connection_list[0][0].node1=0;
    connection_list[0][0].node2=1;
    connection_list[0][1].node1=0;
    connection_list[0][1].node2=2;
    connection_list[0][2].node1=0;
    connection_list[0][2].node2=7;
    connection_list_size[1]=4;
    connection_list[1][0].node1=1;
    connection_list[1][0].node2=0;
    connection_list[1][1].node1=1;
    connection_list[1][1].node2=2;
    connection_list[1][2].node1=1;
    connection_list[1][2].node2=3;
    connection_list[1][3].node1=0;
    connection_list[1][3].node2=2;
    connection_list_size[2]=3;
    connection_list[2][0].node1=2;
    connection_list[2][0].node2=1;
    connection_list[2][1].node1=2;
    connection_list[2][1].node2=0;
    connection_list[2][2].node1=2;
    connection_list[2][2].node2=5;
    connection_list_size[3]=3;
    connection_list[3][0].node1=3;
    connection_list[3][0].node2=1;
    connection_list[3][1].node1=3;
    connection_list[3][1].node2=4;
    connection_list[3][2].node1=3;
    connection_list[3][2].node2=10;
    connection_list_size[4]=4;
    connection_list[4][0].node1=4;
    connection_list[4][0].node2=3;
    connection_list[4][1].node1=4;
    connection_list[4][1].node2=5;
    connection_list[4][2].node1=4;
    connection_list[4][2].node2=6;
    connection_list[4][3].node1=3;
    connection_list[4][3].node2=10;
    connection_list_size[5]=4;
    connection_list[5][0].node1=5;
    connection_list[5][0].node2=2;
    connection_list[5][1].node1=5;
    connection_list[5][1].node2=4;
    connection_list[5][2].node1=5;
    connection_list[5][2].node2=9;
    connection_list[5][3].node1=5;
    connection_list[5][3].node2=13;
    connection_list_size[6]=4;
    connection_list[6][0].node1=6;
    connection_list[6][0].node2=4;
    connection_list[6][1].node1=6;
    connection_list[6][1].node2=7;
    connection_list[6][2].node1=6;
    connection_list[6][2].node2=9;
    connection_list[6][3].node1=0;
    connection_list[6][3].node2=7;
    connection_list_size[7]=3;
    connection_list[7][0].node1=7;
    connection_list[7][0].node2=0;
    connection_list[7][1].node1=7;
    connection_list[7][1].node2=6;
    connection_list[7][2].node1=7;
    connection_list[7][2].node2=8;
    connection_list_size[8]=5;
    connection_list[8][0].node1=8;
    connection_list[8][0].node2=7;
    connection_list[8][1].node1=8;
    connection_list[8][1].node2=12;
    connection_list[8][2].node1=8;
    connection_list[8][2].node2=11;
    connection_list[8][3].node1=8;
    connection_list[8][3].node2=9;
    connection_list[8][4].node1=10;
    connection_list[8][4].node2=12;
    connection_list_size[9]=4;
    connection_list[9][0].node1=9;
    connection_list[9][0].node2=5;
    connection_list[9][1].node1=9;
    connection_list[9][1].node2=8;
    connection_list[9][2].node1=9;
    connection_list[9][2].node2=6;
    connection_list[9][3].node1=5;
    connection_list[9][3].node2=13;
    connection_list_size[10]=3;
    connection_list[10][0].node1=10;
    connection_list[10][0].node2=3;
    connection_list[10][1].node1=10;
    connection_list[10][1].node2=11;
    connection_list[10][2].node1=10;
    connection_list[10][2].node2=12;
    connection_list_size[11]=3;
    connection_list[11][0].node1=11;
    connection_list[11][0].node2=10;
    connection_list[11][1].node1=11;
    connection_list[11][1].node2=8;
    connection_list[11][2].node1=11;
    connection_list[11][2].node2=13;
    connection_list_size[12]=4;
    connection_list[12][0].node1=12;
    connection_list[12][0].node2=10;
    connection_list[12][1].node1=12;
    connection_list[12][1].node2=8;
    connection_list[12][2].node1=12;
    connection_list[12][2].node2=13;
    connection_list[12][3].node1=11;
    connection_list[12][3].node2=13;
    connection_list_size[13]=3;
    connection_list[13][0].node1=13;
    connection_list[13][0].node2=5;
    connection_list[13][1].node1=13;
    connection_list[13][1].node2=11;
    connection_list[13][2].node1=13;
    connection_list[13][2].node2=12;
    return;
}
void initialise_graph_weight()
{
    for(int i=0; i<NO_OF_NODES; i++)
    {
        for(int j=0; j<NO_OF_NODES; j++)
        {
            graph_weight[i][j]=0;
        }
    }
    graph_weight[0][1]=1136;
    graph_weight[0][2]=1702;
    graph_weight[0][7]=2838;
    graph_weight[1][2]=683;
    graph_weight[1][3]=959;
    graph_weight[2][5]=2049;
    graph_weight[3][4]=573;
    graph_weight[3][10]=2349;
    graph_weight[4][5]=1450;
    graph_weight[4][6]=732;
    graph_weight[5][9]=1976;
    graph_weight[5][13]=1128;
    graph_weight[6][7]=718;
    graph_weight[6][9]=1000;
    graph_weight[7][8]=706;
    graph_weight[8][9]=839;
    graph_weight[8][11]=366;
    graph_weight[8][12]=451;
    graph_weight[10][11]=596;
    graph_weight[10][12]=789;
    graph_weight[11][13]=385;
    graph_weight[12][13]=246;
    for(int i=0; i<NO_OF_NODES; i++)
    {
        for(int j=0; j<NO_OF_NODES; j++)
        {
            if(graph_weight[i][j]!=0)
                graph_weight[j][i]=graph_weight[i][j];
        }
    }
}
void display_connection_status()
{
    cout<<"\nConnection status for the graph\n";
    for(int i=0; i<NO_OF_NODES; i++)
    {
        cout<<"\nFailure of Node number "<<(i+1)<<" breaks the following connections :\n";
        for(int j=0; j<connection_list_size[i]; j++)
        {
            cout<<"("<<(connection_list[i][j].node1+1)<<","<<(connection_list[i][j].node2+1)<<")   ";
        }
    }
    return;
}
void display_graph_status()
{
    cout<<"\nGraph Status:\n";
    for(int i=0; i<NO_OF_NODES; i++)
    {
        cout<<"\n";
        for(int j=0; j<NO_OF_NODES; j++)
        {
            cout<<graph1[i][j]<<" ";
        }
    }
    cout<<"\n";
    return;
}




















