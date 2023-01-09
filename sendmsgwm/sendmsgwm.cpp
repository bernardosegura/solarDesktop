/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                           *
 *  Copyright (C) 2013-2017 Chuan Ji <ji@chu4n.com>                          *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *   http://www.apache.org/licenses/LICENSE-2.0                              *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 *   Update from solar-wm by Bernardo Segura                                 *
 *                                                                           *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
//g++ sendmsgwm.cpp -o sendmsgwm -lX11 -lXtst

#include <cstdlib>
#include <cstdio>
extern "C" {
#include <X11/Xutil.h>
}

#include <cstring>
#include <X11/extensions/XTest.h>

bool isDigit(char c){
  char digit[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','a','b','c','d','e','f'};
  if(c == '\0')
    return false;
  for (int i = 0; i < strlen(digit); ++i){
    if(c == digit[i])
      return true;
  }
  return false;
}

int main(int argc, char** argv) {
  char *display_name;
  Status estado;

  if(argc < 2){
    printf("Error --> %s number window, \"keyCode\" or \"theme\"\n",argv[0]);
    return 1;
  }

  display_name = XDisplayName(NULL);

  Display* display = XOpenDisplay(display_name);

  if(display == NULL){
    printf("Error to open display: %s\n",display_name );
    return 1;
  }
    
  if(strcmp(argv[1],"keyCode") == 0){
    unsigned int keyCode = 0;

    if(argc < 3){
      printf("Error --> %s \"keyCode\" A (key CapsLock) or 1 (key NumLock)\n",argv[0]);
      return 1;
    }

    if(strcmp(argv[2],"A") == 0)
      keyCode = XKeysymToKeycode(display,XK_Caps_Lock);

    if(strcmp(argv[2],"1") == 0)
      keyCode = XKeysymToKeycode(display,XK_Num_Lock);
    
    if(keyCode != 0){
      XTestFakeKeyEvent(display,keyCode,true,CurrentTime);
      XFlush(display);
      XTestFakeKeyEvent(display,keyCode,false,CurrentTime);
      XFlush(display);
    }

    printf("%s\n",argv[2]);
  }else{
    if(strcmp(argv[1],"theme") == 0){
        if(argc < 5){
          printf("Error --> %s \"theme\" ([fontcolor] [backcolor] in hexadecimal) number window\n",argv[0]);
          return 1;
        }

        XEvent evt;
        evt.xclient.type = ClientMessage;
        evt.xclient.serial = 0;
        evt.xclient.send_event = true;
        evt.xclient.message_type = XInternAtom(display,"DISPLAY_BRIGHTNESS_VOLUME",false);;
        evt.xclient.format = 8;
        evt.xclient.window = atoi(argv[4]);

        evt.xclient.data.b[0] = 'T';

        for (int i = 1; i < 7; ++i)
        {
          if(strlen(argv[2]) > i-1)
            evt.xclient.data.b[i] = isDigit(argv[2][i-1])?argv[2][i-1]:'0';
          else
            evt.xclient.data.b[i] = '0';
        }

        for (int i = 0; i < 6; ++i)
        {
          if(strlen(argv[3]) > i)
            evt.xclient.data.b[i+7] = isDigit(argv[3][i])?argv[3][i]:'0';
          else
            evt.xclient.data.b[i+7] = '0';
        }

        estado = XSendEvent(display,atoi(argv[4]),false,ExposureMask,&evt);

        if(estado)
          XFlush(display);

        printf("%d\n",estado);
    }else{
      XEvent evt;

      if(argc < 3){
        printf("Error --> %s number window [msg (c,a... is char)] (null is default,[0 not update taskbar], [1 not update taskbar (default)])\n",argv[0]);
        return 1;
      }
      evt.xclient.type = ClientMessage;
      evt.xclient.serial = 0;
      evt.xclient.send_event = true;
      evt.xclient.message_type = XInternAtom(display,"SOLAR_WM",false);;
      evt.xclient.format = 8;
      evt.xclient.window = atoi(argv[1]);
      evt.xclient.data.b[0] = argv[2][0];
      evt.xclient.data.b[1] = '1';//numlock state
      evt.xclient.data.b[2] = '0';//capslock state
      
      if(argc > 3) 
        evt.xclient.data.b[1] = argv[3][0];
      if(argc > 4) 
        evt.xclient.data.b[2] = argv[4][0];

      estado = XSendEvent(display,
        DefaultRootWindow(display),//se puede madar la misma ventana se decide mandar a root
        true,//si no se propaga el mensaje no es cachado por wm
        (SubstructureRedirectMask | SubstructureNotifyMask),//si no se agregan las mascaras que procesa wm es ignorado por el mismo
        &evt);

      if(estado)
        XFlush(display);

      printf("%d\n",estado);
    }
  }

  XCloseDisplay(display);

  return 0;
}
