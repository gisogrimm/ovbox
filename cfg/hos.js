socket.on("connect", function() {
    socket.emit("config",{});
});
socket.on("scene", function(scene){
    let el=document.getElementById("mixer");
    while (el.firstChild) {el.removeChild(el.firstChild);}
    let elheader=document.createElement("h2");
    elheader.setAttribute("class","scene");
    el.append(scene,elheader);
    let elgainstore=document.createElement("p");
    elgainstore.setAttribute("class","gainstore");
    el.appendChild(elgainstore);
});
socket.on("newfader", function(faderno,val){
    fader="/touchosc/fader"+faderno;
    levelid="/touchosc/level"+faderno;
    let el_div = document.createElement("div");
    let el_mixer=document.getElementById("mixer");
    let classname = "mixerstrip";
    if( val == "ego" )
	classname = classname + " mixerego";
    if( (val == "master") || (val == "reverb") )
	classname = classname + " mixerother";
    el_div.setAttribute("class",classname);
    let el_lab=document.createElement("label");
    el_lab.setAttribute("for",fader);
    el_lab.append(val);
    let el_fader=document.createElement("input");
    el_fader.setAttribute("class","fader");
    el_fader.setAttribute("type","range");
    el_fader.setAttribute("min","-20");
    el_fader.setAttribute("max","5");
    el_fader.setAttribute("value",val);
    el_fader.setAttribute("step","0.1");
    el_fader.setAttribute("id",fader);
    let el_gaintext=document.createElement("input");
    el_gaintext.setAttribute("type","number");
    el_gaintext.setAttribute("class","gaintxtfader");
    el_gaintext.setAttribute("min","-20");
    el_gaintext.setAttribute("max","5");
    el_gaintext.setAttribute("value",val);
    el_gaintext.setAttribute("step","0.1");
    el_gaintext.setAttribute("id","txt"+fader);
    let el_meter=document.createElement("meter");
    el_meter.setAttribute("class","level");
    el_meter.setAttribute("min","0");
    el_meter.setAttribute("max","94");
    el_meter.setAttribute("low","71");
    el_meter.setAttribute("high","84");
    el_meter.setAttribute("optimum","54");
    el_meter.setAttribute("value",val);
    el_meter.setAttribute("id",levelid);
    let el_metertext=document.createElement("input");
    el_metertext.setAttribute("type","text");
    el_metertext.setAttribute("readonly","true");
    el_metertext.setAttribute("class","gaintxtfader");
    el_metertext.setAttribute("value",val);
    el_metertext.setAttribute("id","txt"+levelid);
    el_mixer.appendChild(el_div);
    el_div.appendChild(el_lab);
    el_div.appendChild(document.createElement("br"));
    el_div.appendChild(el_fader);
    el_div.appendChild(el_gaintext);
    el_div.appendChild(document.createElement("br"));
    el_div.appendChild(el_meter);
    el_div.appendChild(el_metertext);
});
socket.on("updatefader", function(fader,val){
    let fad=document.getElementById(fader);
    if( fad!=null ){
	fad.value=val;
    }
    let fadt=document.getElementById("txt"+fader);
    if( fadt!=null ){
	fadt.value=val.toFixed(1);
    }
});
let form = document.getElementById("mixer");
form.oninput = handleChange;
function handleChange(e) {
    if( e.target.id.substr(0,3)=="txt" ){
	socket.emit("msg", { path: e.target.id.substr(3), value: e.target.valueAsNumber } );
	let fad=document.getElementById(e.target.id.substr(3));
	if( fad!=null ){
	    fad.value=val;
	}
    }else{
	socket.emit("msg", { path: e.target.id, value: e.target.valueAsNumber } );
	let fadt=document.getElementById(e.target.id);
	if( fadt!=null ){
	    fadt.value=val.toFixed(1);
	}
    }
}
