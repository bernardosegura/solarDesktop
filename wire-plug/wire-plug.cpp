#include <stdio.h>
#include <pwd.h>
#include <cstdlib>
#include <unistd.h>

#include "easywsclient.hpp"
#include <assert.h>
#include "easywsclient.cpp"
#include <memory>

#include <fstream>
#include "json.hpp"
#include <iostream>

#include <bits/stdc++.h>
#include <string>

//g++ wire-plug.cpp -o wire-plug
int main(int arc, char** arv)
{
    //static char *username = NULL;
    char* arg[] = {(char*)"",NULL};
    char jsonPath[80];
    //FILE *jsonFile; 
    using easywsclient::WebSocket;
    using json = nlohmann::json;
    //char catacter;
    //char palabra[50];
    
    
    if(arc < 2){
        fprintf(stderr, "%s [hdmi or ac(0 not connected or 1 connected )]\n", arv[0]);
        return 1;
    }

    /*if(!username){
        username = getenv("USERNAME");
      }

    if(!username){
        std::string logname = getlogin();
        username = (char*)logname.c_str();
      }*/

/*FILE *archivo = fopen("/home/bernardosegura/usuario.txt", "w");
fprintf(archivo, "%s \n", username);
fclose(archivo);*/
      /*if(!home){
        struct passwd *pw = getpwuid(getuid());
        if(pw)
          home = pw->pw_dir;
      }*/

    //if((std::string)arv[1] == "hdmi"){
        //sprintf(jsonPath,"%s/.config/Solar_eDEX/settings.json",home); 
        //sprintf(jsonPath,"/home/%s/.config/Solar_eDEX/settings.json",username); 
        sprintf(jsonPath,"/tmp/cnfgport.json"); 
        std::ifstream f(jsonPath);
        json data = json::parse(f);
        long port = (long)data["port"] - 1; 
        
        if(arc > 2){
            if((std::string)arv[1] == "dusb"){
                char *type = strtok(arv[2],":");
                if((std::string)type == "partition"){
                    char *action = strtok(NULL,":");
                    char *devBuff = strtok(NULL,":");
                    char *uuid = strtok(NULL,":");
                    char *label = strtok(NULL,":");
                    std::string dev(devBuff);
                    std::string mensaje("{\"message\":{\"call\":\"wire-plug\",\"type\":\""+ (std::string)arv[1] + "\",");

                    //FILE *stdOut;
                    for (int i = dev.find("-"); i >= 0; i = dev.find("-")){
                        dev.replace(dev.find("-"),1,"/");
                    }
                    //sprintf(jsonPath,"%s/.config/Solar_eDEX/%s/%s.json",home,arv[1],uuid); 

                    if(label != NULL){
                    	if(uuid != NULL){
                            mensaje += "\"subdata\":{\"subtype\":\"partition\",\"action\":\"" + (std::string)action + "\",\"dev\":\"" + dev + "\",\"uuid\":\"" + (std::string)uuid + "\",\"label\":\"" + (std::string)label + "\"}}}";
	                    }
	                    else{
	                        mensaje += "\"subdata\":{\"subtype\":\"partition\",\"action\":\"" + (std::string)action + "\",\"dev\":\"" + dev + "\",\"uuid\":\"\",\"label\":\"" + (std::string)label + "\"}}}";
	                    }
                    }
                    else{
                        if(uuid != NULL){
                            mensaje += "\"subdata\":{\"subtype\":\"partition\",\"action\":\"" + (std::string)action + "\",\"dev\":\"" + dev + "\",\"uuid\":\"" + (std::string)uuid + "\",\"label\":\"\"}}}";
	                    }
	                    else{
	                        mensaje += "\"subdata\":{\"subtype\":\"partition\",\"action\":\"" + (std::string)action + "\",\"dev\":\"" + dev + "\",\"uuid\":\"\",\"label\":\"\"}}}";
	                    }
                    }

                    /*stdOut = fopen(jsonPath, "w"); 
                    fprintf(stdOut, "%s",mensaje.c_str());
                    fclose(stdOut);*/

                    std::unique_ptr<WebSocket> ws(WebSocket::from_url("ws://localhost:" + std::to_string(port),".wire-plug.rcmSolar"));
                    assert(ws);
                    ws->send(mensaje);
                    ws->poll();
                    ws->close();
                }
                
            }
        }else{

            std::unique_ptr<WebSocket> ws(WebSocket::from_url("ws://localhost:" + std::to_string(port),".wire-plug.rcmSolar"));
            assert(ws);
            ws->send("{\"message\":{\"call\":\"wire-plug\",\"type\":\""+ (std::string)arv[1] + "\"}}");
            ws->poll();
            ws->close();
        }
        /*jsonFile = fopen(jsonPath, "r");
        if(!jsonFile){
            fprintf(stderr, "Errro Open File: %s \n", jsonPath);
            return 1;
        }
        while(!feof(jsonFile)){
            catacter = fgetc(jsonFile);
            if(catacter == ' '){

            }else{
                palabra
            }
        }
        //fprintf(jsonFile,"{\"index\":0,\"cmds\":[\"xrandr --output <-destino-> --same-as <-origen->\",\"xrandr --output <-origen-> --left-of <-destino->\",\"xrandr --output <-origen-> --right-of <-destino->\"]}");
        fclose(jsonFile); */ 
    //}  
    return 0;
}