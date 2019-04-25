#ifndef _COMMAND_
#define _COMMAND_

//-------------------------------------------------
//the command line arguments are parsed as follows:
//
// -<option> <parm1> <parm2> .... -<option2> etc



//structure describing all possible options
struct optiondef
{
 char *name; //name of option
 int (*func)(char *parms); //function to execute option before system initialization
 char flag; //flag indicating whether option was present

 char *syntax; //syntax
 char *desc; //description

 void printhelp();
};


//individual option (with parms) on the command line
struct option
{
 char *name; //actual option name
 char *parm; //parameters

 void execute(int type); //execute parms (0-pre 1-post)
 void print(); //print option+parms
 void clear() {name=parm=0;}
 void free();
};


//entire command line
class commandline
{
 int numoptions;
 option *options; //list of commands

 option *addoption();

 public:
 void execute(int type); //execute options/commands
 void print();


 void parse(int argc,char **argv);
 void parse(char *c);
 void parse(FILE *f);

 //constructors
 commandline();
 ~commandline();
};

extern commandline *cline; //global commandline


int getoption(char *name) ;
int getcommand(char *name) ;
#endif