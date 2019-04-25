//struct of an array of pointers to which an entire
//vol file can be read
struct volbatch
{
 virtual int size()=0;
 int read(char *volfilename); //read each pointer as a sequential element of vol file
 void free(); //free all pointers
 void print();
};
