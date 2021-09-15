class restCommMesg{
	
	constructor(appName,receiveMessage,path){
		const fs = require('fs');
		const dir = require('path');
		let urlDefault = dir.join(process.env.HOME,".containerrcm");//"C:\\Users\\usuario\\Desktop\\node11\\msgDirect\\.containerrcm";
		let now= new Date();
	
		if(!receiveMessage) receiveMessage = false; //(message) =>{};
		this.appName = (appName != '')?appName:now.getTime();
		this.path = (!path)?urlDefault:((path != '')?path:urlDefault);

		if (!fs.existsSync(this.path)) {
			fs.mkdirSync(this.path);
		}

		if(receiveMessage !== false)
			fs.watch(this.path, (event,file) => { 
	                if(event === 'change') //cuando se escribe contenido archivo es change
	                {
	                    if(file.toLowerCase().endsWith("." + this.appName.toLowerCase()))
	                    {
	                      if (fs.existsSync(dir.join(this.path,file))){
		                        //procesar y borrar
		                       let data = JSON.parse(fs.readFileSync(dir.join(this.path,file)).toString());
		                       //if(!data.message) data.message = {};
		                       fs.unlinkSync(dir.join(this.path,file));
		                       receiveMessage(data/*.message*/);
		                      // process.exit();
	                      }
	                    }
	                }
	            });
	}

	sendMessage(appName,message){ //validar y poner parametro por si el mensaje ya existe no replazarlo o remplazarlo
		if(appName == '') throw "appName not found";
		let response = {state:0, message:'OK'} // que sea un oobjeto
		const fs = require('fs');
		const dir = require('path');

		if(!message.isPriority)message.isPriority = false;
		if(message.isPriority !== false)message.isPriority = true;

		//message.callback = this.appName;
		if (!fs.existsSync(dir.join(this.path,this.appName + "." + appName)) || message.isPriority){
			let send = {callback:this.appName,"message":message};
			fs.writeFileSync(dir.join(this.path,"." + this.appName + "." + appName), JSON.stringify(send, 4));
		}else{
			response.state = -1;
			response.message = 'Another message is waiting';
		}
		return response;
	}
}

module.exports = {
    restCommMesg
};