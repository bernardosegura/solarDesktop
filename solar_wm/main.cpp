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

#include <cstdlib>
#include <glog/logging.h>
#include "window_manager.hpp"

/*#include <fstream>
#include <jsoncpp/json/json.h>*/ 

using ::std::unique_ptr;
char cDef[1];

int main(int argc, char** argv) {
  //::google::InitGoogleLogging(argv[0]);

  unique_ptr<WindowManager> window_manager = WindowManager::Create();
  if (!window_manager) {
    LOG(ERROR) << "Failed to initialize window manager.";
    return EXIT_FAILURE;
  }

cDef[0] = '0';
char sPuerto[] = {'2','9','9','9'};  
FILE *fp;
char *data;
 const char* product_name[] = {"butterfly", "link", "lumpy", "parrot", "stout", "stumpy",
 "falco", "leon", "mccloud", "monroe", "panther", "peppy", "tricky", "wolf", "zako",
 "auron_paine", "auron_yuna", "buddy", "gandof", "guado", "lulu", "rikku", "samus", "tidus",
 "banjo", "candy", "clapper", "enguarde","glimmer", "gnawty", "heli", "kip", "ninja", "orco", "quawks", "squawks", "sumo","swanky","winky",
 "banon", "celes", "cyan", "edgar", "kefka", "reks","relm", "setzer", "terra", "terra13", "ultima", "wizpig", 
 "asuka", "caroline", "cave","chell", "lars", "sentry", 
 "astronaut","babymega", "babytiger", "blacktip", "blue", "bruce", "electro", "epaulette", "lava", "nasher","nasher360","pyro","rabbid","reef", "robo", "robo360", "sand", "santa", "snappy", "whitetip", 
 "akali", "akali360","atlas","bard","ekko", "eve","excelsior","jax","karma", "kench","leona","nautilus","nocturne", "pantheon","shyvana","sion","sona", "soraka","syndra","teemo","vayne", "wukong",
 "ampton", "apel","bloog","blooglet","blooguard", "blorb","bluebird","bobba","bobba360", "casta","dood","dorp","droid", "fleex","foob","foob360","garg", "garg360","garfour","glk","glk360", "grabbiter","laser","laser14","lick", "meep","mimrock","nospike","orbatrix", "phaser","phaser360","phaser360s","sparky", "sparky360","vorticon","vortininja",
 "aleena", "barla","careena","kasumi","kasumi360", "liara","treeya","treeya360",
 "arcada", "sarien",
 "akemi","dragonair","drallion", "dratini","duffy","faffy","helios", "jinlon","kaisa","kindred","kled", "kohaku","nightfury","noibat","wyvern",
 "berknip","dirinboz","ezkinil","gumboz", "morphius","vilboz","vilboz14","vilboz360", "woomax",
 "chronicler","collis","copano","delbin", "drobit","eldrid","lillipup","lindar", "voema","chronicler","volet","volta","voxel", 
 "beetley","blipper","bookem","boten", "botenflex","bugzzy","cret","cret360", "drawcia","drawlat","drawman","drawper", "galith","galith360","gallop","galnat", "galnat360","galtic","galtic360","kracko","kracko360","landia","landrid", "lantis","madoo","magister","maglet", "maglia","maglith","magma","magneto", "magolor","magpie","metaknight","pasara", "pirette", "pirika","sasuke","storo","storo360",
 "anahera","banshee","crota","crota360", "felwinter","kano","mithrax","osiris", "primus","redrix","taniks","taeko", "volmar","zavala"};

if(argc == 1)
{
	argv[1] = cDef;
	argv[2] = sPuerto;
  argv[3] = cDef;
  argv[4] = cDef;
  argv[5] = cDef;
}

if(argc == 2)
{
	argv[2] = sPuerto;
  argv[3] = cDef;
  argv[4] = cDef;
  argv[5] = cDef;
}

if(argc == 3)
{
  argv[3] = cDef;
  argv[4] = cDef;
  argv[5] = cDef;
}

if(argc == 4)
{
  argv[4] = cDef;
  argv[5] = cDef;
}

if(argc == 5)
{
  argv[5] = cDef;
}

window_manager->screenWidth = std::stol(argv[1]);
window_manager->puerto = std::stol(argv[2]);
window_manager->BORDER_COLOR = std::stol(argv[3]);//std::stol(argv[3],nullptr,16);
window_manager->BG_COLOR = std::stol(argv[4]);
window_manager->wndPanel = std::stol(argv[5]);

window_manager->isChromebook = false;
fp = fopen("/sys/devices/virtual/dmi/id/board_vendor", "r");
if(fp){
    fseek(fp, 0L, SEEK_END); // Mover el puntero de archivo al final del archivo
    size_t size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    data = (char*)malloc(size * sizeof(char));
    fread(data, 1, size, fp);
    fclose(fp);
    for (size_t i = 0; i < size; i++) {
        if(data[i] == '\n')
          data[i] = '\0';
        data[i] = tolower(data[i]);
    }

    if (strcmp(data, "google") == 0) {
        window_manager->isChromebook = true;
    }
}

if(!window_manager->isChromebook){
  fp = fopen("/sys/devices/virtual/dmi/id/chassis_vendor", "r");
  if(fp){
      fseek(fp, 0L, SEEK_END); // Mover el puntero de archivo al final del archivo
      size_t size = ftell(fp);
      fseek(fp, 0L, SEEK_SET);
      data = (char*)malloc(size * sizeof(char));
      fread(data, 1, size, fp);
      fclose(fp);
      for (size_t i = 0; i < size; i++) {
          if(data[i] == '\n')
            data[i] = '\0';
          data[i] = tolower(data[i]);
      }

      if (strcmp(data, "google") == 0) {
          window_manager->isChromebook = true;
      }
  }
}

if(!window_manager->isChromebook){
  fp = fopen("/sys/devices/virtual/dmi/id/product_name", "r");
  if(fp){
      fseek(fp, 0L, SEEK_END); // Mover el puntero de archivo al final del archivo
      size_t size = ftell(fp);
      fseek(fp, 0L, SEEK_SET);
      data = (char*)malloc(size * sizeof(char));
      fread(data, 1, size, fp);
      fclose(fp);
      for (size_t i = 0; i < size; i++) {
          if(data[i] == '\n')
            data[i] = '\0';
          data[i] = tolower(data[i]);
      }

      for (size_t i = 0; i < (sizeof(product_name)/sizeof(char*)); i++) {
        if (strcmp(data, product_name[i]) == 0){
            window_manager->isChromebook = true;
            break;
        }   
      }  
  }
}

 window_manager->Run();
 return EXIT_SUCCESS;

}
