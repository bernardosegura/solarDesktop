const fs = require('fs');
const path = require('path');

console.log("Execute file xobj in command line. v0.1 - Tools Solar");
console.log();


const restCommMesg = require('./restCommMesg').restCommMesg;
let execute = new restCommMesg();


if(process.argv.length > 2){

	if(process.argv[2] == '?'){

		console.log('Execute only xobj "file name" or "path and file name"');
		console.log();
		return;
	}
	console.log('Executing file... ' + process.argv[2]);
	console.log();
	execute.sendMessage("rcmSolar",{file:process.argv[2], call:"appxwnd",isPriority: true});	  
}else{

	console.log('Execute only xobj "file name" or "path and file name"');
	console.log();
}
