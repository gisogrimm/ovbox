socket.on("connect", function() {
    socket.emit("config",{});
});
socket.on("scene", function(scene){
    let el=document.getElementById("mixer");
    while (el.firstChild) {el.removeChild(el.firstChild);}
    let els=document.createElement("h2");
    els.setAttribute("class","scene");
    el.append(scene,els);
    //<button onclick="socket.emit('loadpreset', 52);">[5/2]</button>
    let elp=document.createElement("p");
    elp.setAttribute("class","gainstore");
    let el1=document.createElement("button");
    el1.setAttribute("id","/savegains/save");
    el1.setAttribute("onclick","socket.emit('msg',{path:'/savegains/save'});");
    el1.append("save");
    elp.appendChild(el1);
    let el2=document.createElement("button");
    el2.setAttribute("id","/savegains/restore");
    el2.setAttribute("onclick","socket.emit('msg',{path:'/savegains/restore'});");
    el2.append("restore");
    elp.appendChild(el2);
    el.appendChild(elp);
});
socket.on("newfader", function(faderno,val){
    fader="/touchosc/fader"+faderno;
    levelid="/touchosc/level"+faderno;
    let el=document.getElementById("mixer");
    let elp=document.createElement("p");
    elp.setAttribute("class","mixerstrip");
    let ell=document.createElement("label");
    ell.setAttribute("for",fader);
    ell.append(val);
    let els=document.createElement("input");
    els.setAttribute("class","fader");
    els.setAttribute("type","range");
    els.setAttribute("min","-30");
    els.setAttribute("max","10");
    els.setAttribute("value",val);
    els.setAttribute("step","0.1");
    els.setAttribute("id",fader);
    let elsl=document.createElement("meter");
    elsl.setAttribute("class","level");
    //elsl.setAttribute("type","range");
    elsl.setAttribute("min","0");
    elsl.setAttribute("max","94");
    elsl.setAttribute("low","71");
    elsl.setAttribute("high","84");
    elsl.setAttribute("optimum","54");
    elsl.setAttribute("value",val);
    elsl.setAttribute("id",levelid);
    el.appendChild(elp);
    elp.appendChild(els);
    elp.appendChild(elsl);
    elp.appendChild(ell);
});
socket.on("updatefader", function(fader,val){
    let fad=document.getElementById(fader);
    if( fad!=null ){
	fad.value=val;
    }
});
let form = document.getElementById("mixer");
form.oninput = handleChange;
function handleChange(e) {
    socket.emit("msg", { path: e.target.id, value: e.target.valueAsNumber } );
}
