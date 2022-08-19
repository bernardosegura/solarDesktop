const fs = require('fs');
const path = require('path');
var WebSocketClient = require('websocket').client;
var client = new WebSocketClient();
const pathSetting = path.join(process.env.HOME, ".config","Solar_eDEX","settings.json");
let pathTheme = path.join(process.env.HOME, ".config","Solar_eDEX","themes");//,setting.theme + ".json")

console.log("Change Background Image v0.1 - Tools Solar");
console.log();
//validar la existencia de los archivos
if (!fs.existsSync(pathSetting)){

	console.log("Not found Solar install");
	console.log();
	return;
}

let setting = require(pathSetting);
let port = setting.port?(setting.port - 1):2999; 

pathTheme = path.join(pathTheme,setting.theme + ".json");

if (!fs.existsSync(pathTheme)){

	console.log("Not found Solar theme install");
	console.log();
	return;
}

let theme =  require(pathTheme);
const restCommMesg = require('./restCommMesg').restCommMesg;
let aplicaImagen = new restCommMesg();

if(process.argv.length > 2){

	if(process.argv[2] == '?'){

		console.log('- execute only cbgImage to remove background image');
		console.log('- execute cbgImage ["path from image"] [transparency]');
		console.log('- "path from image" to put background image');
		console.log('- transparency true to transparency in terminal');
		console.log('- transparency false or no input to not transparency in terminal');
		console.log('- execute cbgImage [?] to this help');
		console.log();
		return;
	}

	  fs.readFile(process.argv[2], function(err, data) {
	  if (err) console.log(err); // Fail if the file can't be read.
	  else{
	  	console.log('Background image install...');
	  	let bgImage = `data:image/${path.extname(process.argv[2]).toLowerCase().replace('.','')};base64,${data.toString('base64')}`;

	  	theme.backgroundImage = bgImage;
	  	
	  	console.log('OK');

	  	let msgSession = '';
	  	if(process.argv.length > 3){
	  
	  		if (process.argv[3] == 'true'){
	  			if(!theme.terminal.background){
	  				console.log("Theme no admit transparency");
	  			}else{
	  				if(theme.terminal.background.length == 7){
	  					if(!theme.terminal.allowTransparency) 
	  						msgSession = 'Efect transparency is OK';
	  					
	  					theme.terminal.allowTransparency = true;
	  					theme.terminal.background = theme.terminal.background + '80';
	  					
	  				}else
	  					console.log("Theme no admit transparency");
	  			}
	  		}else{
	  			if (process.argv[3] == 'false'){
	  				if(theme.terminal.allowTransparency) 
	  						msgSession = 'Efect no transparency is OK';
	  				if(theme.terminal.allowTransparency) delete theme.terminal.allowTransparency;
	  				if(theme.terminal.background){
	  					if(theme.terminal.background.length == 9){
	  						theme.terminal.background = theme.terminal.background.substr(0,7);
	  					}
	  				}
	  			}
	  		}
	  	}

		fs.writeFileSync(path.join(process.env.HOME, ".config","Solar_eDEX","themes",setting.theme + ".json"), JSON.stringify(theme, 4));
		console.log(msgSession);
		if(msgSession != '')
			console.log();
	  	//aplicaImagen.sendMessage("rcmSolar",{img:bgImage, call:"changeImage",isPriority: true, transparency:((theme.terminal.allowTransparency)?theme.terminal.allowTransparency:false)})
	 	client.connect('ws://localhost:' + port + '/',null,".xobj.rcmSolar"); //obtener archivo json para puerto
		client.on('connectFailed', function(error) {
		    console.log('Connect to Core: ' + error.toString());
		    console.log();
		});
		client.on('connect', function(connection) {
		    connection.sendUTF(JSON.stringify({message:{img:bgImage, call:"changeImage",isPriority: true, transparency:((theme.terminal.allowTransparency)?theme.terminal.allowTransparency:false)}}));
		    connection.close();
		}); 
	  }	 
	});

}else{

	if(theme.backgroundImage) delete theme.backgroundImage;

	if(theme.terminal.allowTransparency) delete theme.terminal.allowTransparency;
	
	if(theme.terminal.background){
		if(theme.terminal.background.length == 9){
			theme.terminal.background = theme.terminal.background.substr(0,7);
		}
	}

	console.log('Delete image from background...');
	fs.writeFileSync(pathTheme, JSON.stringify(theme, 4));
	console.log('OK');
	console.log();
	//aplicaImagen.sendMessage("rcmSolar",{call:"changeImage",isPriority: true})
	client.connect('ws://localhost:' + port + '/',null,".xobj.rcmSolar"); //obtener archivo json para puerto
	client.on('connectFailed', function(error) {
	    console.log('Connect to Core: ' + error.toString());
	    console.log();
	});
	client.on('connect', function(connection) {
	    connection.sendUTF(JSON.stringify({message:{call:"changeImage",isPriority: true}}));
	    connection.close();
	}); 
}
