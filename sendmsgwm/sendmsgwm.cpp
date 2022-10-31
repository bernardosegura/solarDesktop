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
//g++ sendmsgwm.cpp -o sendmsgwm -lX11

#include <cstdlib>
#include <cstdio>
extern "C" {
#include <X11/Xutil.h>
}

int main(int argc, char** argv) {
  char *display_name;
  Status estado;

  if(argc < 3)
  {
    printf("Error --> %s window [msg (c,a... is char)] (null is default,[0 not update taskbar], [1 not update taskbar (default)])\n",argv[0]);
    return 1;
  }

  display_name = XDisplayName(NULL);

  Display* display = XOpenDisplay(display_name);

  if(display == NULL){
    printf("Error to open display: %s\n",display_name );
    return 1;
  }
    
  
  XEvent evt;
  evt.xclient.type = ClientMessage;
  evt.xclient.serial = 0;
  evt.xclient.send_event = true;
  evt.xclient.message_type = XInternAtom(display,"SOLAR_WM",false);;
  evt.xclient.format = 32;
  evt.xclient.window = atoi(argv[1]);
  evt.xclient.data.b[0] = argv[2][0];
  evt.xclient.data.b[1] = '1';
  evt.xclient.data.b[2] = '0';
  
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
    
  XCloseDisplay(display);

  printf("%d\n",estado);

  return 0;
}
