const path = require('path');
const os = require('os');
var WebSocketClient = require('websocket').client;
var client = new WebSocketClient();

console.log("Execute file xobj in command line. v0.1 - Tools Solar");
console.log();


//const restCommMesg = require('./restCommMesg').restCommMesg;
//let execute = new restCommMesg();
var settings = require(path.join(os.homedir(), '.config','Solar_eDEX','settings.json'));
let port = settings.port?(settings.port - 1):2999; 

if(process.argv.length > 2){

	if(process.argv[2] == '?'){

		console.log('Execute only xobj "file name" or "path and file name"');
		console.log();
		return;
	}
	console.log('Executing file... ' + process.argv[2]);
	console.log();
	//execute.sendMessage("rcmSolar",{file:process.argv[2], call:"appxwnd",isPriority: true});	
	client.connect('ws://localhost:' + port + '/',null,".xobj.rcmSolar"); //obtener archivo json para puerto
	client.on('connectFailed', function(error) {
	    console.log('Connect to Core: ' + error.toString());
	    console.log();
	});
	client.on('connect', function(connection) {
	    connection.sendUTF(JSON.stringify({message:{file:process.argv[2], call:"appxwnd",isPriority: true}}));
	    connection.close();
	});  
}else{

	console.log('Execute only xobj "file name" or "path and file name"');
	console.log();
}
