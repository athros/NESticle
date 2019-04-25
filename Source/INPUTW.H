#ifndef _INPUTW_
#define _INPUTW_

typedef input *INPUTPTR;
class changeinputdevice:public GUIcontents
{
 protected:
 INPUTPTR &in; //pointer to input device being changed

 public:
 changeinputdevice(INPUTPTR &tin);
 virtual ~changeinputdevice();

 virtual int sendmessage(GUIrect *c,int guimsg);

 virtual int acceptfocus() {return 1;}
 virtual char *getname() {return "changeinputdevice";}
 virtual void draw(char *dest);

 static void open(int inputdevicenum);
};




#endif