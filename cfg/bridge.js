var http = require('http');
var os = require('os');
var fs = require('fs');

http.createServer(function (req, res) {
    var hosjs = fs.readFileSync('hos.js');
    var hoscss = fs.readFileSync('hos.css');
    res.writeHead(200, {'Content-Type': 'text/html'});
    res.write('<!DOCTYPE HTML>\n');
    res.write('<html><head><style>');
    res.write(hoscss);
    res.write('</style><title>ORLANDOviols consort box</title>\n</head><body>\n');
    res.write('<h1>ORLANDOviols consort box ('+os.hostname()+')</h1>\n<div id="mixer">mixer</div>\n');
    res.write('<script src="http://'+os.hostname()+':8081/socket.io/socket.io.js"></script>\n');
    res.write('<script>\n');
    res.write('var socket = io("http://'+os.hostname()+':8081");\n');
    res.write(hosjs);
    res.end('</script>\n</body></html>');
}).listen(8080);

var osc = require('node-osc');
var io = require('socket.io').listen(8081);

var oscServer, oscClient;

oscServer = new osc.Server( 9000, '0.0.0.0' );
oscClient = new osc.Client( 'localhost', 9877 );

io.on('connection', function (socket) {
    socket.on('config', function (obj) {
	oscClient.send('/status', socket.id + ' connected');
	oscServer.on('message', async function(msg, rinfo) {
	    if( msg[0] == '/touchosc/scene' ){
		socket.emit('scene', 'scene');
	    }
	    if( msg[0].startsWith('/touchosc/label') && (!msg[0].endsWith('/color')) && (msg[1].length>1)){
		socket.emit('newfader', msg[0].substr(15), msg[1] );
		await new Promise(r => setTimeout(r, 100));
	    }
	    if( msg[0].startsWith('/touchosc/fader') && (!msg[0].endsWith('/color')) ){
		socket.emit('updatefader', msg[0], msg[1] );
	    }
	});
	oscClient.send('/touchosc/connect',16);
    });
    socket.on('message', function (obj) {
	oscClient.send(obj);
    });
    socket.on('msg', function (obj) {
	if( obj.hasOwnProperty('value') ){
	    oscClient.send( obj.path, obj.value );
	}else{
	    oscClient.send( obj.path );
	}
    });
    socket.on('defaultgains', function (obj) {
	fs.unlink('savedgains');
    });
});
