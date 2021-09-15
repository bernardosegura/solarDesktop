
const restCommMesg = require('./restCommMesg').restCommMesg;

let commMesg = new restCommMesg("mensajerouno",(data)=>{
  console.log(data);
  if(data.message.saludo != "Hola Mundo")
    process.exit();
  else
     if(data.message.call == 'prueba')
        console.log(commMesg.sendMessage(data.callback,{saludo:"Hola Mundo"}));
});

console.log(commMesg.sendMessage("mensajerouno",{saludo:"Hola Mundo", call:"prueba"}));
console.log(commMesg.sendMessage("mensajerouno",{saludo:"Hola Mundo", call:"prueba",isPriority: false}));
console.log(commMesg.sendMessage("mensajerouno",{saludo:"Hola Mundo", call:"prueba",isPriority: true}));
