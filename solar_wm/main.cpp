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

if(argc == 1)
{
	argv[1] = cDef;
	argv[2] = cDef;
  argv[3] = cDef;
}

if(argc == 2)
{
	argv[2] = cDef;
  argv[3] = cDef;
}

if(argc == 3)
{
  argv[3] = cDef;
}

window_manager->BORDER_COLOR = std::stol(argv[1]);
window_manager->BG_COLOR = std::stol(argv[2]);
window_manager->wndPanel = std::stol(argv[3]);

  window_manager->Run();

  return EXIT_SUCCESS;
}
