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
    let elbutsave=document.createElement("button");
    elbutsave.setAttribute("id","/savegains/save");
    elbutsave.setAttribute("onclick","socket.emit('msg',{path:'/savegains/save'});");
    elbutsave.append("save");
    elgainstore.appendChild(elbutsave);
    let elbutrestore=document.createElement("button");
    elbutrestore.setAttribute("id","/savegains/restore");
    elbutrestore.setAttribute("onclick","socket.emit('msg',{path:'/savegains/restore'});");
    elbutrestore.append("restore");
    elgainstore.appendChild(elbutrestore);
    el.appendChild(elgainstore);
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
    els.setAttribute("min","-20");
    els.setAttribute("max","5");
    els.setAttribute("value",val);
    els.setAttribute("step","0.1");
    els.setAttribute("id",fader);
    let elsl=document.createElement("meter");
    elsl.setAttribute("class","level");
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
let netcf=document.getElementById("peer2peer");
netcf.oninput = handleNetChange;
function handleNetChange(e) {
    socket.emit("peer2peer", e.target.checked);
}
let netdup = document.getElementById("duplicates");
netdup.oninput = (function (e) { socket.emit("duplicates", e.target.checked);});
socket.on("updatep2p", function(val){
    let fad=document.getElementById("peer2peer");
    if( fad!=null ){
	fad.checked=(val>0);
    }
});
socket.on("updateduplicates", function(val){
    let fad=document.getElementById("duplicates");
    if( fad!=null ){
	fad.checked=(val>0);
    }
});
